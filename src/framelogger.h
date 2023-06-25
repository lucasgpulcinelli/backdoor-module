#ifndef __FRAMELOGGER_H__
#define __FRAMELOGGER_H__

/*
    Struct that represents the frame buffer.
    	@xres the x resolution of the frame
    	@yres the y resolution of the frame
    	@rgb_buffer the RGB values of the buffer
    
*/
struct frame_buffer {
	int xres;
	int yres;
	unsigned char* rgb_buffer;
};


// populate frame

// destroy frame

#endif /* __FRAMLOGGER_H__ */
