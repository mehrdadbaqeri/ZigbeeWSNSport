#ifndef MYPACKET_H
#define MYPACKET_H

#include "sportwsn.h"

struct temp_packet
{
	char dataType; 
	uint8_t path[20];
	uint8_t retryNumber;
	rimeaddr_t source;
	rimeaddr_t destination;
	uint16_t packetSeqNo;
	float data;
};

struct step_packet
{
	char dataType; 
	uint8_t path[20];
	uint8_t retryNumber;
	rimeaddr_t source;
	rimeaddr_t destination;
	uint16_t packetSeqNo;
	uint16_t data;	
};

#endif /* MYPACKET_H */
