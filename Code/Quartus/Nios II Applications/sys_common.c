// Trigger interrupt - proc must obtain appropriate mutex lock BEFORE calling this function
void trigger_interrupt(alt_up_parallel_port_dev *signal, unsigned char mask)
{
	alt_up_parallel_port_write_data(signal, 0x00);
	alt_up_parallel_port_write_data(signal, mask);	
}