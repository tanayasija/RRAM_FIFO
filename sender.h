#ifndef sender_H
#define sender_H

#include <systemc>
using namespace std;
using namespace sc_core;
using namespace sc_dt;

SC_MODULE(sender)
{
	sc_out<bool> clk;
	sc_out<sc_bit> cs_pin;
	sc_port<sc_fifo_out_if<sc_bit> > bit_out;
	sc_port<sc_fifo_in_if<sc_bit> >  bit_in;

	sc_event clk_posedge;
	SC_CTOR(sender)
	{
		SC_THREAD(transciever);
		sensitive <<clk_posedge;
		SC_THREAD(clock);
		cs_pin.initialize(sc_bit('1'));
	}
	void transciever(void);
	void clock(void);
};
#endif
