#include "RRAM.h"
#include "sender.h"
#include <cmath>

void RRAM::start()
{
	cs_val = cs_p->read();
	if (!cs_low & cs_val==SC_LOGIC_0)
	{
		cout << "CS set low" << endl;
		cs_low= true;
		ins_read.notify();
	}
	else if (cs_low & cs_val==SC_LOGIC_1)
	{
		cout << "CS set high" << endl;
		cs_low=false;
	}
	cs_p->default_event();
	next_trigger();
}

void RRAM::read_instruction()
{
	for (;;)
	{
		cout << "Reading instruction" << endl;
		int i=0;
		while (cs_low == true && i<8)
		{
			cout << "Reading instruction bit " << i+1  << endl;
			if (data_in_p->num_available())
			{
				instruction[7-i]=data_in_p->read();
			}
			else
			{
				cout << "Waiting for data" << endl;
				wait(data_in_p->data_written_event());
				cout << "Data recieved" << endl;
				cout << data_in_p->num_available() << endl ;
				instruction[7-i]=data_in_p->read();
				cout << "Data read" << endl;
			}
			data_in_p_trace.write(sc_bit(instruction[7-i]));
			i++;
			cout << "Instruction bit " << i << " read" << endl;
		}		
		cout << "Instruction read " << instruction.to_string(SC_BIN) << endl;

		if (i==8)
		{
			if (instruction.to_string(SC_BIN)==instruction_read)
			{	
				if (status_register_1[0]==SC_LOGIC_0)
				{
					begin_read.notify();
				}
				else
				{
					cout << "Instruction ignored" << endl;
					begin_ignore_data_in_p.notify();
				}			
			}
			else if (instruction.to_string(SC_BIN)==instruction_write_enable)
			{
				if (status_register_1[0]==SC_LOGIC_0)
				{
					begin_write_enable.notify();
					cout << "Write enable instruction read" << endl;
				}
				else
				{
					cout << "Instruction ignored" << endl;
					begin_ignore_data_in_p.notify();
				}
			} 
			else if (instruction.to_string(SC_BIN)==instruction_program_page)
			{
				if (status_register_1[1]==1 && status_register_1[0]==SC_LOGIC_0)
				{
					begin_program_page.notify();
				}
				else
				{
					cout << "Instruction ignored" << endl;
					begin_ignore_data_in_p.notify();
				}
			}
			else if (instruction.to_string(SC_BIN)==instruction_page_write)
			{
				if (status_register_1[1]==SC_LOGIC_1 &&  status_register_1[0]==SC_LOGIC_0)
				{
					begin_page_write.notify();
					cout << "Write enable instruction read" << endl;
				}
				else
				{
					cout << "Instruction ignored" << endl;
					begin_ignore_data_in_p.notify();
				}
			} 
			else if (instruction.to_string(SC_BIN)==instruction_page_erase)
			{
				if (status_register_1[1]==1 && status_register_1[0]==SC_LOGIC_0)
				{
					begin_page_erase.notify();
				}
				else
				{
					cout << "Instruction ignored" << endl;
					begin_ignore_data_in_p.notify();
				}
			}
			else if (instruction.to_string(SC_BIN)==instruction_read_status_register)
			{
				begin_read_status_register.notify();
			}
			else
			{
				cout << "Instruction not recognized, ignoring instruction" << endl;
			} 
		}
		else
		{
			cout << "Instruction not recognized, ignoring instruction" << endl;
		}
		wait();
	}
}

void RRAM::read_data()
{	
	for(;;)
	{
		wait(begin_read);
		cout << "Page read started" << endl;
		int j=0;
		address_int = 0;

        while(cs_low && j<24)
        {
            if (data_in_p->num_available())
            {
                address[23-j]=data_in_p->read();
            }
            else
            {
                wait(data_in_p->data_written_event());
                address[23-j]=data_in_p->read();
            }
            j++;
        }
        if (cs_low && j==24)
        {
            for (int k=0;k<16;k++)
            {
				sc_bit add_bit = (sc_bit)address[k];
				if (add_bit==SC_LOGIC_1)
                {	
					address_int +=(int)pow(2.0,15-k) ;
            	}
	   		}	
		}
		cout << "Starting read operation" << endl;	
		int row=address_int/256;
		cout << row << endl;
		int col=address_int%256;
		int i=col*8;
		cout << "Sending data at location " << endl; cout<< "Row " << setw(2) << "Column " << setw(2) << "Data" << endl;		
		cout << row << " " << col << " " << i << endl;	
		while (cs_low)
		{  
			cout << "Waiting for negative clock edge" << endl;
			wait(clk_p->negedge_event() | cs_p.default_event());
			if (cs_low==false)
			{
				break;
			}
			bit = data[row][i];
			cout << setw(3) <<  row << setw(8) << i << setw(6) << bit << endl;
			data_out_p->write(bit);
			i++;
			if (2048<=i)
			{
				i=0;
				cout << "Moving to next row" << endl ;
				++row;		
			}
			if (256<=row)
			{
				cout << "Moving to the first cell" << endl ;
				row=0;
			}		
		}
		cout << "CS set HIGH, ending read operation" << endl;		
	}
}

void RRAM::write_enable(void)
{
	for(;;)
	{
		wait(begin_write_enable);
		status_register_1[1]=SC_LOGIC_1;
		cout << "Write enable bit set" << endl;
	}
}

void RRAM::program_page(void)
{
	for(;;)
	{	
		wait(begin_program_page);
		cout << "Page program started" << endl;
		int j=0;
		address_int = 0;

        while(cs_low && j<24)
        {
            if (data_in_p->num_available())
            {
                address[23-j]=data_in_p->read();
            }
            else
            {
                wait(data_in_p->data_written_event());
                address[23-j]=data_in_p->read();
            }
            j++;
        }
        if (cs_low && j==24)
        {
            for (int k=0;k<16;k++)
            {
				sc_bit add_bit = (sc_bit)address[k];
				if (add_bit==SC_LOGIC_1)
                {	
					address_int +=(int)pow(2.0,15-k) ;
            	}
	   		}	
		}
		cout << "Setting busy bit to 1" << endl;
		status_register_1[0]=SC_LOGIC_1;

		int row=address_int/256;
		int col=address_int%256;
		int i=col*8;

		
		cout << row << " " << col << " " << i << endl;	
		sc_bit program_data;
		bool over_write = false;
		static sc_bit data_deepcopy[256][2048];
		
		for (int j=0;j<256;j++)
		{
			for (int k=0;k<2048;k++)
			{
				data_deepcopy[j][k]=SC_LOGIC_1;
			}
		}
		cout << "Deepcopy initialized" << endl;
		for (int j=0;j<256;j++)
		{
			for (int k=0;k<2048;k++)
			{
				data_deepcopy[j][k]=data[j][k];
			}
		}
		
		cout << "Deepcopy created" << endl;
		int num=0;
		while (cs_low)
		{
			if (2048<=i)
			{
				cout << "Moving to initial cell" << endl ;
				i=0;
				over_write = true;
			}
			if (cs_p->event() && cs_low == false)
			{
				break;
			}
			if (data_in_p->num_available())
			{
				 program_data = data_in_p->read();
			}
			else
			{
				wait(data_in_p->data_written_event() | cs_p->default_event() );
				if (cs_p->event() && cs_low == false)
				{
					break;
				}
				program_data = data_in_p->read();
			}
			if (data[row][i]==SC_LOGIC_1 && program_data==SC_LOGIC_0)
			{
				data_deepcopy[row][i] = SC_LOGIC_0;
			}
			else if (over_write==true)
			{
				data_deepcopy[row][i] = program_data;
			}
			num++;
			i++;
		}
		cout << "CS set high, checking complete byte transfer with number of bytes transferred " << num  << endl;
		if ((i-col*8)%8==0)
		{
			for (int j=0;j<256;j++)
			{
				for (int k=0;k<2048;k++)
				{
					data[j][k]=data_deepcopy[j][k];
				}
			}
			cout << "Programming data" << endl;
			wait(time_program_page);
			cout << "Programmed bits successfully" << endl;
		}
		status_register_1[0]=SC_LOGIC_0;
		cout << "Busy bit set to 0" << endl;
		status_register_1[1]=SC_LOGIC_0;
		cout << "Write enable latch set to 0" << endl;
	}
}

void RRAM::page_write(void)
{
	for(;;)
	{	
		wait(begin_page_write);
		cout << "Page write started" << endl;
		int j=0;
		address_int = 0;

        while(cs_low && j<24)
        {
            if (data_in_p->num_available())
            {
                address[23-j]=data_in_p->read();
            }
            else
            {
                wait(data_in_p->data_written_event());
                address[23-j]=data_in_p->read();
            }
            j++;
        }
        if (cs_low && j==24)
        {
            for (int k=0;k<16;k++)
            {
				sc_bit add_bit = (sc_bit)address[k];
				if (add_bit==SC_LOGIC_1)
                {	
					address_int +=(int)pow(2.0,15-k) ;
            	}
	   		}	
		}
		cout << "Setting busy bit to 1" << endl;
		status_register_1[0]=SC_LOGIC_1;

		int row=address_int/256;
		int col=address_int%256;
		int i=col*8;

		cout << row << " " << col << " " << i << endl;
		sc_bit program_data = sc_bit('1');	
		static sc_bit data_deepcopy[256][2048];
		
		for (int j=0;j<256;j++)
		{
			for (int k=0;k<2048;k++)
			{
				data_deepcopy[j][k]=SC_LOGIC_1;
			}
		}
		cout << "Deepcopy initialized" << endl;
		for (int j=0;j<256;j++)
		{
			for (int k=0;k<2048;k++)
			{
				data_deepcopy[j][k]=data[j][k];
			}
		}
		
		cout << "Deepcopy created" << endl;
		while (cs_low)
		{
			if (data_in_p->num_available())
			{
				 program_data = data_in_p->read();
			}
			else
			{
				wait(data_in_p->data_written_event());
				program_data = data_in_p->read();
			}
			data_deepcopy[row][i]=program_data;
			i++;
			if (2048<=i)
			{
				cout << "Moving to initial cell" << endl ;
				i=0;
			}
		}
		if ((i-col*8)%8==0)
		{
			for (int j=0;j<256;j++)
			{
				for (int k=0;k<2048;k++)
				{
					data[j][k]=data_deepcopy[j][k];
				}
			}
			cout << "Writing data" << endl;
			wait(time_page_write);
			cout << "Written bits successfully" << endl;
		}
		status_register_1[0]=SC_LOGIC_0;
		cout << "Busy bit set to 0" << endl;
		status_register_1[1]=SC_LOGIC_0;
		cout << "Write enable latch set to 0" << endl;
	}
}

void RRAM::page_erase(void)
{
	for(;;)
	{	
		wait(begin_page_erase);
		cout << "Page erase started" << endl;
		int j=0;
		address_int = 0;

        while(cs_low && j<24)
        {
            if (data_in_p->num_available())
            {
                address[23-j]=data_in_p->read();
            }
            else
            {
                wait(data_in_p->data_written_event());
                address[23-j]=data_in_p->read();
            }
            j++;
        }
        if (cs_low && j==24)
        {
            for (int k=0;k<16;k++)
            {
				sc_bit add_bit = (sc_bit)address[k];
				if (add_bit==SC_LOGIC_1)
                {	
					address_int +=(int)pow(2.0,15-k) ;
            	}
	   		}	
		}
		if (j==24)
		{
			cout << "Setting busy bit to 1" << endl;
			status_register_1[0]=SC_LOGIC_1;

			int row=address_int/256;
			int col=address_int%256;
			int i=col*8;

			cout << row << " " << col << " " << i << endl;
			for (int k=i;k<2048;k++)
			{
				data[row][k]=SC_LOGIC_1;
			}

			cout << "Erasing data" << endl;
			wait(time_page_write);
			cout << "Erased bits successfully" << endl;
		}
		status_register_1[0]=SC_LOGIC_0;
		cout << "Busy bit set to 0" << endl;
		status_register_1[1]=SC_LOGIC_0;
		cout << "Write enable latch set to 0" << endl;
	}
}

void RRAM::read_status_register(void)
{
	for(;;)
	{
		wait(begin_read_status_register);

		int i=0;
		while(cs_low)
		{
			wait(clk_p->negedge_event() | cs_p.default_event());
			if (cs_p.event() && cs_low ==false)
			{
				break;
			}
			sc_bit status_data =(sc_bit) status_register_1[i];
			data_out_p->write(status_data);
			i++;
			if (i==8)
			{
				i=0;
			}
		}
		cout << "Exiting read status register" << endl;
	}
}

void RRAM::ignore_data_in_p(void)
{
	for(;;)
	{
		wait(begin_ignore_data_in_p);
		cout << "Ignoring data" << endl;
		sc_bit dummy;
		while (cs_low == true)
		{
			if (data_in_p->num_available())
			{
				dummy=data_in_p->read();
			}
			else
			{
				wait(data_in_p->data_written_event() | cs_p->default_event() );
				if(cs_p->event() && cs_low == false)
				{
					break;
				}
				dummy=data_in_p->read();
			}
		}
		cout << "Exiting dummy read" << endl;
	}
}

RRAM::~RRAM()
{
	sc_close_vcd_trace_file(spio);
}
