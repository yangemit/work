# CONFIG_UCLIBC_BUILD=y

CROSS_COMPILE ?= mips-linux-gnu-
CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)g++

 
EXEC = adb
OBJS = \
	adb.o \
	console.o \
	usb_libusb.o \
	transport.o \
	transport_local.o \
	transport_usb.o  \
	commandline.o \
	adb_client.o \
	sockets.o \
	services.o \
	file_sync_client.o \
	get_my_path_linux.o \
	usb_linux.o \
	utils.o \
	usb_vendors.o \
	fdevent.o

FLTFLAGS += -s 8192


# CFLAGS +=  -g  -Wall -Wno-unused-parameter -march=mips32r2\
#             -D_XOPEN_SOURCE -D_GNU_SOURCE -DALLOW_ADBD_ROOT=1 -I. \
			
CFLAGS +=  -g  -Wall -Wno-unused-parameter -march=mips32r2 -I.
LDFLAG += -Wl,-gc-sections

CFLAGS += \
	-O2 \
	-g \
	-DADB_HOST=1 \
	-DADB_HOST_ON_TARGET=1 \
	-Wall \
	-Wno-unused-parameter \
	-D_XOPEN_SOURCE \
	-D_GNU_SOURCE	\
	-DHAVE_FORKEXEC

LIBUTIL += -lutil $(LDLIBS)  -lpthread -lm -lrt -ldl


ifeq ($(CONFIG_UCLIBC_BUILD), y)
	CFLAGS += -muclibc
	LDFLAG += -muclibc
endif


all: $(EXEC)

$(EXEC): $(OBJS)
	$(CPLUSPLUS) $(LDFLAG) -o $@ $(OBJS) $(LIBUTIL) 
	# cp adbd adbd.elf

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

# $(OBJS): adb_client.h  adb.h  fdevent.h  file_sync_service.h  mutex_list.h  sysdeps.h  transport.h  usb_vendors.h  utils.h

