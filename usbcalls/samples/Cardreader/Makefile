

CFLAGS= -O2  -g -I/usr/local/pcsc/include/ -I.

targets= GCUSBTransport.o ifdhandler.o usblinux.o usbserial.o


all: $(targets)
	$(CC) -shared -o ./ifd-GemPC430.bundle/Contents/Linux/ifd-GemPC430 $(INCLUDE) $(targets)

clean:
	rm -f $(targets) ./ifd-GemPC430.bundle/Contents/Linux/ifd-GemPC430 

install:
	mkdir -p /usr/local/pcsc/drivers/
	cp -r ifd-GemPC430.bundle /usr/local/pcsc/drivers/
