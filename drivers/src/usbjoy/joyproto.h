/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  USBPROTO.H                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB keyboard driver function prototypes               */
/*                                                                            */
/*   FUNCTION: This module is the USB Joystick driver function prototype      */
/*             include file.                                                  */
/*   NOTES:                                                                   */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS:  None                                                      */
/*                                                                            */
/*   EXTERNAL REFERENCES:  None                                               */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*          00/01/04  MM                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

// JOYSTRAT.C
void FAR JOYStrategy (void);
void     JOYInitComplete (void);
// JOYIDC.C
void FAR JOYidc   (RP_GENIOCTL FAR *pRP_GENIOCTL);
void     JOYlegIO (RP_GENIOCTL FAR *pRP_GENIOCTL);
void     JOYserv  (RP_GENIOCTL FAR *pRP_GENIOCTL);
void     JOYdet   (RP_GENIOCTL FAR *pRP_GENIOCTL);

void     SetLEDs     (BYTE LEDstate);
void     SetIdleTime (USHORT joyIndex, USHORT kbdIRQstatus);

void     JOYEnumDevices(USHORT usGameDriverId);
int      GenJoyRegisterDevice(USHORT usDevID);
void     GenJoyUpdateState(USHORT usDevID);

void     JOYirq   (RP_GENIOCTL FAR *pRP_GENIOCTL); // JOYIRQ.C
//void     IdleSet (RP_GENIOCTL FAR *pRP_GENIOCTL);
void     ReadInterruptPipe (RP_GENIOCTL FAR *pRP_GENIOCTL);
void     InterruptDataReceived (RP_GENIOCTL FAR *pRP_GENIOCTL);
void     JOYClearStalled (RP_GENIOCTL FAR *pRP_GENIOCTL);

void JOYInit (void);  // JOYINIT.C

