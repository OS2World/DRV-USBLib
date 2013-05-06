/*
    Author : David Corcoran
    Title  : usblinux.h
    Purpose: To provide Linux abstraction to searaching the
             USB layer.
*/

#ifndef __usblinux_h__
#define __usblinux_h__


struct usb_bulktransfer {
  /* keep in sync with usb.h:usb_proc_bulktransfer */
     unsigned int ep;
     unsigned int len;
     unsigned int timeout;   /* in milliseconds */
 
     /* pointer to data */
     void *data;
};        


struct usb_device_descriptor {
	u_int8_t  bLength;
	u_int8_t  bDescriptorType;
	u_int16_t bcdUSB;
	u_int8_t  bDeviceClass;
	u_int8_t  bDeviceSubClass;
	u_int8_t  bDeviceProtocol;
	u_int8_t  bMaxPacketSize0;
	u_int16_t idVendor;
	u_int16_t idProduct;
	u_int16_t bcdDevice;
	u_int8_t  iManufacturer;
	u_int8_t  iProduct;
	u_int8_t  iSerialNumber;
	u_int8_t  bNumConfigurations;
} __attribute__ ((packed));

#define IOCTL_USB_BULK          _IOWR('U', 2, struct usb_bulktransfer) 

int open_linux_usb_dev ( unsigned int, unsigned int, unsigned int );
int close_linux_usb_dev( int );

int bulk_linux_usb_dev( int, int, unsigned char *, int *, int );

#endif

