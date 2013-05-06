/*
    Author : David Corcoran
    Title  : usblinux.h
    Purpose: To provide Linux abstraction to searaching the
             USB layer.
*/

#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <limits.h>
#include <dirent.h>
#include <stdlib.h>
#include <usblinux.h>
#include <stdio.h>

#define PCSCLITE_USB_PATH      "/proc/bus/usb"


int open_linux_usb_dev ( unsigned int manuID, unsigned int prodID, 
			 unsigned int lunNum ) {

  DIR                         *dir, *dirB;
  struct dirent               *entry, *entryB;
  char                         dirpath[150];
  struct usb_device_descriptor usbDescriptor;

  
  dir = opendir(PCSCLITE_USB_PATH);
  if (!dir) {
    printf("Cannot Open USB Path Directory\n");
  }
  
  
  while ((entry = readdir(dir)) != NULL) {
    
    /* Skip anything starting with a . */
    if (entry->d_name[0] == '.')
      continue;
    if (!strchr("0123456789", entry->d_name[strlen(entry->d_name) - 1])) {
      continue;
    }
    
    sprintf(dirpath, "%s/%s", PCSCLITE_USB_PATH, entry->d_name);
    
    dirB = opendir(dirpath);
    
    if (!dirB) {
      printf("Path does not exist - do you have USB ?\n");
    }
    
    while ((entryB = readdir(dirB)) != NULL) {
      char filename[PATH_MAX + 1];
      struct usb_device *dev;
      int fd, ret;
      
      /* Skip anything starting with a . */
      if (entryB->d_name[0] == '.')
	continue;
      
      
      sprintf(filename, "%s/%s", dirpath, entryB->d_name);
      fd = open(filename, O_RDWR);
      if (fd < 0) {
	continue;
      }
      
      ret = read(fd, (void *)&usbDescriptor, sizeof(usbDescriptor));

      if (ret < 0) {
	continue;
      }
      
      /* Device is found and we don't know about it */
      if ( usbDescriptor.idVendor == manuID && 
	   usbDescriptor.idProduct == prodID ) {
        closedir(dir); closedir(dirB);
	return fd;
      } else {
	close(fd);
      }      

    }
  }

  closedir(dir); closedir(dirB);
  return -1;
}  
      


int close_linux_usb_dev( int fd ) {
  return close( fd );
}

int bulk_linux_usb_dev( int fd, int pipeNum, unsigned char *buffer, 
			int *length, int timeout ) {
  
  struct usb_bulktransfer bulk;
  int    ret, retrieved = 0;

  bulk.ep      = pipeNum;
  bulk.len     = *length;
  bulk.timeout = timeout;
  bulk.data    = buffer;

  ret = ioctl( fd, IOCTL_USB_BULK, &bulk );

  if ( ret < 0 ) {
    *length = 0;
    return -1;
  }

  *length = ret;

  return ret;  
}
