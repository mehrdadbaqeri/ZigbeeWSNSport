ifndef TARGET
TARGET=z1
endif

CONTIKI_PROJECT = sportwsn sportwsnbs
CONTIKI_SOURCEFILES += cc2420-arch.c
PROJECT_SOURCEFILES = i2cmaster.c tmp102.c adxl345.c

all: $(CONTIKI_PROJECT)

CONTIKI = /home/mehrdad/Desktop/contiki-2.x
include $(CONTIKI)/Makefile.include
