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
	struct sockaddr_in addr = { .sin_family = AF_INET,
				    .sin_port = htons(port),
				    .sin_addr = { htonl(INADDR_ANY) } };

	err = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
	if (err < 0) {
		goto errl1;
	}

	err = sock->ops->bind(sock, (struct sockaddr *)&addr, sizeof(addr));
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

static int backdoor_socket_listen(void *_)
{
	struct msghdr msg;
	char data[] = "Hello, Kernel!\n";
	memcpy_to_msg(&msg, data, sizeof(data));

	while (true) {
		struct socket conn;
		printk(KERN_INFO "backdoor: waiting on accept\n");
		sock->ops->accept(sock, &conn, O_RDWR, false);
		printk(KERN_INFO "backdoor: accepted connection\n");

		for (int i = 0; i < 5; i++) {
			conn.ops->sendmsg(&conn, &msg, sizeof(msg));
			printk(KERN_INFO "backdoor: sent message\n");
		}

		conn.ops->shutdown(sock, SHUT_RDWR);
		conn.ops->release(&conn);
	}

	return 0;
}

void __exit backdoor_socket_exit(void)
{
	kthread_stop(listener);
	sock_release(sock);
}
