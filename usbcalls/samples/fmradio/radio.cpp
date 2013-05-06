/***************************/
/*                         */ 
/*  USB FM Radio SDK       */
/*  by Dr. Ebony Huang     */
/*  ysh@pidc.gov.tw        */ 
/*  OS/2 implementation    */ 
/*  by Markus Montkowski   */ 
/***************************/

/* After the installation of USB FM Radio, one can locate a file 
   called radio.dll in the Windows folder. The Radio.dll provides 
   some functions which can be used to control and communicate with 
   Radio hardware. The details of the functions in radio.dll 
   is explained as follows. You are welcome to write your own application 
   to control the Radio hardware. If you want to share your application 
   with others, please send to me via email:ysh@pidc.gov.tw We will 
   put your app. in the website www.fmbox.com.tw .
*/
#define INCL_DOS
#include <os2.h>
#include "..\..\usbcalls.h"


/* The Org Windows DLL also doesn't implement the open and close
   Function and does open/close in each function.
*/

/*
 Open the USB Radio hardware.
 Return
   0 : Failure, can not find Radio hardware.
   1 : Success, the hardware is correctly connected to PC and is opened.
*/
int _System open_radio(void)
{
  return 1;
}



/*
 Close the USB Radio hardware.
 Return 
   0       : Failure
   Nonzero : Success   
 In general, before going to control the Radio hardware, 
 one has to call the open_radio to see if the hardware exists. 
 Before close the application, one has to call the close_radio 
 to tell PC that you don't want to control the radio anymore.
*/
int _System close_radio(void)
{
  return 1;
}

/*
  Turning Radio on/off (mute or not)
  f:1 Turn on the radio, active
  f:0 Turn off the radio, inactive.
 Return 
   0       : Failure
   Nonzero : Success   
*/
int _System turn_power(int f)
{
  APIRET rc;
  USBHANDLE Handle;
  UCHAR ucData[8];

  rc = UsbOpen( &Handle,
                0x04b4,
                0x1002,
                USB_ANY_PRODUCTVERSION,
                USB_OPEN_FIRST_UNUSED);
  if(!rc)
  {
    rc = UsbCtrlMessage( Handle,
                         0xC0, 0x02,
                         f!=0?1:0, 0x00,
                         1,(UCHAR*)&ucData,
                         0);
    UsbClose(Handle);
  }
  return((rc==0)?1:0);
}



/*
  Setting radio frequency. The Japanese Radio hardware is different 
  from other area's Radio hardware. Japanese Radio hardware covers 
  the radio freq. 76.00~91.00 MHz, others cover 88.00~108.00 MHz. 
  The minimum freq step is 0.05 MHz. Since we use the integer ch(channel)  
  as the parameter to set radio frequency,  the ch, therefore, 
  is defined as ch=Freq*20. This means that when one to set the 100.1 MHz,  
  one should put ch to be 100.1*20 and the japan as 0 as the parameter 
  in the set_freq function
  For the japanese radio hardware, one has to set japan to be 1 when 
  invoking the set_freq function.

 Return 
   0       : Failure
   Nonzero : Success   
*/
int _System set_freq(int ch,int japan)
{
  double dFreq;
  ULONG ulFreq;
  APIRET rc;
  USBHANDLE Handle;
  UCHAR ucData[8];

  dFreq = ch * 0.05;
  if(japan)
  {
    ulFreq = ((dFreq-10.7)*80);
  }
  else
  {
    ulFreq = ((dFreq+10.7)*80);
  }
  rc = UsbOpen( &Handle,
                0x04b4,
                0x1002,
                USB_ANY_PRODUCTVERSION,
                USB_OPEN_FIRST_UNUSED);
  if(!rc)
  {
    rc = UsbCtrlMessage( Handle,
                         0xC0, 0x01,
                         ulFreq>>8,ulFreq,
                         1,(UCHAR*)&ucData,
                         0);
    UsbClose(Handle);
  }
  return(rc==0?1:0);
}




/*
  Get the Radio status.

 Return 
   0       : non stereo
   1       : Radio is in stereo mode, this also means the radio is tuned 
             to some station.

*/
int _System get_stereo(void)
{
  APIRET rc;
  USBHANDLE Handle;
  UCHAR ucData[8];

  rc = UsbOpen( &Handle,
                0x04b4,
                0x1002,
                USB_ANY_PRODUCTVERSION,
                USB_OPEN_FIRST_UNUSED);
  if(!rc)
  {
    rc = UsbCtrlMessage( Handle,
                         0xC0, 0x00,
                         0, 0x00,//0x24,
                         1,(UCHAR*)&ucData,
                         0);
    UsbClose(Handle);
  }
  return(rc==0?((ucData[0]&0x01)==0x00)?1:0:-1);
}


/*
   The above five basic functions can let the programmer to write his/her own
   special application to control the Radio hardware. 
   But we do not provide the searching radio station function in the radio.dll. 
   The users can combine the set_freq and get_stereo functions to do the 
   seraching station function. One thing should be noted: 
   after calling the set_freq, one should delay for a while,say 100ms, 
   before reading the radio status.(get_stereo). 
   If the radio is not tuned, the get_stereo will retrun 0, 
   otherwise it will return a nonzero value.

                          100.1 MHz
    searching up     100.0   100.2
    ---->----------------****----------
                    There is a station covers 100.0~100.2 MHz 
                       
The above approach cannot locate the best radio station because it will
stop immediately when the get_stereo return 1. 
Like the above figure,the seraching function will stop at 100.00 MHz 
instead of the best station 100.1 MHz. Therefore, during the searching 
mode, when one finds the radio is in the stereo mode, one has to keep 
tuning to find out what radio frequency is not tuned(here is 100.2MHz), 
after the above two steps, we can conclude the searching radio station 
should stop at 100.1MHz=(100.0+100.2)/2.


Ebony Huang
*/
