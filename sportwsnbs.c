#include "sportwsn.h"
#include "mypacket.h"

#include <stdio.h>
#include <string.h>

rimeaddr_t myAddr;
static struct mesh_conn mesh_temp;

/*---------------------------------------------------------------------------*/
PROCESS(temp_process, "Temp process");
AUTOSTART_PROCESSES(&temp_process);
/*---------------------------------------------------------------------------*/
/* Declare step process*/
PROCESS(step_process, "Step process");

static void
recv_temp(struct mesh_conn *c, const rimeaddr_t *from, uint8_t hops)
{
	int i;
	struct temp_packet *p1;
	
	p1 = (struct temp_packet *)  packetbuf_dataptr();
	
	printf("%d\n",p1->source.u8[0]);//source
	printf("%d\n",p1->destination.u8[0]);//destination
	printf("%c\n",p1->dataType);//datatype
        //printf("%s\n",p1->data);//data
	printf("%ld.%04u\n", (long)(p1->data), (unsigned)(((p1->data) - floor(p1->data))*10000) ); //data
	for (i=0; i<hops; i++)
	        printf("%d", p1->path[i]);
        printf("\n");//path
	printf("%d\n", hops);//hops
	printf("%d\n", p1->retryNumber);// retry number      
	printf("%d\n", p1->packetSeqNo);//packetSeqNo
        leds_toggle(LEDS_BLUE);        
}       

const static struct mesh_callbacks callbacks_temp = { recv_temp };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(temp_process, ev, data)
{
    PROCESS_EXITHANDLER(mesh_close(&mesh_temp);)
    PROCESS_BEGIN();
    myAddr=rimeaddr_node_addr;
    mesh_open(&mesh_temp, SPORTWSN_TEMP_CHANNEL, &callbacks_temp);
    SENSORS_ACTIVATE(button_sensor);
	
	process_start(&step_process, "");

    PROCESS_END();
}
/*---------------------------------------------------------------------------*/

static struct mesh_conn mesh_step;

static void
recv_step(struct mesh_conn *c, const rimeaddr_t *from, uint8_t hops)
{
	int i;
	struct step_packet *p1;
	
	p1 = (struct step_packet *)  packetbuf_dataptr();
	
	printf("%d\n",p1->source.u8[0]);//source
	printf("%d\n",p1->destination.u8[0]);//destination
	printf("%c\n",p1->dataType);//datatype
        printf("%d\n",p1->data);//data
	for (i=0; i<hops; i++)
	        printf("%d", p1->path[i]);
        printf("\n");//path
	printf("%d\n", hops);//hops
	printf("%d\n", p1->retryNumber);// retry number      
	printf("%d\n", p1->packetSeqNo);//packetSeqNo
        leds_toggle(LEDS_GREEN);        
}       

const static struct mesh_callbacks callbacks_step = {recv_step};

/* Our main process. */
PROCESS_THREAD(step_process, ev, data) {
	
    PROCESS_EXITHANDLER(mesh_close(&mesh_step);)
    PROCESS_BEGIN();
    

    mesh_open(&mesh_step, SPORTWSN_STEP_CHANNEL, &callbacks_step);


	PROCESS_END();
}

