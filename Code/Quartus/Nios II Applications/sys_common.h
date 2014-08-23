#ifndef SYS_COMMON_H
#define SYS_COMMON_H

/* Operation Modes */
#define MODE_LISTEN		0
#define MODE_RECORD		1
#define MODE_PLAYBACK	2
#define MODE_ANALYZE	3
#define NUM_MODES		4

/* Keybuttons */
#define KEY0	0
#define KEY1	1
#define KEY2	2
#define KEY3	3

/* Shared Resources */
#define Shared_State_Addr 			(volatile unsigned char *) 0x000800c0
#define Shared_Cursor_Addr			(volatile unsigned char *) 0x000800e0
#define Shared_Data_Addr			(volatile unsigned char *) 0x00080120
#define Shared_Feature_Addr 		(volatile unsigned char *) 0x00080160
#define Shared_Classification_Addr  (volatile unsigned char *) 0x00080180

/* Proc IDs */
#define STATE_CONTROLLER_ID		1
#define VIDEO_CONTROLLER_ID		2
#define AUDIO_CONTROLLER_ID		3

/* Interrupt Masks */
#define IRQ0_STATE		0x01
#define IRQ0_CURSOR		0x02

#define IRQ1_DATA			0x01
#define IRQ1_REC_DONE		0x02
#define IRQ1_PLAY_DONE		0x04
#define IRQ1_ANALYSIS_DONE	0x08

/* Audio Constants */
#define SAMP_FREQ		32000	// hz
#define RECORD_SIZE		77838	// samples	
#define RECORD_TIME		6		// seconds	
#define SVM_BOUNDARY	86
#define CLASS_NORMAL	0
#define CLASS_MURMUR	1

/* Video Constants */
#define PLOT_RANGE_TIME	2		// seconds
#define PLOT_RANGE_X	250		// pixels
#define PLOT_RANGE_Y	128		// pixels
#define VIS_CNTR_MAX	103

/* Global Variables */
volatile unsigned char sys_mode = MODE_LISTEN;
volatile unsigned char mode_cursor = MODE_LISTEN;

#endif