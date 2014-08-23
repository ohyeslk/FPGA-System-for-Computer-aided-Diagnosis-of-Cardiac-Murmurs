#include <altera_avalon_mutex.h>
#include "altera_up_avalon_parallel_port.h"
#include "altera_up_avalon_video_pixel_buffer_dma.h"
#include "altera_up_avalon_video_character_buffer_with_dma.h"
#include "sys/alt_irq.h"
#include "sys_common.h"
#include "sys_common.c"

/********************************************************************************
 * Video Controller - Controls VGA interface for data visualization.
********************************************************************************/

const char STATUS_LISTENING[] 	 	= "Listening...         ";
const char STATUS_RECORDING[] 	 	= "Recording...         ";
const char STATUS_RECORD_DONE[]  	= "Recording...DONE     ";
const char STATUS_PLAYING[] 	 	= "Playing...           ";
const char STATUS_PLAYING_DONE[] 	= "Playing...DONE       ";
const char STATUS_ANALYZING[] 	 	= "Analyzing...         ";
const char STATUS_ANALYZING_DONE[] 	= "Analyzing...DONE     ";

const char CLASS_NORMAL_STRING[]	= "Normal          ";
const char CLASS_MURMUR_STRING[]	= "Murmur          ";

/* devices */
alt_up_pixel_buffer_dma_dev *pixel_buffer_dev;
alt_up_char_buffer_dev      *char_buffer_dev;
alt_up_parallel_port_dev 	*Signal_User_Input_Video_dev;
alt_up_parallel_port_dev 	*Signal_Data_Ready_dev;
alt_mutex_dev 				*State_Mutex_Video_dev;
alt_mutex_dev 				*Data_Mutex_dev;

/* global variables */
int plot_cursor = 0;
int plot_cursor_val = 64;
int plot_empty_flag = 1;

/* interrupt service routine */
void interrupt_handler(void)
{
	int ipending = __builtin_rdctl(4);
	// IRQ0
	if ((ipending & 0x1) != 0) 	   
	{
		int mode_flag = 0;
		int cursor_flag = 0;
		
		altera_avalon_mutex_lock(State_Mutex_Video_dev, VIDEO_CONTROLLER_ID);
			mode_flag = (((unsigned char) alt_up_parallel_port_read_edge_capture(Signal_User_Input_Video_dev) & IRQ0_STATE) != 0x00) ? 1 : 0;
			cursor_flag = (((unsigned char) alt_up_parallel_port_read_edge_capture(Signal_User_Input_Video_dev) & IRQ0_CURSOR) != 0x00) ? 1 : 0;
			sys_mode = *Shared_State_Addr;
			mode_cursor = *Shared_Cursor_Addr;
			alt_up_parallel_port_clear_edge_capture(Signal_User_Input_Video_dev);
		altera_avalon_mutex_unlock(State_Mutex_Video_dev);		
		
		if (mode_flag)
		{
			change_mode();		
		}
		if (cursor_flag)
		{
			change_cursor();
		}
	}
	// IRQ1
	if ((ipending & 0x2) != 0)
	{
		int data_flag = 0;
		int rec_done_flag = 0;
		int play_done_flag = 0;
		int analysis_done_flag = 0;
		
		altera_avalon_mutex_lock(Data_Mutex_dev, VIDEO_CONTROLLER_ID);
			data_flag = (((unsigned char) alt_up_parallel_port_read_edge_capture(Signal_Data_Ready_dev) & IRQ1_DATA) != 0x00) ? 1 : 0;
			rec_done_flag = (((unsigned char) alt_up_parallel_port_read_edge_capture(Signal_Data_Ready_dev) & IRQ1_REC_DONE) != 0x00) ? 1 : 0;
			play_done_flag = (((unsigned char) alt_up_parallel_port_read_edge_capture(Signal_Data_Ready_dev) & IRQ1_PLAY_DONE) != 0x00) ? 1 : 0;
			analysis_done_flag = (((unsigned char) alt_up_parallel_port_read_edge_capture(Signal_Data_Ready_dev) & IRQ1_ANALYSIS_DONE) != 0x00) ? 1 : 0;
			if (data_flag)
			{
				draw_data_point(*Shared_Data_Addr);
			}			
			if (analysis_done_flag)
			{
				draw_results((unsigned char) *Shared_Feature_Addr, (unsigned char) *Shared_Classification_Addr);
				draw_status(STATUS_ANALYZING_DONE);
			}
			alt_up_parallel_port_clear_edge_capture(Signal_Data_Ready_dev);
		altera_avalon_mutex_unlock(Data_Mutex_dev);
		
		if (rec_done_flag) 
		{
			draw_status(STATUS_RECORD_DONE);
		}
		if (play_done_flag)
		{
			draw_status(STATUS_PLAYING_DONE);
		}		
	}
	return;
}

int main(void)
{		
	/* used for drawing coordinates */
	int x1, y1, x2, y2 = 0;
	
	/* initialize devices */
	pixel_buffer_dev 			= alt_up_pixel_buffer_dma_open_dev	("/dev/Pixel_Buffer_DMA");
	char_buffer_dev 			= alt_up_char_buffer_open_dev 		("/dev/Char_Buffer_with_DMA");
	Signal_User_Input_Video_dev = alt_up_parallel_port_open_dev 	("/dev/Signal_User_Input_Video");
	Signal_Data_Ready_dev 		= alt_up_parallel_port_open_dev 	("/dev/Signal_Data_Ready");
	State_Mutex_Video_dev 		= altera_avalon_mutex_open			("/dev/State_Mutex_Video");
	Data_Mutex_dev 				= altera_avalon_mutex_open			("/dev/Data_Mutex");
	
	/* enable and register interrupts */
	alt_irq_enable(0);
	alt_irq_enable(1);
	alt_irq_register(0, NULL, interrupt_handler);
	alt_irq_register(1, NULL, interrupt_handler);
	
	/* clear the screen */
	alt_up_char_buffer_clear(char_buffer_dev);
	alt_up_pixel_buffer_dma_clear_screen(pixel_buffer_dev, 0);
		
	/* draw shapes */
	draw_plot();
	draw_controls();
	draw_mode();
	draw_description();
	draw_status(STATUS_LISTENING);
	
	while(1)
	{
	}
}

/* called by ISR when a mode change occurs */
void change_mode(void)
{
	switch(sys_mode)
	{
		case MODE_LISTEN:
			// reset cursor
			reset_cursor();
			
			// draw shapes
			draw_plot();
			draw_controls();
			draw_mode();
			draw_description();
			draw_status(STATUS_LISTENING);
			break;
			
		case MODE_RECORD:
			// reset cursor
			reset_cursor();
			
			// draw shapes
			draw_plot();
			draw_controls();
			draw_mode();
			draw_description();
			draw_status(STATUS_RECORDING);
			break;
			
		case MODE_PLAYBACK:
			// reset cursor
			reset_cursor();
			
			// draw shapes
			draw_plot();
			draw_controls();
			draw_mode();
			draw_description();
			draw_status(STATUS_PLAYING);
			break;
			
		case MODE_ANALYZE:
			// reset cursor
			reset_cursor();
			
			// draw shapes
			draw_plot();
			draw_controls();
			draw_mode();
			draw_description();
			draw_status(STATUS_ANALYZING);
			draw_analysis();
			break;
			
		default:
			break;
	}	
}

/* called by ISR when a cursor change occurs */
void change_cursor(void)
{
	/* draw shapes */
	draw_mode();
	draw_description();
}

/* resets cursor position and value */
void reset_cursor(void)
{
	plot_cursor = 0;
	plot_cursor_val = 64;
	plot_empty_flag = 1;
}

/* draws data point */
void draw_data_point(unsigned char data)
{
	// plot is empty
	if (plot_empty_flag)
	{
		alt_up_pixel_buffer_dma_draw(pixel_buffer_dev, 0xFFFF, 33, (141-data));
		plot_cursor = 0;
		plot_cursor_val = data;
		plot_empty_flag = 0;
	}
	// plot contains data, but is not full
	if (plot_cursor < PLOT_RANGE_X-1)
	{
		// draw line connecting new data point
		alt_up_pixel_buffer_dma_draw_line(pixel_buffer_dev, (33+plot_cursor), (141-plot_cursor_val), (33+plot_cursor+1), (141-data), 0xFFFF, 0);
		
		// update plot cursor
		plot_cursor++;
		plot_cursor_val = data;
	}	
	// plot is full
	else
	{
		// clear plot
		alt_up_pixel_buffer_dma_draw_box(pixel_buffer_dev, 33, 14, 282, 141, 0, 0);
		
		// draw new point
		alt_up_pixel_buffer_dma_draw(pixel_buffer_dev, 0xFFFF, 33, (141-data));
		
		// reset plot cursor
		plot_cursor = 0;
		plot_cursor_val = data;
	}
}

/* draws plot */
void draw_plot(void)
{
	/* draw plot title */
	alt_up_char_buffer_string(char_buffer_dev, "Cardiac Signal Analyzer", 28, 2);
	
	/* draw plot axis labels */
	alt_up_char_buffer_string(char_buffer_dev, "0V", 5, 3+32);
	alt_up_char_buffer_string(char_buffer_dev, "3.3V", 3, 3);
	alt_up_char_buffer_string(char_buffer_dev, "0S", 7, 37);
	if (sys_mode == MODE_RECORD)
	{
		alt_up_char_buffer_string(char_buffer_dev, "4S", 7+63, 37);
	}
	else
	{
		alt_up_char_buffer_string(char_buffer_dev, "2S", 7+63, 37);
	}
	
	/* draw plot borders */
	alt_up_pixel_buffer_dma_draw_hline(pixel_buffer_dev, 32,  283, 13,  0xFFFF, 0);
	alt_up_pixel_buffer_dma_draw_hline(pixel_buffer_dev, 32,  283, 142, 0xFFFF, 0);
	alt_up_pixel_buffer_dma_draw_vline(pixel_buffer_dev, 32,  13,  142, 0xFFFF, 0);
	alt_up_pixel_buffer_dma_draw_vline(pixel_buffer_dev, 283, 13,  142, 0xFFFF, 0);
	
	// clear plot data
	alt_up_pixel_buffer_dma_draw_box(pixel_buffer_dev, 33, 14, 282, 141, 0, 0);
}

/* draws control box */
void draw_controls(void)
{
	/* draw controls text */
	alt_up_char_buffer_string(char_buffer_dev, "Controls", 7, 40);	
	alt_up_char_buffer_string(char_buffer_dev, " Key0: Previous", 7, 42);
	alt_up_char_buffer_string(char_buffer_dev, " Key1: Next    ", 7, 43);
	alt_up_char_buffer_string(char_buffer_dev, " Key2: Select  ", 7, 44);
	alt_up_char_buffer_string(char_buffer_dev, " Key3: Reset   ", 7, 45);
	
	/* draw controls box */
	alt_up_pixel_buffer_dma_draw_rectangle(pixel_buffer_dev, 27, 166, 27+65, 166+19, 0xFFFF, 0);
}

/* draws mode box */
void draw_mode(void)
{	
	/* draw mode text */
	alt_up_char_buffer_string(char_buffer_dev, "Mode", 25, 40);
	alt_up_char_buffer_string(char_buffer_dev, "  Listen", 25, 42);
	alt_up_char_buffer_string(char_buffer_dev, "  Record", 25, 43);
	alt_up_char_buffer_string(char_buffer_dev, "  Playback", 25, 44);
	alt_up_char_buffer_string(char_buffer_dev, "  Analyze", 25, 45);
	
	/* draw cursor */
	alt_up_char_buffer_string(char_buffer_dev, "*", 26, 42+mode_cursor);
	
	/* draw mode box */
	alt_up_pixel_buffer_dma_draw_rectangle(pixel_buffer_dev, 99, 166, 99+45, 166+19, 0xFFFF, 0);
}

/* draws description box */
void draw_description(void)
{
	/* draw description text */
	alt_up_char_buffer_string(char_buffer_dev, "Mode Description                  ", 38, 40);
	switch(mode_cursor)
	{
		case MODE_LISTEN:
			alt_up_char_buffer_string(char_buffer_dev, " Listen to real-time audio signal ", 38, 42);
			alt_up_char_buffer_string(char_buffer_dev, " from digital stethoscope.        ", 38, 43);
			alt_up_char_buffer_string(char_buffer_dev, "                                  ", 38, 44);
			alt_up_char_buffer_string(char_buffer_dev, "                                  ", 38, 45);
			break;
		case MODE_RECORD:
			alt_up_char_buffer_string(char_buffer_dev, " Record signal and store in memory", 38, 42);
			alt_up_char_buffer_string(char_buffer_dev, " for future playback or analysis. ", 38, 43);
			alt_up_char_buffer_string(char_buffer_dev, "                                  ", 38, 44);
			alt_up_char_buffer_string(char_buffer_dev, "                                  ", 38, 45);
			break;
		case MODE_PLAYBACK:
			alt_up_char_buffer_string(char_buffer_dev, " Playback last recorded signal.   ", 38, 42);
			alt_up_char_buffer_string(char_buffer_dev, "                                  ", 38, 43);
			alt_up_char_buffer_string(char_buffer_dev, "                                  ", 38, 44);
			alt_up_char_buffer_string(char_buffer_dev, "                                  ", 38, 45);
			break;
		case MODE_ANALYZE:
			alt_up_char_buffer_string(char_buffer_dev, " Analyze the last recorded signal.", 38, 42);
			alt_up_char_buffer_string(char_buffer_dev, " Low Energy Rate and heart murmur ", 38, 43);
			alt_up_char_buffer_string(char_buffer_dev, " diagnosis are reported.          ", 38, 44);
			alt_up_char_buffer_string(char_buffer_dev, "                                  ", 38, 45);
			break;
		default:
			break;
	}
	
	/* draw description box */
	alt_up_pixel_buffer_dma_draw_rectangle(pixel_buffer_dev, 151, 166, 291, 166+19, 0xFFFF, 0);	
}

/* draw status box */
void draw_status(char *message)
{
	alt_up_char_buffer_string(char_buffer_dev, "Status: ", 7, 48);
	alt_up_char_buffer_string(char_buffer_dev, message, 15, 48);
}

/* draws analysis box */
void draw_analysis(void)
{
	/* draw analysis text */
	alt_up_char_buffer_string(char_buffer_dev, "Signal Analysis                   ", 38, 48);
	alt_up_char_buffer_string(char_buffer_dev, " Low Energy Rate: <calculating>   ", 38, 50);
	alt_up_char_buffer_string(char_buffer_dev, "       Diagnosis: <calculating>   ", 38, 51);
	
	/* draw analysis box */
	alt_up_pixel_buffer_dma_draw_rectangle(pixel_buffer_dev, 151, 198, 291, 198+11, 0xFFFF, 0);	
}

/* draw results */
void draw_results(unsigned char feature, unsigned char classification)
{
	char feature_string[17];
	char ones_digit = '0' + (feature%10);
	char tens_digit = '0' + (feature/10);
	
	if (feature == 0)
	{
		memcpy(feature_string, "0%              ", 17);
	}
	else if (feature == 100)
	{
		memcpy(feature_string, "100%            ", 17);
	}
	else
	{
		memcpy(feature_string, &tens_digit, 1);
		memcpy(feature_string+1, &ones_digit, 1);
		memcpy(feature_string+2, "%             ", 15);
	}
	
	alt_up_char_buffer_string(char_buffer_dev, feature_string, 56, 50);
	if (classification == CLASS_NORMAL)
	{
		alt_up_char_buffer_string(char_buffer_dev, CLASS_NORMAL_STRING, 56, 51);
	}
	else
	{
		alt_up_char_buffer_string(char_buffer_dev, CLASS_MURMUR_STRING, 56, 51);
	}	
}