# Student's Makefile for the CS:APP Performance Lab
TEAM = bovik
VERSION = 1
HANDINDIR = /opt/prac/cs_ai/perflab/handin2012

WSIZE = 64
OPTLEVEL = 2

CC = gcc
CFLAGS = -Wall -g -O$(OPTLEVEL) -mssse3  -mfpmath=sse  -m$(WSIZE) 
CFLAGS = -Wall -g -m$(WSIZE)
CFLAGS = -Wall -g 

LIBS = -lm -lrt

OBJS = rotate-versions.o driver.o kernels.o fcyc.o cpe.o
OBJS = rotate-versions.o driver.o kernels.o fcyc.o clock.o cpe.o lsquare.o
OBJS = line-versions.o line-baseline.o rotate-baseline.o driver.o kernels.o fcyc.o clock.o cpe.o lsquare.o

all: driver

driver: $(OBJS) fcyc.h clock.h defs.h config.h rotate-versions.h cpe.h
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o driver

.PHONY : handin
handin:
	mkdir -p $(TEAM)-$(VERSION)
	cp kernels.c $(TEAM)-$(VERSION)
	cp line-versions.c $(TEAM)-$(VERSION)
	cp line-versions.h $(TEAM)-$(VERSION)
	cp labbook.txt       $(TEAM)-$(VERSION)
	cp Makefile          $(TEAM)-$(VERSION)
	tar zcvf ./$(TEAM)-$(VERSION)-perflab.tgz $(TEAM)-$(VERSION)/*
	install -m=+r $(TEAM)-$(VERSION)-perflab.tgz $(HANDINDIR)/

.PHONY : clean
clean: 
	-rm -f $(OBJS) driver core *~ *.o

clock.o: clock.h
driver.o: fcyc.h defs.h config.h
fcyc.o: fcyc.h clock.h
kernels.o: defs.h rotate-versions.h
rotate-versions.o: defs.h rotate-versions.h
line-versions.o: defs.h line-versions.h
cpe.o: cpe.h
