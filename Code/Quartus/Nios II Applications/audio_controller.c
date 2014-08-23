#include <altera_avalon_mutex.h>
#include <altera_up_avalon_audio_and_video_config.h>
#include "altera_up_avalon_parallel_port.h"
#include "altera_up_avalon_audio.h"
#include "sys/alt_irq.h"
#include "alt_types.h"
#include "sys_common.h"
#include "sys_common.c"

/********************************************************************************
 * Audio Controller - Controls audio codec for signal acquisition and performs
 * feature extraction and classification of recorded waveforms.
********************************************************************************/

volatile char* wave_data_addr = 0x00082000;
const int WAVE_DATA_SIZE = 8192;

// Devices
alt_up_audio_dev 			*Audio_dev;
alt_mutex_dev 				*State_Mutex_Audio_dev;
alt_mutex_dev 				*Data_Mutex_dev;
alt_up_parallel_port_dev 	*Signal_User_Input_Audio_dev;
alt_up_parallel_port_dev 	*Signal_Data_Ready_dev;
alt_up_av_config_dev		*AV_Config_dev;

// Global variables 
volatile int visual_counter = 0;
volatile int recording_data[RECORD_SIZE];
volatile char analysis_data[RECORD_SIZE];
volatile int recording_index = 0;
volatile int wavedata_index = 0;
volatile int done_rec_flag = 0;
volatile int playback_valid = 0;
volatile int done_play_flag = 0;
volatile int done_analyzing = 0;

// Interrupt service routine
void interrupt_handler(void)
{
	int ipending = __builtin_rdctl(4);
	// IRQ0
	if ((ipending & 0x1) != 0) 	   
	{
		int change_mode_flag = 0;
		int change_cursor_flag = 0;
		
		altera_avalon_mutex_lock(State_Mutex_Audio_dev, AUDIO_CONTROLLER_ID);
			change_mode_flag = (((unsigned char) alt_up_parallel_port_read_edge_capture(Signal_User_Input_Audio_dev) & IRQ0_STATE) != 0x00) ? 1 : 0;
			change_cursor_flag = (((unsigned char) alt_up_parallel_port_read_edge_capture(Signal_User_Input_Audio_dev) & IRQ0_CURSOR) != 0x00) ? 1 : 0;
			sys_mode = *Shared_State_Addr;
			mode_cursor = *Shared_Cursor_Addr;
			if (change_mode_flag)
			{
				change_mode();
			}
			alt_up_parallel_port_clear_edge_capture(Signal_User_Input_Audio_dev);
		altera_avalon_mutex_unlock(State_Mutex_Audio_dev);
	}
	return;
}

int main(void)
{	
	// Open devices
	Audio_dev 					= alt_up_audio_open_dev 		("/dev/Audio");
	State_Mutex_Audio_dev 		= altera_avalon_mutex_open		("/dev/State_Mutex_Audio");
	Data_Mutex_dev 				= altera_avalon_mutex_open		("/dev/Data_Mutex");
	Signal_User_Input_Audio_dev = alt_up_parallel_port_open_dev ("/dev/Signal_User_Input_Audio");
	Signal_Data_Ready_dev 		= alt_up_parallel_port_open_dev ("/dev/Signal_Data_Ready");
	AV_Config_dev				= alt_up_av_config_open_dev("/dev/AV_Config");
	
	// Initialize interrupts
	alt_up_parallel_port_set_interrupt_mask(Signal_Data_Ready_dev, 0xff);
	
	// Set audio/video config
	alt_up_av_config_write_audio_cfg_register(AV_Config_dev, 0x00, 0x37);	// Line In
	alt_up_av_config_write_audio_cfg_register(AV_Config_dev, 0x01, 0x37);	// Line In
	alt_up_av_config_write_audio_cfg_register(AV_Config_dev, 0x02, 0x79);	// Line Out
	alt_up_av_config_write_audio_cfg_register(AV_Config_dev, 0x03, 0x79);	// Line Out
	//alt_up_av_config_write_audio_cfg_register(AV_Config_dev, 0x04, 0x14);	// ADC
	alt_up_av_config_write_audio_cfg_register(AV_Config_dev, 0x04, 0x24);
	//alt_up_av_config_write_audio_cfg_register(AV_Config_dev, 0x05, 0x06);	// DAC
	//alt_up_av_config_write_audio_cfg_register(AV_Config_dev, 0x08, 0x1c);	// Sampling Control
	
	// Enable and register interrupts 
	alt_irq_enable(0);
	alt_irq_register(0, NULL, interrupt_handler);
	
	while(1)
	{
		switch(sys_mode)
		{
			case MODE_LISTEN:
				exec_listen();
				break;
				
			case MODE_RECORD:				
				if (done_rec_flag == 0)
				{
					exec_record();
				}
				break;
				
			case MODE_PLAYBACK:
				if (playback_valid && done_play_flag == 0)
				{
					exec_playback();
				}
				break;
				
			case MODE_ANALYZE:
				if (playback_valid && done_analyzing == 0)
				{
					exec_analyze();
				}
				break;
				
			default:
				break;
		}		
	}
}

void change_mode(void)
{
	switch(sys_mode)
	{
		case MODE_LISTEN:
			alt_up_av_config_write_audio_cfg_register(AV_Config_dev, 0x04, 0x24);
			visual_counter = 0;
			break;
			
		case MODE_RECORD:
			alt_up_av_config_write_audio_cfg_register(AV_Config_dev, 0x04, 0x24);
			recording_index = 0;
			wavedata_index = 0;
			done_rec_flag = 0;
			visual_counter = 0;
			break;
			
		case MODE_PLAYBACK:
			alt_up_av_config_write_audio_cfg_register(AV_Config_dev, 0x04, 0x14);
			visual_counter = 0;
			done_play_flag = 0;
			break;
			
		case MODE_ANALYZE:
			done_analyzing = 0;
			break;
			
		default:
			break;
	}
}

void exec_listen(void)
{
	/* used for audio record/playback */
	int l_buf;
	int r_buf;
	
	// read audio buffer
	int num_read = alt_up_audio_record_r (Audio_dev, &(r_buf), 1);
	(void) alt_up_audio_record_l (Audio_dev, &(l_buf), 1);
	if ( num_read > 0 ) // check if data is available
	{			
		// write audio buffer
		//alt_up_audio_play_r (Audio_dev, &(r_buf), 1);
		//alt_up_audio_play_l (Audio_dev, &(r_buf), 1);
	
		// update visual counter
		visual_counter++;
		if (visual_counter == VIS_CNTR_MAX)
		{
			// send data to video controller
			altera_avalon_mutex_lock(Data_Mutex_dev, AUDIO_CONTROLLER_ID);
				*Shared_Data_Addr = (((unsigned char) (((char) (r_buf/256)) + 128)) >> 1) & 0x7f;
				trigger_interrupt(Signal_Data_Ready_dev, IRQ1_DATA);
			altera_avalon_mutex_unlock(Data_Mutex_dev);
			
			// reset counter
			visual_counter = 0;
		}
	}
}

void exec_record(void)
{
	/* used for audio record/playback */
	int l_buf;
	int r_buf;
	int num_read;
	int num_visualized;
	
	num_visualized = 0;
	while (recording_index < RECORD_SIZE)
	{
		// read audio buffer
		num_read = alt_up_audio_record_r (Audio_dev, &(r_buf), 1);
		(void) alt_up_audio_record_l (Audio_dev, &(l_buf), 1);
		if ( num_read > 0 ) // check if data is available
		{
			// update visual counter
			visual_counter++;
			if (visual_counter == VIS_CNTR_MAX && num_visualized < PLOT_RANGE_X * (RECORD_TIME / PLOT_RANGE_TIME)-1)
			{
				// send data to video controller
				altera_avalon_mutex_lock(Data_Mutex_dev, AUDIO_CONTROLLER_ID);
					*Shared_Data_Addr = (((unsigned char) (((char) (r_buf/256)) + 128)) >> 1) & 0x7f;
					trigger_interrupt(Signal_Data_Ready_dev, IRQ1_DATA);
				altera_avalon_mutex_unlock(Data_Mutex_dev);
				
				// reset counter
				visual_counter = 0;			
				num_visualized++;				
			}

			// store audio data
			if (recording_index%10 == 0 && wavedata_index < WAVE_DATA_SIZE)
			{
				wave_data_addr[wavedata_index++] = (char) (r_buf/256);
			}
			recording_data[recording_index++] = r_buf;			
		}
	}
	
	// notify video controller that recording is done
	done_rec_flag = 1;
	playback_valid = 1;
	altera_avalon_mutex_lock(Data_Mutex_dev, AUDIO_CONTROLLER_ID);
		trigger_interrupt(Signal_Data_Ready_dev, IRQ1_REC_DONE);
	altera_avalon_mutex_unlock(Data_Mutex_dev);
}

void exec_playback(void)
{
	int i;
	int num_visualized = 0;
	for (i=0; i<RECORD_SIZE; i++)
	{
		// write audio buffer
		alt_up_audio_play_r (Audio_dev, &(recording_data[i]), 1);
		alt_up_audio_play_l (Audio_dev, &(recording_data[i]), 1);
		alt_up_audio_play_r (Audio_dev, &(recording_data[i]), 1);
		alt_up_audio_play_l (Audio_dev, &(recording_data[i]), 1);
		
		// update visual counter
		visual_counter++;
		if (visual_counter == VIS_CNTR_MAX && num_visualized < PLOT_RANGE_X * (RECORD_TIME / PLOT_RANGE_TIME)-1)
		{
			// send data to video controller
			altera_avalon_mutex_lock(Data_Mutex_dev, AUDIO_CONTROLLER_ID);
				*Shared_Data_Addr = (((unsigned char) (((char) (recording_data[i]/256)) + 128)) >> 1) & 0x7f;
				trigger_interrupt(Signal_Data_Ready_dev, IRQ1_DATA);
			altera_avalon_mutex_unlock(Data_Mutex_dev);
			
			// reset counter
			visual_counter = 0;			
			num_visualized++;	
		}
	}
	
	done_play_flag = 1;
	altera_avalon_mutex_lock(Data_Mutex_dev, AUDIO_CONTROLLER_ID);
		trigger_interrupt(Signal_Data_Ready_dev, IRQ1_PLAY_DONE);
	altera_avalon_mutex_unlock(Data_Mutex_dev);
}

void exec_analyze(void)
{
	/* Feature Extraction */	
	// calculate rms value	
	int i;
	alt_u64 sum = 0;
	alt_u64 rms_sq = 0;
	char value = 0;
	for (i=0; i<wavedata_index; i++)
	{
		value = wave_data_addr[i];
		sum += value * value;
	}
	rms_sq = sum/wavedata_index;
				
	// calculate low energy rate
	unsigned char low_energy_rate = 0;
	unsigned int low_count = 0;
	for (i=0; i<wavedata_index; i++)
	{
		value = wave_data_addr[i];
		if (value * value < rms_sq)
		{
			low_count++;
		}
	}
	low_energy_rate = (low_count * 100) / wavedata_index;
	
	/* Classification */
	unsigned char classification;
	if (low_energy_rate <= SVM_BOUNDARY)
	{
		classification = CLASS_MURMUR;
	}
	else
	{
		classification = CLASS_NORMAL;
	}
	
	done_analyzing = 1;
	altera_avalon_mutex_lock(Data_Mutex_dev, AUDIO_CONTROLLER_ID);
		*Shared_Feature_Addr = low_energy_rate; 
		*Shared_Classification_Addr = classification;
		trigger_interrupt(Signal_Data_Ready_dev, IRQ1_ANALYSIS_DONE);
	altera_avalon_mutex_unlock(Data_Mutex_dev);
}
