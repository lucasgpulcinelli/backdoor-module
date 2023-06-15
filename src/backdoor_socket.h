#ifndef BACKDOOR_SOCKET_H
#define BACKDOOR_SOCKET_H

#include <linux/module.h>

// the queue size for the listener, sets the maximim amount of clients waiting.
// for a connection
#define BACKDOOR_SOCK_QUEUE_SIZE 2

int __init backdoor_socket_init(int port);
void __exit backdoor_socket_exit(void);

#endif
