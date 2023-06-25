#ifndef __FRAMELOGGER_H__
#define __FRAMELOGGER_H__

/*
    Struct that represents the frame buffer.
    @attr xres the x resolution of the frame
    @attr yres the y resolution of the frame
    @attr rgb_buffer the RGB values of the buffer
    
*/
struct frame_buffer {
	int xres;
	int yres;
	unsigned char* rgb_buffer;
};

/*
    Function that records the frame buffer 0 and stores
    the information at the struct passed as parameter.

    IMPORTANT! Remember to free the rgb_buffer inside
    the fb struct! This is done by calling the
    clean_frame_buffer() after consuming the frame_buffer

    @param frame_buffer pointer to struct to be filled
*/
int record_frame_buffer(struct frame_buffer* fb);

/*
    Function that cleans the frame_buffer by freeing the
    rgb_buffer allocated by record_frame_buffer()
 */
void clean_frame_buffer(struct frame_buffer* fb);

#endif /* __FRAMLOGGER_H__ */
