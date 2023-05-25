#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

static int __init backdoor_init(void)
{
	printk(KERN_INFO "Hello, World!\n");
	return 0;
}

static void __exit backdoor_exit(void)
{
	printk(KERN_INFO "Goodbye world...\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lucas Eduardo Gulka Pulcinelli");
MODULE_DESCRIPTION(
	"A demonstration of a keylogger at the operating system level");
MODULE_VERSION("0.0.1");

module_init(backdoor_init);
module_exit(backdoor_exit);
