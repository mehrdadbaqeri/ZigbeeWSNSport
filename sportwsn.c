#include "sportwsn.h"
#include "mypacket.h"

#include <stdio.h>
#include <string.h>

//#define TEMP_PACKET_SIZE sizeof(temp_packet)
//#define STEP_PACKET_SIZE sizeof(step_packet)

rimeaddr_t myAddr;
static uint16_t packSeqNo=0;
static struct mesh_conn mesh_temp;
static struct temp_packet tp;

/*---------------------------------------------------------------------------*/
PROCESS(temp_process, "Temp process");
AUTOSTART_PROCESSES(&temp_process);
/*---------------------------------------------------------------------------*/
/* Declare step process*/
PROCESS(step_process, "Step process");

static void
sent_temp(struct mesh_conn *c)
{
    //printf("temp packet sent\n");
    leds_toggle(LEDS_BLUE);
    leds_on(LEDS_RED);
}

static void
timedout_temp(struct mesh_conn *c)
{
	if( tp.retryNumber < MAXRETRY ) {
        	//printf("packet timedout.. sending again..\n");
		tp.retryNumber++;
        	packetbuf_copyfrom(&tp,sizeof(tp));
        	mesh_send(&mesh_temp,&tp.destination);
		leds_on(LEDS_RED);
	}
}

static void
recv_temp(struct mesh_conn *c, const rimeaddr_t *from, uint8_t hops)
{
/*	int i;
	struct temp_packet *p1;
	
	p1 = (struct temp_packet *)  packetbuf_dataptr();

	//printf("Received.........\n");
	//printf("Type        : %c", (p1->dataType));
	//printf("Data        : %ld.%04u\n", (long)(p1->data), (unsigned)(((p1->data) - floor(p1->data))*10000) );
	//printf("Source      : %d \n",p1->source.u8[0]);
  	//printf("Packet sequence number :  %d \n", p1->packetSeqNo);
	//printf("destination : %d \n",p1->destination.u8[0]);
	//printf("Path:\t");
	for (i=0; i<hops; i++)
	    //printf("%d \t", p1->path[i]);
  	//printf("\n");
  	//printf("Hops :  %d \n", hops);
  	//printf("Retry number :  %d \n", p1->retryNumber);
*/
        leds_toggle(LEDS_GREEN);
}       

const static struct mesh_callbacks callbacks_temp = {recv_temp, sent_temp, timedout_temp};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(temp_process, ev, data)
{
    PROCESS_EXITHANDLER(mesh_close(&mesh_temp);)
    PROCESS_BEGIN();
    myAddr=rimeaddr_node_addr;
    mesh_open(&mesh_temp, SPORTWSN_TEMP_CHANNEL, &callbacks_temp);
    SENSORS_ACTIVATE(button_sensor);

	process_start(&step_process, "");
s
    static float temperature;
    int16_t  tempint;
    uint16_t tempfrac;
    int16_t  raw;
    uint16_t absraw;
    int16_t  sign;

    tmp102_init();
    while(myAddr.u8[0]!=BASE_STATION_ADDRESS)
//while(0)
    {
        static struct etimer et;
	////printf("My address");
        etimer_set(&et, CLOCK_SECOND * 2);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
        sign = 1;

	raw = tmp102_read_temp_raw();
        absraw = raw;
        if (raw < 0) 
	{ // Perform 2C's if sensor returned negative data
          absraw = (raw ^ 0xFFFF) + 1;
          sign = -1;
        }
	
	tempint = (absraw >> 8);
        tempfrac = ((absraw>>4) % 16) * 625; // Info in 1/10000 of degree
	temperature=(float)(sign*(tempint+tempfrac/10000.0));

	packSeqNo++;
	tp.data=temperature;
	tp.source=myAddr;
	tp.destination.u8[0]=BASE_STATION_ADDRESS;
	tp.destination.u8[1]=0;
	tp.dataType='T';
	tp.packetSeqNo=packSeqNo;
	tp.retryNumber=0;
	////printf("data %s\n",tp.data);        
	packetbuf_copyfrom(&tp,sizeof(tp));
        mesh_send(&mesh_temp, &tp.destination);
    }
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/

#define MAXARRCONST 0  
#define MINARRCONST 65535    
#define Sample_Counter_Threshold 50
//interval betweein two steps
#define Min_Interval_Threshold 10 
//interval betweein two steps
#define Max_Interval_Threshold 100

#define STEP_INTERVAL (CLOCK_SECOND/50)

#define XSIZE 5 //X_Array size


/* The client process should be started automatically when
* the node has booted. */
//AUTOSTART_PROCESSES(&step_process);

static uint16_t packSeqNo_step=0;
static struct mesh_conn mesh_step;
static struct step_packet sp;

static struct etimer step_et;

static uint16_t Sample_Counter = 0;
static uint16_t Interval = 0;
static uint16_t TimeCounter = 0;

static int16_t Res_Array[3] = {0,0,0};
static uint16_t DC_Mag = 0;
static uint16_t Vpp_Mag = 0;
static uint16_t Precision = 0; 
static uint16_t Precision_Percent = 50;

static uint16_t Min_Mag = MINARRCONST;
static uint16_t Max_Mag = MAXARRCONST;
static uint16_t Old_Mag = 0;
static uint16_t New_Mag = 0;
static int16_t Initial_Array[3] = {0,0,0}; //Initial mote position. Use to calculate acceleration magnitude correctly
static uint16_t Accel_Mag = 0;

static uint16_t Step_Counter = 0;
static uint16_t NewStepCount = 0;

//return back to SumFilter function
static int16_t X_Array[XSIZE] ={0,0,0,0,0};
static int16_t Y_Array[XSIZE] ={0,0,0,0,0};
static int16_t Z_Array[XSIZE] ={0,0,0,0,0};

inline int16_t abs(const int16_t x)
{
	if( x < 0 )
		return -x;
	return x;
}

void PeakFilter()
{
	Sample_Counter = 0;

	Vpp_Mag = abs( Max_Mag - Min_Mag );
	DC_Mag = Vpp_Mag/2;
	Min_Mag = MINARRCONST;
	Max_Mag = MAXARRCONST;
	Precision = Vpp_Mag * Precision_Percent / 100;

}

void DetermineStep()
{
		Old_Mag = New_Mag;
		if( abs( Accel_Mag - New_Mag ) > Precision )
		{

			New_Mag = Accel_Mag;
			
			if ( (Old_Mag > DC_Mag) && (DC_Mag > New_Mag) )
			{
			//TimeWindow
				if( (Interval < Max_Interval_Threshold) && (Interval > Min_Interval_Threshold) ){
					Step_Counter++; // Finally we get a step :D
					Interval = 0;
				} else if( Interval > Max_Interval_Threshold ) {
					Interval = 0;
				}				
			}	
		}
}


void SumFilter(int16_t Res_Array1[])
{
	static int i=0;
	for(i=0;i<3;i++)
		Res_Array1[ i ] = 0;
	
	for(i=XSIZE-1; i>0; i--)
	{
		X_Array[i]= X_Array[i-1];
		Y_Array[i]= Y_Array[i-1];
		Z_Array[i]= Z_Array[i-1];
		
		Res_Array1[0] +=  X_Array[i]; 
		Res_Array1[1] +=  Y_Array[i]; 
		Res_Array1[2] +=  Z_Array[i];           
	}
	
	X_Array[0]= accm_read_axis(X_AXIS) - Initial_Array[0];               
	Y_Array[0]= accm_read_axis(Y_AXIS) - Initial_Array[1];
	Z_Array[0]= accm_read_axis(Z_AXIS) - Initial_Array[2];
	
	Res_Array1[0] +=  X_Array[0]; 
	Res_Array1[1] +=  Y_Array[0]; 
	Res_Array1[2] +=  Z_Array[0]; 
	
	Res_Array1[0] /= XSIZE;
	Res_Array1[1] /= XSIZE;
	Res_Array1[2] /= XSIZE;
}

uint16_t SenseStep()
{
	// step 1: initiate the values of the max and min values for comparing :
	// Make the maximum register min and minmum register max, so that they can be update at the next cycle immediately
	
	// we set it at first 

	// step 2: Save the last 3-axis samples to the shift registers for sum filtering: 
	//	   X_Axis_result =()/4;  
	//         Y_Axis_result =()/4; 
	//         Z_Axis_result =()/4;
	SumFilter(Res_Array);
	//MedianFilter(Res_Array);
	
	//calculate acceleration magnitude
	Accel_Mag = Res_Array[0]*Res_Array[0] + Res_Array[1]*Res_Array[1] + Res_Array[2]*Res_Array[2];
	Accel_Mag = sqrtf( Accel_Mag );
	
	// step 3: Find 3-axis max value and min value : 
	//		find x-axis max and min value
	//		find y-axis max and min value
	//		find z-axis max and min value
	//UpdateMinMax(Res_Array);
	if( Max_Mag < Accel_Mag ) Max_Mag = Accel_Mag;
	if( Min_Mag > Accel_Mag ) Min_Mag = Accel_Mag;
	
	// step 4 : Increase the sample counter
	Sample_Counter++; 

	// step 5 : Sample counter >= 50
	if (Sample_Counter >= Sample_Counter_Threshold)
	{
		PeakFilter(); // modifies the max and min values to latest values
	}
	// step 6 : determine the correct step
	DetermineStep();
	Interval++; //increase Interval ( 1 = sampling time = 20ms )
	TimeCounter++; //increase Interval ( 1 = sampling time = 20ms )
	return Step_Counter;
}

static void
sent_step(struct mesh_conn *c)
{
    ////printf("step packet sent\n");
    leds_toggle(LEDS_GREEN);
    leds_off(LEDS_RED);
}

static void
timedout_step(struct mesh_conn *c)
{
	if( sp.retryNumber < MAXRETRY ) {
        	////printf("step packet timedout.. sending again..\n");
		sp.retryNumber++;
        	packetbuf_copyfrom(&sp,sizeof(sp));
        	mesh_send(&mesh_step,&sp.destination);
		leds_on(LEDS_RED);
	}
}

static void
recv_step(struct mesh_conn *c, const rimeaddr_t *from, uint8_t hops)
{
	/*
	int i;
	struct step_packet *p1;
	
	p1 = (struct step_packet *)  packetbuf_dataptr();

	//printf("Received step.........\n");
	//printf("Type        : %c", (p1->dataType));
	//printf("Data        : %d\n", (p1->data) );
	//printf("Source      : %d \n",p1->source.u8[0]);
  	//printf("Packet sequence number :  %d \n", p1->packetSeqNo);
	//printf("destination : %d \n",p1->destination.u8[0]);
	//printf("Path:\t");
	for (i=0; i<hops; i++)
	    //printf("%d \t", p1->path[i]);
  	//printf("\n");
  	//printf("Hops :  %d \n", hops);
  	//printf("Retry number :  %d \n", p1->retryNumber);
        leds_toggle(LEDS_GREEN);
	*/
}       

const static struct mesh_callbacks callbacks_step = {recv_step, sent_step, timedout_step};
static uint8_t sendTimeCounter=0;

/* Our main process. */
PROCESS_THREAD(step_process, ev, data) {
	
    PROCESS_EXITHANDLER(mesh_close(&mesh_step);)
    PROCESS_BEGIN();
    

    mesh_open(&mesh_step, SPORTWSN_STEP_CHANNEL, &callbacks_step);

	/* Start and setup the accelerometer with default values, eg no interrupts enabled. */
	accm_init();
	
	//get the initial position
	Initial_Array[0]= accm_read_axis(X_AXIS);               
	Initial_Array[1]= accm_read_axis(Y_AXIS);
	Initial_Array[2]= accm_read_axis(Z_AXIS);
	
	/* Loop forever. */
	while (myAddr.u8[0]!=BASE_STATION_ADDRESS) {
		etimer_set(&step_et, STEP_INTERVAL);// 20ms
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&step_et));
		////printf("Steps: %d\n", NewStepCount );

		//DisplayTemp();
		NewStepCount = SenseStep();
		
		if( sendTimeCounter++>=50 ){
			sendTimeCounter=0;
			
			packSeqNo_step++;
			sp.data=NewStepCount;
			//printf("Steps: %d, or: %d\n", NewStepCount, sp.data );
			sp.source=myAddr;
			sp.destination.u8[0]=BASE_STATION_ADDRESS;
			sp.destination.u8[1]=0;
			sp.dataType='S';
			sp.packetSeqNo=packSeqNo_step;
			sp.retryNumber=0;
			////printf("data %s\n",tp.data);        
			packetbuf_copyfrom(&sp,sizeof(sp));
 		       mesh_send(&mesh_step, &sp.destination);
		}
		
	}

	/* This will never be reached, but we'll put it here for
* the sake of completeness: Close the broadcast handle. */

	PROCESS_END();
}

