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

// GENSTRAT.C
void FAR GameStrategy (void);
void CmdError (PRPH pRP);
void     GameOpen(PRPH pRP);
void     GameClose(PRPH pRP);
void     GameIOCTL(PRPH pRP);
// GENIOCTL.C
USHORT VerifyData (PRP_GENIOCTL pRP, USHORT dataLength);
USHORT VerifyParam (PRP_GENIOCTL pRP, USHORT paramLength);

void     GetVersion(PRP_GENIOCTL pRP);        // 0x01
void     GetParam(PRP_GENIOCTL pRP);          // 0x02
void     SetParam(PRP_GENIOCTL pRP);          // 0x03
void     GetCalib(PRP_GENIOCTL pRP);          // 0x04
void     SetCalib(PRP_GENIOCTL pRP);          // 0x05
void     GetDigSet(PRP_GENIOCTL pRP);         // 0x06
void     SetDigSet(PRP_GENIOCTL pRP);         // 0x07
void     GetStatus(PRP_GENIOCTL pRP);         // 0x10
void     GetStatusButWait(PRP_GENIOCTL pRP);  // 0x11
void     GetStatusSampWait(PRP_GENIOCTL pRP); // 0x12
void     SwitchToAdvanced(PRP_GENIOCTL pRP);  // 0x20
void     GetNumDevices(PRP_GENIOCTL pRP);     // 0x21
void     GetDeviceString(PRP_GENIOCTL pRP);   // 0x22
void     SelectDevice(PRP_GENIOCTL pRP);      // 0x23
void     GetDevCaps(PRP_GENIOCTL pRP);        // 0x24
void     GetJoyState(PRP_GENIOCTL pRP);       // 0x25
void     GetDeadZone(PRP_GENIOCTL pRP);       // 0x26
void     SetDeadZone(PRP_GENIOCTL pRP);       // 0x27


void     FuncError(PRP_GENIOCTL pRP);
// GENIDC.C
void FAR Gameidc   (RP_GENIOCTL FAR *pRP_GENIOCTL);
void     GameAttachDevice (RP_GENIOCTL FAR *pRP_GENIOCTL);
void     GameDetachDevice (RP_GENIOCTL FAR *pRP_GENIOCTL);
void     GameProcessSample(RP_GENIOCTL FAR *pRP_GENIOCTL);
void     GameRegisterClient(RP_GENIOCTL FAR *pRP_GENIOCTL);
// GENINIT.C
void     GameInit(PRPH pRP);
static BOOL RegisterDrv (void);

