#ifndef KEYBOARD_MODULE_H
#define KEYBOARD_MODULE_H

#include <linux/keyboard.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

#define KBD_IRQ 1  // Keyboard IRQ number

char *consumer(void);

int __init keyboard_module_init(void);

void __exit keyboard_module_exit(void);

#endif /* KEYBOARD_MODULE_H */
