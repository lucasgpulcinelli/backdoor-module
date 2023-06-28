# Linux backdoor module
A simple demonstration of a linux kernel module that sends keyboard data to an external server, used for learning modules for the operating systems I class.

## Steps to compile the module
- install a fedora 38 virtual machine (please use a virtual machine, you don't want to have a keylogger backdoor in your main system)
- switch the windowing manager to "gnome on Xorg" (to do that, log out, and in the same screen you put your password in you will see a cog in the bottom right corner)
- update the kernel to version 6.3.6-200.fc38.x86\_64, a few minor versions above or below should be fine
- `sudo dnf install kernel-devel-6.3.6-200`
- reboot the machine to use the correct kernel version (only if was not running it before)
- go to the root of the backdoor project
- `make` to compile the module itself, this is the only step that needs to be repeated if the source of the module changes
- `make insert` to insert the backdoor module in the currently running kernel. **BE CAREFUL WITH THIS COMMAND**
- `make remove` to remove the backdoor module. Note that after inserting the kernel will be considered "tainted" until the next reboot, because the backdoor module, for obvious reasons, is not in the main kernel tree.

## Made fully by
- [Lucas Eduardo Gulka Pulcinelli](https://github.com/lucasgpulcinelli)
- [Matheus Henrique Dias Cirillo](https://github.com/cirillom)
- [Carlos Henrique Craveiro Aquino Veras](https://github.com/CarlosCraveiro) 
