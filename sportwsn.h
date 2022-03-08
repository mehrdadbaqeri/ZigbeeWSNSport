#ifndef _SPORTWSN_H
#define _SPORTWSN_H

#include "contiki.h"
#include "net/rime.h"
#include "net/rime/mesh.h"
#include "net/rime/route.h"
#include <stdlib.h>

#include "dev/button-sensor.h"
#include "dev/i2cmaster.h"
#include "dev/tmp102.h"
#include "dev/leds.h"
#include "adxl345.h"
#include <math.h>

#define MAXRETRY 5

/* Rime channel used for this application. */
#define SPORTWSN_TEMP_CHANNEL 13
#define SPORTWSN_STEP_CHANNEL 25

/* IEEE 802.15.4 channel used for this application. */
#define IEEE802_15_4_CHANNEL 7

/* Transmission power for this application. */
#define CC2420_TX_POWER 1

#define BASE_STATION_ADDRESS 46

#endif
