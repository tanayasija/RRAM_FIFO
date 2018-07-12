#include "sender.h"

void sender::transciever()
{
	cs_pin->write(sc_bit('1'));
	wait();
	wait(5,SC_NS);
	cs_pin->write(sc_bit('0'));
	for (int i=0;i<7;i++){
		wait();
		bit_out->write(sc_bit('1'));
	}
	wait();
	bit_out->write(sc_bit('0'));
	wait(5,SC_NS);
	cs_pin->write(sc_bit('1'));
	cout << "1 " <<  sc_time_stamp() << endl;
	wait();
	wait(5,SC_NS);
	cs_pin->write(sc_bit('0'));
	sc_bv<8> read_ins = "11000000";
	for (int i=0;i<8;i++){
		wait();
		sc_bit r = (sc_bit)read_ins[i];
		bit_out->write(r);
	}
	cout << "2 " <<  sc_time_stamp() << endl;
	for (int i=0;i<24;i++){
		wait();
		bit_out->write(sc_bit('1'));
	}
	cout << sc_time_stamp() << endl;
	for (int i=0;i<24;i++){
		if (bit_in->num_available())
		{
			sc_bit d = bit_in->read();
			cout << "Bit " << i << " read at time "<< sc_time_stamp() << d << endl;
		}
		else
		{	
			wait(bit_in->data_written_event());
			sc_bit d = bit_in->read();
			cout << "Bit " << i << " read at time " << sc_time_stamp()  << d  << endl;
		}
		if (i==20)
		{
			wait(5,SC_NS);
			cs_pin->write(sc_bit('1'));
			break;	
		}			
	}
	wait();
	wait(5,SC_NS);
	cs_pin->write(sc_bit('0'));
	sc_bv<8> write_enable_ins = "01100000";
	for (int i=0;i<8;i++){
		wait();
		sc_bit r = (sc_bit)write_enable_ins[i];
		bit_out->write(r);
	}
	cs_pin->write(sc_bit('1'));
	wait();
	wait(5,SC_NS);
	cs_pin->write(sc_bit('0'));
	sc_bv<8> write_ins = "01000000";
	for (int i=0;i<8;i++){
		wait();
		sc_bit r = (sc_bit)write_ins[i];
		bit_out->write(r);
	}
	cout <<  sc_time_stamp() << endl;
	for (int i=0;i<24;i++){
		wait();
		bit_out->write(sc_bit('1'));
	}
	cout << sc_time_stamp() << endl;
	for (int i=0;i<8;i++){
		wait();
		sc_bit r = (sc_bit)write_ins[i];
		bit_out->write(r);
	}
	wait(5,SC_NS);
	cs_pin->write(sc_bit('1'));
	wait();
	wait(5,SC_NS);
	cs_pin->write(sc_bit('0'));
	for (int i=0;i<8;i++){
		cout << i << endl ;
		wait();
		sc_bit r = (sc_bit)write_enable_ins[i];
		if (bit_out->num_free())
		{
			bit_out->write(r);
		}
		else
		{
			wait(bit_out->data_read_event());
			bit_out->write(r);
		}
	}
	wait(5,SC_NS);
	cs_pin->write(sc_bit('1'));
	sc_bv<8> ins_read_status = "10100000";
	wait();
	wait(5,SC_NS);
	cs_pin->write(sc_bit('0'));
	for (int i=0;i<8;i++){
		cout << i << endl ;
		wait();
		sc_bit r = (sc_bit)ins_read_status[i];
		if (bit_out->num_free())
		{
			bit_out->write(r);
		}
		else
		{
			wait(bit_out->data_read_event());
			bit_out->write(r);
		}
	}
	for (int i=0;i<8;i++)
	{
		static sc_bv<1> d_vec;
		if (bit_in->num_available())
		{
			sc_bit d = bit_in->read();
			d_vec[0]=d;
			cout << "1. Status bit " << i << " read at time "<< sc_time_stamp() << "is" << d_vec.to_string() << "boolean" << endl;
		}
		else
		{	
			wait(bit_in->data_written_event());
			sc_bit d = bit_in->read();
			d_vec[0]=d;
			cout << "2. Status bit " << i << " read at time "<< sc_time_stamp() << "is " << d_vec.to_string() << " boolean" << endl;
		}
	}
	wait();
	wait(5,SC_NS); 
	cs_pin->write(sc_bit('1'));
	wait();
	wait(5,SC_NS);
	cs_pin->write(sc_bit('0'));
	for (int i=0;i<8;i++){
		cout << i << endl ;
		wait();
		sc_bit r = (sc_bit)ins_read_status[i];
		if (bit_out->num_free())
		{
			bit_out->write(r);
		}
		else
		{
			wait(bit_out->data_read_event());
			bit_out->write(r);
		}
	}
	for (int i=0;i<8;i++)
	{
		static sc_bv<1> d_vec;
		if (bit_in->num_available())
		{
			sc_bit d = bit_in->read();
			d_vec[0]=d;
		cout << "1. Status bit " << i << " read at time "<< sc_time_stamp() << " is " << d_vec.to_string() << " boolean" << endl;
		}
		else
		{	
			wait(bit_in->data_written_event());
			sc_bit d = bit_in->read();
			d_vec[0]=d;
			cout << "2. Status bit " << i << " read at time "<< sc_time_stamp() << " is " << d_vec.to_string() << " boolean" << endl;
		}
	}
	cout << sc_time_stamp() << endl;
}

void sender::clock(void)
{
	for(int i=0;i<26000;i++)
	{
		clk->write(0);
		wait(10,SC_NS);
		clk->write(1);	
		clk_posedge.notify();
		wait(10,SC_NS);
	}
}
