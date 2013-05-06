/* fm-radio.c: A user-level program to set the Gemtek (D-Link) FM tuner. */
/*
  Written 2000 by Donald Becker, Scyld Computing Corporation.
  Released under the GPL.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define INCL_DOS
#include <os2.h>
#include "..\..\usbcalls.h"


/* The following are the known command of the radio. */
#if 0
static struct usb_op_request
    get_status = {0xC0, 0x00, 0x00, 0x24, 1},
    set_freq   = {0xC0, 0x01, 0, 0, 1},
    power      = {0xC0, 0x02, 0 /* or 1 to switch on */, 0, 1},
#endif

/* The operational mode of the TEA5757 AM/FM radio chip is set by 24 bit
   words, send as a serial bit stream.  The low 16 bits set the PLL
   (phase locked loop) multiplier, in 12.5 KHz steps.  The "FM value" is
   result of this frequency multiplication.  The tuned RF frequency is
   10.7Mhz below this value.
   (10.7 Mhz is a standard FM local oscillator frequency.)
   For instance, to tune 99.1Mhz set the FM value to 109.8Mhz, or
   109.8Mhz/12.5Khz = 8784.
   The 16 bit tuning value is put into the low bytes of the C0 01 command.
   There is no apparent way to set the radio mode bits, 23-16.
*/


const int if_freq = 10700001; /* +1 for rounding.  */
#define STERO_DELAY 240

struct top {
    char *cmdopt;
    char *msg;
} cmp_ops[] = {
    {"off", "Turning tuner off"},
    {"mute", "Turning tuner off"},
    {"on", "Turn tuner on"},
    {"unmute", "Turn tuner on"},
    {"status", "Get Status"},
    {0,}};


BOOL fFmOn(USBHANDLE Handle)
{
  APIRET rc;
  UCHAR ucData[8];
  printf("fFmOn\r\n",rc);
  rc = UsbCtrlMessage( Handle,
                       0xC0, 0x00,
                       0x17, 0x08,
                       1,(UCHAR*)&ucData,
                       0);
  rc = UsbCtrlMessage( Handle,
                       0xC0, 0x02,
                       0x01, 0x00,
                       1,(UCHAR*)&ucData,
                       0);
  if(rc)
    printf("Error rc=%d\r\n",rc);
#ifdef DEBUG
  else
    printf("Radio status %02X\r\n",ucData[0]);
#endif
  return (rc==0);
}

BOOL fFmOff(USBHANDLE Handle)
{
  APIRET rc;
  UCHAR ucData[8];
  printf("fFmOff\r\n",rc);
  rc = UsbCtrlMessage( Handle,
                       0xC0, 0x00,
                       0x94, 0xfc,
                       1,(UCHAR*)&ucData,
                       0);
  rc = UsbCtrlMessage( Handle,
                       0xC0, 0x02,
                       0x00, 0x00,
                       1,(UCHAR*)&ucData,
                       0);
  if(rc)
    printf("Error rc=%d\r\n",rc);
#ifdef DEBUG
  else
    printf("Radio status %02X\r\n",ucData[0]);
#endif
  return (rc==0);
}

BOOL fTunerSetMute(USBHANDLE Handle, BOOL fMute)
{
  APIRET rc;
  UCHAR ucData[8];
  rc = UsbCtrlMessage( Handle,
                          0xC0, 0x02,
                          fMute?0:1, 0,
                          1,(UCHAR*)&ucData,
                          0);
  if(rc)
    printf("Error rc=%d\r\n",rc);
#ifdef DEBUG
  else
    printf("Radio status %02X\r\n",ucData[0]);
#endif
  return (rc==0);
}

BOOL fSetFreq(USBHANDLE Handle, double Freq)
{
  APIRET rc;
  UCHAR ucData[8];
  int pll_div;
  pll_div = ((Freq+10.7)*80);
  printf("Setting tuner to %3.2f Mhz (%d=0x%04X).\n", Freq, pll_div,pll_div);
  rc = UsbCtrlMessage( Handle,
                       0xC0, 0x01,
                       pll_div>>8,pll_div,
                       1,(UCHAR*)&ucData,
                       0);
  if(rc)
    printf("Error rc=%d\r\n",rc);
#ifdef DEBUG
  else
    printf("Radio status %02X\r\n",ucData[0]);
#endif
  return (rc==0);
}

BOOL fGetStatus(USBHANDLE Handle, BOOL *fStereo)
{
  APIRET rc;
  UCHAR ucData[8];
  printf("GetStatus\r\n",rc);
  rc = UsbCtrlMessage( Handle,
                       0xC0, 0x00,
                       0, 0x00,//0x24,
                       1,(UCHAR*)&ucData,
                       0);
  *fStereo = (ucData[0]&0x01)==0x00;
  if(rc)
    printf("Error rc=%d\r\n",rc);
#ifdef DEBUG
  else
    printf("Radio status %02X\r\n",ucData[0]);
#endif
  return (rc==0);
}

int main(int argc, char *argv[])
{
  USBHANDLE hRadio;
  BOOL fOk;
  APIRET rc;
  BOOL IsStereo;
  int saved_freq;
  USHORT usVendor  = 0x04b4;
  USHORT usProduct = 0x1002;

  rc = UsbOpen( &hRadio,
                usVendor,
                usProduct,
                0xFFFF,
                USB_OPEN_FIRST_UNUSED);
  if(rc)
  {
    fprintf(stderr, "Cannot open Radio");
    return (-3);
  }

  UsbSetDeviceConfiguration(hRadio,1);

  if (argc >= 2  && argv[1])
  {
    double freq_mhz = 99.9;
    int i;
    for (i = 0; cmp_ops[i].cmdopt; i++)
    {
      if (strcmp(argv[1], cmp_ops[i].cmdopt) == 0)
      {
        switch(i)
        {
          case 0:
          case 1:
            fFmOff(hRadio);
            break;
          case 2:
          case 3:
            fFmOn(hRadio);
            break;
          case 4:
            fGetStatus(hRadio,&IsStereo);
            printf( " Stereo indicator is %s.\n",IsStereo ? "on" : "off");
            return IsStereo?0:-1;
          default:
            printf("Unsupported parameter\r\n");
            break;
        }

        UsbClose(hRadio);
        return 0;
      }
    }

    if ( ((freq_mhz = atof(argv[1])) == 0) ||
         (freq_mhz<88) || (freq_mhz>109.8) )
    {
      fprintf(stderr, "The frequency must be a number 88.0 - 109.8.\n");
      UsbClose(hRadio);
      return -2;
    }
    fOk = fSetFreq(hRadio, freq_mhz);
    DosSleep(STERO_DELAY);
    fGetStatus(hRadio, &IsStereo);
    printf( " Stereo indicator is %s.\n",IsStereo ? "on" : "off");
    UsbClose(hRadio);
    return IsStereo?0:-1;
  }

  // Do a frequency scan.
  int pll_div = 8800;
  printf( "Starting scan frequency is %3.2f MHz.\n", (pll_div/100.0) );
  for (; pll_div < 10980; pll_div += 5)
  {
    fSetFreq(hRadio, (pll_div/100.0));
    DosSleep(STERO_DELAY);
    fGetStatus(hRadio,&IsStereo);
    if (IsStereo)
    {
      printf(" Stereo is on!\n");
      saved_freq = pll_div;
    }
  }

  if (saved_freq)
  {
    fSetFreq(hRadio, (saved_freq/100.0));
    printf( "\rSetting tuner to last station, %3.2f.\n",(saved_freq/100.0));
    return 0;
  } else
      printf("\r Finished scan with no found stations.\n");

  UsbClose(hRadio);
  return -1;

}

