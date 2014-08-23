#include <altera_avalon_mutex.h>
#include "altera_up_avalon_parallel_port.h"
#include "sys/alt_irq.h"
#include "sys_common.h"
#include "sys_common.c"

/********************************************************************************
 * User Interface Controller - Contains state machine logic and monitors user
 * inputs.
********************************************************************************/

// Devices
alt_up_parallel_port_dev *Pushbuttons_dev;
alt_up_parallel_port_dev *Green_LEDs_dev;
alt_up_parallel_port_dev *Red_LEDs_dev;
alt_up_parallel_port_dev *Switches_dev;
alt_up_parallel_port_dev *Signal_User_Input_Video_dev;
alt_up_parallel_port_dev *Signal_User_Input_Audio_dev;
alt_mutex_dev 			 *State_Mutex_Video_dev;
alt_mutex_dev 			 *State_Mutex_Audio_dev;

int main(void)
{		
	// Open devices
	Pushbuttons_dev 			= alt_up_parallel_port_open_dev ("/dev/Pushbuttons");
	Green_LEDs_dev 				= alt_up_parallel_port_open_dev ("/dev/Green_LEDs");
	Red_LEDs_dev 				= alt_up_parallel_port_open_dev ("/dev/Red_LEDs");
	Switches_dev 				= alt_up_parallel_port_open_dev ("/dev/Switches");
	Signal_User_Input_Video_dev = alt_up_parallel_port_open_dev ("/dev/Signal_User_Input_Video");
	Signal_User_Input_Audio_dev = alt_up_parallel_port_open_dev ("/dev/Signal_User_Input_Audio");
	State_Mutex_Video_dev 		= altera_avalon_mutex_open		("/dev/State_Mutex_Video");
	State_Mutex_Audio_dev 		= altera_avalon_mutex_open		("/dev/State_Mutex_Audio");
	
	// Initialize interrupts
	alt_up_parallel_port_set_interrupt_mask(Signal_User_Input_Video_dev, 0xff);
	alt_up_parallel_port_set_interrupt_mask(Signal_User_Input_Audio_dev, 0xff);
	
	// Set green led indicators to ON
	alt_up_parallel_port_write_data(Green_LEDs_dev, 0xff);	
	
	while(1)
	{		
		// Check status of pushbuttons
		int key = check_keys();
		switch(key)
		{
			// Previous
			case KEY0:	
				altera_avalon_mutex_lock(State_Mutex_Video_dev, STATE_CONTROLLER_ID);
				altera_avalon_mutex_lock(State_Mutex_Audio_dev, STATE_CONTROLLER_ID);
					// update cursor
					mode_cursor = ((mode_cursor) + NUM_MODES - 1) % NUM_MODES;
					*Shared_Cursor_Addr = (unsigned char) mode_cursor;
					trigger_interrupt(Signal_User_Input_Video_dev, IRQ0_CURSOR);
					trigger_interrupt(Signal_User_Input_Audio_dev, IRQ0_CURSOR);
				altera_avalon_mutex_unlock(State_Mutex_Audio_dev);
				altera_avalon_mutex_unlock(State_Mutex_Video_dev);
				break;
			
			// Next
			case KEY1:
				altera_avalon_mutex_lock(State_Mutex_Video_dev, STATE_CONTROLLER_ID);
				altera_avalon_mutex_lock(State_Mutex_Audio_dev, STATE_CONTROLLER_ID);
					// update cursor
					mode_cursor = ((mode_cursor) + 1) % NUM_MODES;
					*Shared_Cursor_Addr = (unsigned char) mode_cursor;
					trigger_interrupt(Signal_User_Input_Video_dev, IRQ0_CURSOR);
					trigger_interrupt(Signal_User_Input_Audio_dev, IRQ0_CURSOR);
				altera_avalon_mutex_unlock(State_Mutex_Audio_dev);
				altera_avalon_mutex_unlock(State_Mutex_Video_dev);
				break;
			
			// Select
			case KEY2:	
				altera_avalon_mutex_lock(State_Mutex_Video_dev, STATE_CONTROLLER_ID);
				altera_avalon_mutex_lock(State_Mutex_Audio_dev, STATE_CONTROLLER_ID);
					// change mode
					sys_mode = mode_cursor;
					*Shared_State_Addr = (unsigned char) sys_mode;
					trigger_interrupt(Signal_User_Input_Video_dev, IRQ0_STATE);
					trigger_interrupt(Signal_User_Input_Audio_dev, IRQ0_STATE);
				altera_avalon_mutex_unlock(State_Mutex_Audio_dev);
				altera_avalon_mutex_unlock(State_Mutex_Video_dev);
				break;
			
			// Reset
			case KEY3:	
				break;
			default:
				break;
		}
	}
}

// Read pushbutton keys
int check_keys()
{
	int key, read_data, button_mask;
	button_mask = alt_up_parallel_port_read_data(Pushbuttons_dev);			// read the pushbutton KEY values
	while (button_mask && alt_up_parallel_port_read_data(Pushbuttons_dev));	// wait for pushbutton KEY release
	
	switch(button_mask)
	{
		case (1 << KEY0):
			key = KEY0;
			read_data = alt_up_parallel_port_read_data(Red_LEDs_dev) & 0x1;
			alt_up_parallel_port_write_data(Red_LEDs_dev, read_data ^ 0x1);
			break;
		case (1 << KEY1):
			key = KEY1;
			read_data = alt_up_parallel_port_read_data(Red_LEDs_dev) & 0x2;
			alt_up_parallel_port_write_data(Red_LEDs_dev, read_data ^ 0x2);
			break;
		case (1 << KEY2):
			key = KEY2;
			read_data = alt_up_parallel_port_read_data(Red_LEDs_dev) & 0x4;
			alt_up_parallel_port_write_data(Red_LEDs_dev, read_data ^ 0x4);
			break;
		case (1 << KEY3):
			key = KEY3;
			read_data = alt_up_parallel_port_read_data(Red_LEDs_dev) & 0x8;
			alt_up_parallel_port_write_data(Red_LEDs_dev, read_data ^ 0x8);
			break;
		default:
			key = -1;
			break;
	}
	
	return key;
}