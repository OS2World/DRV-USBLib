
/*
 *@@sourcefile ____sample.c:
 *      code template for an XCenter widget.
 *
 *      This is an example of an XCenter widget plugin.
 *      This code is compiled into a separate widget
 *      plugin DLL, which  (as with all widget plugins)
 *      must be put into the plugins/xcenter directory
 *      of the XWorkplace installation directory.
 *
 *      This dummy widget only displays a small rectangle
 *      with a 3D frame. This can be taken as a template
 *      for implementing something more useful.
 *
 *      This code might look terribly complex even though
 *      it does close to nothing. However, this gives you
 *      a good impression about how to structure a widget
 *      class in order to be able to extend it later.
 *
 *      In this template, we have basic support for
 *      setup strings. The widget does save colors and
 *      fonts dropped on it in its setup string.
 *
 *      This template does _not_ contain a settings dialog
 *      though. If you want to implement such a thing,
 *      take a look at the window list widget (w_winlist.c).
 *
 *      Of course, you are free not to use this code and
 *      rewrite everything from scratch.
 *
 *      Any XCenter widget plugin DLL must export the
 *      following procedures by ordinal:
 *
 *      -- Ordinal 1 (WgtInitModule): this must
 *         return the widgets which this DLL provides.
 *
 *      -- Ordinal 2 (WgtUnInitModule): this must
 *         clean up global DLL data.
 *
 *      Unless you start your own threads in your widget,
 *      you can safely compile the widget with the VAC
 *      subsystem libraries to reduce the DLL's size.
 *      You can also import functions from XFLDR.DLL
 *      to avoid code duplication.
 *
 *      This is all new with V0.9.7.
 *
 *@@added V0.9.7 (2000-12-31) [umoeller]
 *@@header "shared\center.h"
 */

/*
 *      Copyright (C) 2000 Ulrich M”ller.
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#pragma strings(readonly)

/*
 *  Suggested #include order:
 *  1)  os2.h
 *  2)  C library headers
 *  3)  setup.h (code generation and debugging options)
cd src\widgets *  4)  headers in helpers\
 *  5)  at least one SOM implementation header (*.ih)
 *  6)  dlgids.h, headers in shared\ (as needed)
 *  7)  headers in implementation dirs (e.g. filesys\, as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSMODULEMGR
#define INCL_DOSERRORS
#define INCL_DOSSEMAPHORES
#define INCL_DOSPROCESS
#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINDIALOGS
#define INCL_WININPUT
#define INCL_WINSWITCHLIST
#define INCL_WINRECTANGLES
#define INCL_WINPOINTERS
#define INCL_WINSYS
#define INCL_WINLISTBOXES
#define INCL_WINENTRYFIELDS

#define INCL_GPIPRIMITIVES
#define INCL_GPILOGCOLORTABLE
#include <os2.h>

// C library headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#define DONT_REPLACE_MALLOC         // in case mem debug is enabled
#include "setup.h"                      // code generation and debugging options

// disable wrappers, because we're not linking statically
#ifdef WINH_STANDARDWRAPPERS
  #undef WINH_STANDARDWRAPPERS
#endif

// headers in /helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\gpih.h"               // GPI helper routines
#include "helpers\prfh.h"               // INI file helper routines;
// this include is required for some
// of the structures in shared\center.h
#include "helpers\winh.h"               // PM helper routines
#include "helpers\xstring.h"            // extended string helpers
#include "helpers\comctl.h"             // For Tooltip

// XWorkplace implementation headers
#include "shared\center.h"              // public XCenter interfaces
#include "shared\common.h"              // the majestic XWorkplace include file
#include "widget.h"
#define USB_BIND_DYNAMIC 1
#include "..\..\usbcalls.h"
#pragma hdrstop                     // VAC++ keeps crashing otherwise

/* ******************************************************************
 *
 *   Private definitions
 *
 ********************************************************************/

/* ******************************************************************
 *
 *   XCenter widget class definition
 *
 ********************************************************************/

/*
 *      This contains the name of the PM window class and
 *      the XCENTERWIDGETCLASS definition(s) for the widget
 *      class(es) in this DLL.
 */

#define WNDCLASS_WIDGET_RADIO "XWPCenterRadioWidget"

static XCENTERWIDGETCLASS G_WidgetClasses[]
= {
  WNDCLASS_WIDGET_RADIO,     // PM window class name
  0,                          // additional flag, not used here
  "RadioControlWidget",       // internal widget class name
  "USB-Radio",                // widget class name displayed to user
  WGTF_UNIQUEPERXCENTER | WGTF_TRAYABLE | WGTF_TOOLTIP,      // widget class flags
  NULL                        // no settings dialog
};

/* ******************************************************************
 *
 *   Function imports from XFLDR.DLL
 *
 ********************************************************************/

/*
 *      To reduce the size of the widget DLL, it can
 *      be compiled with the VAC subsystem libraries.
 *      In addition, instead of linking frequently
 *      used helpers against the DLL again, you can
 *      import them from XFLDR.DLL, whose module handle
 *      is given to you in the INITMODULE export.
 *
 *      Note that importing functions from XFLDR.DLL
 *      is _not_ a requirement. We only do this to
 *      avoid duplicate code.
 *
 *      For each funtion that you need, add a global
 *      function pointer variable and an entry to
 *      the G_aImports array. These better match.
 *
 *      The actual imports are then made by WgtInitModule.
 */

// resolved function pointers from XFLDR.DLL
PCMNQUERYDEFAULTFONT pcmnQueryDefaultFont = NULL;

PCTRDISPLAYHELP pctrDisplayHelp = NULL;
PCTRFREESETUPVALUE pctrFreeSetupValue = NULL;
PCTRPARSECOLORSTRING pctrParseColorString = NULL;
PCTRSCANSETUPSTRING pctrScanSetupString = NULL;
PCTRSETSETUPSTRING pctrSetSetupString = NULL;

PGPIHDRAW3DFRAME pgpihDraw3DFrame = NULL;
PGPIHSWITCHTORGB pgpihSwitchToRGB = NULL;

PWINHFREE pwinhFree = NULL;
PWINHQUERYPRESCOLOR pwinhQueryPresColor = NULL;
PWINHQUERYWINDOWFONT pwinhQueryWindowFont = NULL;
PWINHSETWINDOWFONT pwinhSetWindowFont = NULL;

PXSTRCAT pxstrcat = NULL;
PXSTRCLEAR pxstrClear = NULL;
PXSTRINIT pxstrInit = NULL;

RESOLVEFUNCTION G_aImports[] =
{
  "cmnQueryDefaultFont", (PFN*)&pcmnQueryDefaultFont,

  "ctrDisplayHelp", (PFN*)&pctrDisplayHelp,
  "ctrFreeSetupValue", (PFN*)&pctrFreeSetupValue,
  "ctrParseColorString", (PFN*)&pctrParseColorString,
  "ctrScanSetupString", (PFN*)&pctrScanSetupString,
  "ctrSetSetupString", (PFN*)&pctrSetSetupString,

  "gpihDraw3DFrame", (PFN*)&pgpihDraw3DFrame,
  "gpihSwitchToRGB", (PFN*)&pgpihSwitchToRGB,

  "winhFree", (PFN*)&pwinhFree,
  "winhQueryPresColor", (PFN*)&pwinhQueryPresColor,
  "winhQueryWindowFont", (PFN*)&pwinhQueryWindowFont,
  "winhSetWindowFont", (PFN*)&pwinhSetWindowFont,

  "xstrcat", (PFN*)&pxstrcat,
  "xstrClear", (PFN*)&pxstrClear,
  "xstrInit", (PFN*)&pxstrInit
};

/* ******************************************************************
 *
 *   Private widget instance data
 *
 ********************************************************************/

/*
 *@@ RADIOSETUP:
 *      instance data to which setup strings correspond.
 *      This is also a member of SAMPLEPRIVATE.
 *
 *      Putting these settings into a separate structure
 *      is no requirement, but comes in handy if you
 *      want to use the same setup string routines on
 *      both the open widget window and a settings dialog.
 */

typedef struct _RADIOSETUP
{
  BOOL fTurnOn;           // Is the Radio switched On
  BOOL fOnOnAttach;       // Turn on the Radio when pluged in
  ULONG ulCurrentFreq;    // Current Freq the Radio is on or set to when pluged in/switched on
  ULONG ulStoredFreq[4];  // Station storage
} RADIOSETUP, *PRADIOSETUP;

#define  RADIO_STEREO   0x00000001
#define  RADIO_SCANUP   0x00000010
#define  RADIO_SCANDOWN 0x00000020
#define  RADIO_SCAN     0x00000030
#define  RADIO_TUNEUP   0x00000100
#define  RADIO_TUNEDOWN 0x00000200
#define  RADIO_TUNE     0x00000300
#define  RADIO_USBCHECK 0x00001000
typedef struct _DIGITSET
{
  HBITMAP hDigit[11];
  ULONG ulHeight;
  ULONG ulWidth;
}DIGITSET, *PDIGITSET;

#define OFF 0
#define ON  1
#define MOUSE_OVER 2
typedef struct _SKINITEM
{
  ULONG ulFlags;
  HBITMAP hBmp[3];
  SIZEL  Size;
  POINTL ptlPos;
}SKINITEM, *PSKINITEM;

enum ITEMNAMES
{
  Power    = 0,
  Stereo   = 1,
  UsbPres  = 2,
  ScanUp   = 3,
  ScanDown = 4,
  TuneUp   = 5,
  TuneDown = 6,
  Station1 = 7,
  Station2 = 8,
  Station3 = 9,
  Station4 = 10,
  Station1Set = 11,
  Station2Set = 12,
  Station3Set = 13,
  Station4Set = 14,
  MHz         = 15
};

typedef struct _RADIOSKIN
{
  ULONG ulType;
  HBITMAP hBackground;
  SKINITEM HundretDigit;
  DIGITSET NormalDigits;
  POINTL   ptlNormalDigitPos[2];
  DIGITSET SmallDigits;
  POINTL   ptlSmallDigitPos[2];
  SKINITEM SItems[16];
}RADIOSKIN, *PRADIOSKIN;

/*
 *@@ RADIOPRIVATE:
 *      more window data for the widget.
 *
 *      An instance of this is created on WM_CREATE in
 *      fnwpSampleWidget and stored in XCENTERWIDGET.pUser.
 */

typedef struct _RADIOPRIVATE
{
  // reverse ptr to general widget data ptr; we need
  // that all the time and don't want to pass it on
  // the stack with each function call
  PXCENTERWIDGET pWidget;


  RADIOSETUP Setup;   // widget settings that correspond to a setup string
  HEV hRadioPluged;   // Event Set When an USB Radio Is pluged in;
  HEV hRadioUnpluged; // Event Set When an USB Radio is Unpluged
  HEV hEvtFreqChange; // Freq Changed Set Device and Display
  HMUX hMuxWait;      // MuxWait used for the thread
  TID  hMonThread;    // Thread ID of the notify handler
  TID  hFreqThread;   // Thread ID of the Freq Thread
  USBNOTIFY NotifyID; // Id Returned from UsbRegisterDeviceNotification
  USBHANDLE hDevice;  // Handle of the USBDevice;
  ULONG ulState;      // State of the Radio
  ULONG ulScanStart;  // Memorises the start Freq when user presses one of the scan buttons
  ULONG ulStation;    // Current Selected Station;
  ULONG ulNumRadios;  // Number of Radios Attached to the PC
  PRADIOSKIN pSkin;  // SkinInfo
  BOOL fTooltipShowing; // Is there a Tooltip?
} RADIOPRIVATE, *PRADIOPRIVATE;

typedef struct _USBCALLS
{
  HMODULE                        hDLL;
  PUSBREGISTERDEVICENOTIFICATION pUsbRegisterDeviceNotification;
  PUSBDEREGISTERNOTIFICATION     pUsbDeregisterNotification;
  PUSBOPEN                       pUsbOpen;
  PUSBCLOSE                      pUsbClose;
  PUSBCTRLMESSAGE                pUsbCtrlMessage;
}USBCALLS, *PUSBCALLS;

HMODULE g_hResDLL;
USBCALLS g_USBFuncs;

void QueryBitmapDimension( HBITMAP hBmp, PSIZEL pSize)
{
  BITMAPINFOHEADER2 BmpInfo;
  BOOL rc;

  BmpInfo.cbFix = sizeof(BITMAPINFOHEADER2);
  rc = GpiQueryBitmapInfoHeader( hBmp, &BmpInfo);
  if(!rc)
  {
    pSize->cx = 0;
    pSize->cy = 0;
  }
  else
  {
    pSize->cx = BmpInfo.cx;
    pSize->cy = BmpInfo.cy;
  }
}

void LoadSkin(HPS hps,PSZ *pszSkinname, PRADIOPRIVATE pRadio)
{
  ULONG i;
  if (pRadio->pSkin)
    free(pRadio->pSkin);

  pRadio->pSkin = (PRADIOSKIN) malloc(sizeof(RADIOSKIN));
  // @@ToDo Use the spzSkinName to load a skin from a real resource file.
  pRadio->pSkin->hBackground = GpiLoadBitmap(hps, g_hResDLL, ID_BmpBackground,0,0);
  pRadio->pSkin->SItems[Power].ptlPos.x = 6;
  pRadio->pSkin->SItems[Power].ptlPos.y = 17;
  pRadio->pSkin->SItems[Power].hBmp[0]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpPowerOff ,0,0);
  pRadio->pSkin->SItems[Power].hBmp[1]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpPowerOn ,0,0);
  QueryBitmapDimension( pRadio->pSkin->SItems[Power].hBmp[0],
                        &pRadio->pSkin->SItems[Power].Size );
  pRadio->pSkin->SItems[Stereo].ptlPos.x = 13;
  pRadio->pSkin->SItems[Stereo].ptlPos.y = 17;
  pRadio->pSkin->SItems[Stereo].hBmp[0]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpStereoOff ,0,0);
  pRadio->pSkin->SItems[Stereo].hBmp[1]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpStereoOn ,0,0);
  QueryBitmapDimension( pRadio->pSkin->SItems[Stereo].hBmp[0],
                        &pRadio->pSkin->SItems[Stereo].Size );
  pRadio->pSkin->SItems[Station1].ptlPos.x = 38;
  pRadio->pSkin->SItems[Station1].ptlPos.y = 17;
  pRadio->pSkin->SItems[Station1].hBmp[0]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpStation1Off ,0,0);
  pRadio->pSkin->SItems[Station1].hBmp[1]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpStation1On ,0,0);
  QueryBitmapDimension( pRadio->pSkin->SItems[Station1].hBmp[0],
                        &pRadio->pSkin->SItems[Station1].Size );
  pRadio->pSkin->SItems[Station2].ptlPos.x = 47;
  pRadio->pSkin->SItems[Station2].ptlPos.y = 17;
  pRadio->pSkin->SItems[Station2].hBmp[0]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpStation2Off ,0,0);
  pRadio->pSkin->SItems[Station2].hBmp[1]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpStation2On ,0,0);
  QueryBitmapDimension( pRadio->pSkin->SItems[Station2].hBmp[0],
                        &pRadio->pSkin->SItems[Station2].Size );
  pRadio->pSkin->SItems[Station1Set].ptlPos.x = 38;
  pRadio->pSkin->SItems[Station1Set].ptlPos.y = 14;
  pRadio->pSkin->SItems[Station1Set].hBmp[0]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpStoreEmtpy ,0,0);
  pRadio->pSkin->SItems[Station1Set].hBmp[1]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpStoreUsed ,0,0);
  QueryBitmapDimension( pRadio->pSkin->SItems[Station1Set].hBmp[0],
                        &pRadio->pSkin->SItems[Station1Set].Size );
  pRadio->pSkin->SItems[Station2Set].ptlPos.x = 47;
  pRadio->pSkin->SItems[Station2Set].ptlPos.y = 14;
  pRadio->pSkin->SItems[Station2Set].hBmp[0]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpStoreEmtpy ,0,0);
  pRadio->pSkin->SItems[Station2Set].hBmp[1]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpStoreUsed ,0,0);
  QueryBitmapDimension( pRadio->pSkin->SItems[Station2Set].hBmp[0],
                        &pRadio->pSkin->SItems[Station2Set].Size );

  pRadio->pSkin->SItems[ScanDown].ptlPos.x = 6;
  pRadio->pSkin->SItems[ScanDown].ptlPos.y = 5;
  pRadio->pSkin->SItems[ScanDown].hBmp[0]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpScanDownOff ,0,0);
  pRadio->pSkin->SItems[ScanDown].hBmp[1]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpScanDownOn ,0,0);
  QueryBitmapDimension( pRadio->pSkin->SItems[ScanDown].hBmp[0],
                        &pRadio->pSkin->SItems[ScanDown].Size );
  pRadio->pSkin->SItems[TuneDown].ptlPos.x = 16;
  pRadio->pSkin->SItems[TuneDown].ptlPos.y = 5;
  pRadio->pSkin->SItems[TuneDown].hBmp[0]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpTuneDownOff ,0,0);
  pRadio->pSkin->SItems[TuneDown].hBmp[1]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpTuneDownOn ,0,0);
  QueryBitmapDimension( pRadio->pSkin->SItems[TuneDown].hBmp[0],
                        &pRadio->pSkin->SItems[TuneDown].Size );
  pRadio->pSkin->SItems[TuneUp].ptlPos.x = 22;
  pRadio->pSkin->SItems[TuneUp].ptlPos.y = 5;
  pRadio->pSkin->SItems[TuneUp].hBmp[0]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpTuneUpOff ,0,0);
  pRadio->pSkin->SItems[TuneUp].hBmp[1]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpTuneUpOn ,0,0);
  QueryBitmapDimension( pRadio->pSkin->SItems[TuneUp].hBmp[0],
                        &pRadio->pSkin->SItems[TuneUp].Size );
  pRadio->pSkin->SItems[ScanUp].ptlPos.x = 28;
  pRadio->pSkin->SItems[ScanUp].ptlPos.y = 5;
  pRadio->pSkin->SItems[ScanUp].hBmp[0]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpScanUpOff ,0,0);
  pRadio->pSkin->SItems[ScanUp].hBmp[1]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpScanUpOn ,0,0);
  QueryBitmapDimension( pRadio->pSkin->SItems[ScanUp].hBmp[0],
                        &pRadio->pSkin->SItems[ScanUp].Size );

  pRadio->pSkin->SItems[Station3].ptlPos.x = 38;
  pRadio->pSkin->SItems[Station3].ptlPos.y = 8;
  pRadio->pSkin->SItems[Station3].hBmp[0]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpStation3Off ,0,0);
  pRadio->pSkin->SItems[Station3].hBmp[1]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpStation3On ,0,0);
  QueryBitmapDimension( pRadio->pSkin->SItems[Station3].hBmp[0],
                        &pRadio->pSkin->SItems[Station3].Size );
  pRadio->pSkin->SItems[Station4].ptlPos.x = 47;
  pRadio->pSkin->SItems[Station4].ptlPos.y = 8;
  pRadio->pSkin->SItems[Station4].hBmp[0]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpStation4Off ,0,0);
  pRadio->pSkin->SItems[Station4].hBmp[1]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpStation4On ,0,0);
  QueryBitmapDimension( pRadio->pSkin->SItems[Station4].hBmp[0],
                        &pRadio->pSkin->SItems[Station4].Size );
  pRadio->pSkin->SItems[Station3Set].ptlPos.x = 38;
  pRadio->pSkin->SItems[Station3Set].ptlPos.y = 5;
  pRadio->pSkin->SItems[Station3Set].hBmp[0]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpStoreEmtpy ,0,0);
  pRadio->pSkin->SItems[Station3Set].hBmp[1]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpStoreUsed ,0,0);
  QueryBitmapDimension( pRadio->pSkin->SItems[Station3Set].hBmp[0],
                        &pRadio->pSkin->SItems[Station3Set].Size );
  pRadio->pSkin->SItems[Station4Set].ptlPos.x = 47;
  pRadio->pSkin->SItems[Station4Set].ptlPos.y = 5;
  pRadio->pSkin->SItems[Station4Set].hBmp[0]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpStoreEmtpy ,0,0);
  pRadio->pSkin->SItems[Station4Set].hBmp[1]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpStoreUsed ,0,0);
  QueryBitmapDimension( pRadio->pSkin->SItems[Station4Set].hBmp[0],
                        &pRadio->pSkin->SItems[Station4Set].Size );

  pRadio->pSkin->HundretDigit.ptlPos.x = 56;
  pRadio->pSkin->HundretDigit.ptlPos.y = 7;
  pRadio->pSkin->HundretDigit.hBmp[0]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpNDigit100Off ,0,0);
  pRadio->pSkin->HundretDigit.hBmp[1]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpNDigit100 ,0,0);
  pRadio->pSkin->ptlNormalDigitPos[0].x = 59;
  pRadio->pSkin->ptlNormalDigitPos[0].y = 7;
  pRadio->pSkin->ptlNormalDigitPos[1].x = 69;
  pRadio->pSkin->ptlNormalDigitPos[1].y = 7;
  for (i=0;i<11;i++)
  {
    pRadio->pSkin->NormalDigits.hDigit[i] = GpiLoadBitmap(hps, g_hResDLL, ID_BmpNDigit0 + i ,0,0);
  }
  pRadio->pSkin->ptlSmallDigitPos[0].x = 79;
  pRadio->pSkin->ptlSmallDigitPos[0].y = 11;
  pRadio->pSkin->ptlSmallDigitPos[1].x = 87;
  pRadio->pSkin->ptlSmallDigitPos[1].y = 11;
  for (i=0;i<11;i++)
  {
    pRadio->pSkin->SmallDigits.hDigit[i] = GpiLoadBitmap(hps, g_hResDLL, ID_BmpSDigit0 + i ,0,0);
  }

  pRadio->pSkin->SItems[UsbPres].ptlPos.x = 95;
  pRadio->pSkin->SItems[UsbPres].ptlPos.y = 5;
  pRadio->pSkin->SItems[UsbPres].hBmp[0]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpUsbAbsent ,0,0);
  pRadio->pSkin->SItems[UsbPres].hBmp[1]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpUsbPresent ,0,0);
  QueryBitmapDimension( pRadio->pSkin->SItems[UsbPres].hBmp[0],
                        &pRadio->pSkin->SItems[UsbPres].Size );

  pRadio->pSkin->SItems[MHz].ptlPos.x = 95;
  pRadio->pSkin->SItems[MHz].ptlPos.y = 14;
  pRadio->pSkin->SItems[MHz].hBmp[0]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpMhzOff ,0,0);
  pRadio->pSkin->SItems[MHz].hBmp[1]  = GpiLoadBitmap(hps, g_hResDLL, ID_BmpMhzOn ,0,0);
  QueryBitmapDimension( pRadio->pSkin->SItems[MHz].hBmp[0],
                        &pRadio->pSkin->SItems[MHz].Size );

}

LONG GetSkinItemFromPos(PPOINTL pptlPos, PRADIOPRIVATE pRadio)
{
  LONG lItem;
  for(lItem = Power; lItem <=MHz;lItem++)
  {
    if( (pptlPos->x >= pRadio->pSkin->SItems[lItem].ptlPos.x) &&
        (pptlPos->x <= (pRadio->pSkin->SItems[lItem].ptlPos.x + 
                        pRadio->pSkin->SItems[lItem].Size.cx)) &&
        (pptlPos->y >= pRadio->pSkin->SItems[lItem].ptlPos.y) &&
        (pptlPos->y <= (pRadio->pSkin->SItems[lItem].ptlPos.y + 
                        pRadio->pSkin->SItems[lItem].Size.cy)) )
      return lItem;
  }
  return -1; // No Item Found
}

BOOL LoadUSBDLL()
{
  APIRET rc;
  char szError[260];
  if (!g_USBFuncs.hDLL)
  {
    rc = DosLoadModule(szError, sizeof(szError),"USBCALLS.DLL",&g_USBFuncs.hDLL);
    if (!rc)
    {
      rc = DosQueryProcAddr( g_USBFuncs.hDLL,
                             0, "UsbRegisterDeviceNotification",
                             (PFN*)&g_USBFuncs.pUsbRegisterDeviceNotification);
      if (!rc)
        rc = DosQueryProcAddr( g_USBFuncs.hDLL,
                               0, "UsbDeregisterNotification",
                               (PFN*)&g_USBFuncs.pUsbDeregisterNotification);
      if (!rc)
        rc = DosQueryProcAddr( g_USBFuncs.hDLL,
                               0, "UsbOpen",
                               (PFN*)&g_USBFuncs.pUsbOpen);
      if (!rc)
        rc = DosQueryProcAddr( g_USBFuncs.hDLL,
                               0, "UsbClose",
                               (PFN*)&g_USBFuncs.pUsbClose);
      if (!rc)
        rc = DosQueryProcAddr( g_USBFuncs.hDLL,
                               0, "UsbCtrlMessage",
                               (PFN*)&g_USBFuncs.pUsbCtrlMessage);
      if (rc)
        DosFreeModule( g_USBFuncs.hDLL );
    }
  }
  return(g_USBFuncs.hDLL!=NULL);
}


APIRET RadioSetFreq(ULONG ulNewFreq)
{
  double dFreq;
  ULONG ulFreq;
  dFreq = ulNewFreq / 100.0;
  USBHANDLE Handle;
  APIRET rc;
  UCHAR ucData[8];
/*
  if(japan)
  {
    ulFreq = ((dFreq-10.7)*80);
  }
  else
*/
  {
    ulFreq = ((dFreq+10.7)*80);
  }
  rc = g_USBFuncs.pUsbOpen( &Handle,
                            0x04b4,
                            0x1002,
                            USB_ANY_PRODUCTVERSION,
                            USB_OPEN_FIRST_UNUSED);
  if (!rc)
  {
    rc = g_USBFuncs.pUsbCtrlMessage( Handle,
                                     0xC0, 0x01,
                                     ulFreq>>8,ulFreq,
                                     1,(UCHAR*)&ucData,
                                     0);

    g_USBFuncs.pUsbClose(Handle);
  }
  return(rc);
}

BOOL RadioGetStereo()
{
  USBHANDLE Handle;
  APIRET rc;
  UCHAR ucData[8];
  BOOL fStereo = FALSE;
  rc = g_USBFuncs.pUsbOpen( &Handle,
                            0x04b4,
                            0x1002,
                            USB_ANY_PRODUCTVERSION,
                            USB_OPEN_FIRST_UNUSED);
  if (!rc)
  {
    rc = g_USBFuncs.pUsbCtrlMessage( Handle,
                                     0xC0, 0x00,
                                     0, 0x00,
                                     1,(UCHAR*)&ucData,
                                     0);
    fStereo = (ucData[0]&0x01)==0x00;

    g_USBFuncs.pUsbClose(Handle);
  }
  return(fStereo);
}

APIRET RadioPower(BOOL fTurnOn)
{
  USBHANDLE Handle;
  APIRET rc;
  UCHAR ucData[8];
  BOOL fStereo = FALSE;
  rc = g_USBFuncs.pUsbOpen( &Handle,
                            0x04b4,
                            0x1002,
                            USB_ANY_PRODUCTVERSION,
                            USB_OPEN_FIRST_UNUSED);
  if (!rc)
  {
    rc = g_USBFuncs.pUsbCtrlMessage( Handle,
                                     0xC0, 0x02,
                                     fTurnOn?1:0, 0,
                                     1,(UCHAR*)&ucData,
                                     0);
    g_USBFuncs.pUsbClose(Handle);
  }
  return(rc);
}

/* ******************************************************************
 *
 *   Widget setup management
 *
 ********************************************************************/

/*
 *      This section contains shared code to manage the
 *      widget's settings. This can translate a widget
 *      setup string into the fields of a binary setup
 *      structure and vice versa. This code is used by
 *      an open widget window, but could be shared with
 *      a settings dialog, if you implement one.
 */

/*
 *@@ WgtClearSetup:
 *      cleans up the data in the specified setup
 *      structure, but does not free the structure
 *      itself.
 */

VOID WgtClearSetup(PRADIOSETUP pSetup)
{
  if (pSetup)
  {
    pSetup->fTurnOn        = FALSE;
    pSetup->fOnOnAttach    = TRUE;
    pSetup->ulCurrentFreq   = 88.0;
    pSetup->ulStoredFreq[0] = 0;
    pSetup->ulStoredFreq[1] = 0;
    pSetup->ulStoredFreq[2] = 0;
    pSetup->ulStoredFreq[3] = 0;
  }
}

/*
 *@@ WgtScanSetup:
 *      scans the given setup string and translates
 *      its data into the specified binary setup
 *      structure.
 *
 *      NOTE: It is assumed that pSetup is zeroed
 *      out. We do not clean up previous data here.
 */

VOID WgtScanSetup(const char *pcszSetupString,
                  PRADIOSETUP pSetup)
{
  PSZ p;
  char szStation[] = "STATION-0";
  // On Off setting
  p = pctrScanSetupString( pcszSetupString,
                           "RADIO_POWER");
  if (p!=NULL)
  {
    pSetup->fTurnOn = stricmp(p,"ON")==0;
    pctrFreeSetupValue(p);
  }
  else
    // default to Off
    pSetup->fTurnOn = FALSE;

  // turn of if a USB radio is attached
  p = pctrScanSetupString(pcszSetupString,
                          "ONONATTACH");
  if (p!=NULL)
  {
    pSetup->fOnOnAttach = stricmp(p,"YES")==0;
    pctrFreeSetupValue(p);
  }
  else
    pSetup->fOnOnAttach = FALSE;

  // font:
  // we set the font presparam, which automatically
  // affects the cached presentation spaces
  p = pctrScanSetupString(pcszSetupString,
                          "FREQENCY");
  if (p!=NULL)
  {
    ULONG NewFreq = atol(p);
    if ( NewFreq >=8800 && NewFreq<=10800)
      pSetup->ulCurrentFreq = NewFreq;
    pctrFreeSetupValue(p);
  }
  else
    pSetup->ulCurrentFreq = 10760;

  for (char i=0;i<4;i++)
  {
    szStation[strlen(szStation)] = '0'+i;
    p = pctrScanSetupString(pcszSetupString,
                            szStation);
    if (p!=NULL)
    {
      ULONG NewFreq = atol(p);
      if ( NewFreq >=8800 && NewFreq<=10800)
        pSetup->ulStoredFreq[i] = NewFreq;
      else
        pSetup->ulStoredFreq[i] = 0;
      pctrFreeSetupValue(p);
    }
    else
      pSetup->ulStoredFreq[i] = 0;

  }

}

/*
 *@@ WgtSaveSetup:
 *      composes a new setup string.
 *      The caller must invoke xstrClear on the
 *      string after use.
 */

VOID WgtSaveSetup(PXSTRING pstrSetup,       // out: setup string (is cleared first)
                  PRADIOSETUP pSetup)
{
  char szStation[] = "STATION-0";
  CHAR    szTemp[100];

  pxstrInit(pstrSetup, 200);

  sprintf(szTemp, "RADIO_POWER=%s;",
          pSetup->fTurnOn?"ON":"OFF");
  pxstrcat(pstrSetup, szTemp, 0);

  sprintf(szTemp, "ONONATTACH=%s;",
          pSetup->fOnOnAttach?"YES":"NO");
  pxstrcat(pstrSetup, szTemp, 0);

  sprintf(szTemp, "FREQENCY=%d;",
          pSetup->ulCurrentFreq);
  pxstrcat(pstrSetup, szTemp, 0);

  for (char i=0;i<4;i++)
  {
    szStation[strlen(szStation)] = '0'+i;
    if ( pSetup->ulStoredFreq[i] >=8800 &&
         pSetup->ulStoredFreq[i] <=10800)
    {
      sprintf(szTemp, "%s=%d;",
              szStation,
              pSetup->ulStoredFreq[i]);
      pxstrcat(pstrSetup, szTemp, 0);
    }
  }
}

void ProcessRadioUnpluged(PRADIOPRIVATE pRadio)
{
  pRadio->ulNumRadios--;
  if (!pRadio->ulNumRadios)
  {
    DosBeep(440,50);
    DosBeep(220,50);
    pRadio->ulState = RADIO_USBCHECK;
    pRadio->Setup.fTurnOn = FALSE;
  }
  WinInvalidateRegion(pRadio->pWidget->hwndWidget, NULL, TRUE);
}

void ProcessRadioPluged(PRADIOPRIVATE pRadio)
{
  APIRET rc;
  if (!pRadio->ulNumRadios)
  {
    if (pRadio->Setup.fOnOnAttach)
    {
      DosBeep(220,50);
      DosBeep(440,50);
      pRadio->Setup.fTurnOn = TRUE;
      DosPostEventSem(pRadio->hEvtFreqChange);
    }
  }
  pRadio->ulNumRadios++;
  WinInvalidateRegion(pRadio->pWidget->hwndWidget, NULL, TRUE);
}

void _Optlink NotifyThread(void* args)
{
  PRADIOPRIVATE pRadio = (PRADIOPRIVATE)args;
  ULONG ulWhich;
  ULONG ulCnt;
  pRadio->ulState |= RADIO_USBCHECK;
  while (pRadio->ulState  & RADIO_USBCHECK)
  {
    DosWaitMuxWaitSem(pRadio->hMuxWait,SEM_INDEFINITE_WAIT,&ulWhich);
    if (!pRadio->ulState)
      break;
    if (ulWhich)
    {
      ProcessRadioUnpluged(pRadio);
      DosResetEventSem(pRadio->hRadioPluged, &ulCnt);
    }
    else
    {
      ProcessRadioPluged(pRadio);
      DosResetEventSem(pRadio->hRadioPluged, &ulCnt);
    }
  }
  g_USBFuncs.pUsbDeregisterNotification(pRadio->NotifyID);
  DosCloseMutexSem(pRadio->hMuxWait);
  DosCloseEventSem(pRadio->hRadioPluged);
  DosCloseEventSem(pRadio->hRadioUnpluged);

}

void _Optlink FreqThread(void* args)
{
  PRADIOPRIVATE pRadio = (PRADIOPRIVATE)args;
  ULONG ulCnt;
  while (pRadio->ulState  & RADIO_USBCHECK)
  {
    DosWaitEventSem(pRadio->hEvtFreqChange, SEM_INDEFINITE_WAIT);
    if (!pRadio->ulState)
      break;
    do
    {
      if (pRadio->ulState & RADIO_SCANUP ||
          pRadio->ulState & RADIO_TUNEUP)
      {
        pRadio->Setup.ulCurrentFreq += 5;
        if(pRadio->Setup.ulCurrentFreq > 10800)
          pRadio->Setup.ulCurrentFreq = 8800;
      }
      else
        if (pRadio->ulState & RADIO_SCANDOWN ||
            pRadio->ulState & RADIO_TUNEDOWN)
        {
          pRadio->Setup.ulCurrentFreq -= 5;
          if(pRadio->Setup.ulCurrentFreq < 8800)
            pRadio->Setup.ulCurrentFreq = 10800;
        }
      if( pRadio->Setup.ulCurrentFreq == pRadio->ulScanStart)
        pRadio->ulState &= ~RADIO_SCAN;

      if(pRadio->ulNumRadios &&
         pRadio->Setup.fTurnOn)
      {
        XSTRING strSetup;

        RadioSetFreq(pRadio->Setup.ulCurrentFreq);
        WinInvalidateRect(pRadio->pWidget->hwndWidget,NULL, FALSE);
        DosSleep(80);
        if(RadioGetStereo())
        {
          pRadio->ulState |= RADIO_STEREO;
          pRadio->ulState &= ~RADIO_SCAN;  // End Scanning if we tuned in.
        }
        else
          pRadio->ulState &= ~RADIO_STEREO;
        WgtSaveSetup(&strSetup,
                     &pRadio->Setup);
        //pData->pctrSetSetupString(pData->hSettings,
        //                          strSetup.psz);
        pxstrClear(&strSetup);
      }
      pRadio->ulState &= ~RADIO_TUNE;      
      WinInvalidateRect(pRadio->pWidget->hwndWidget,NULL, FALSE);
    }
    while(pRadio->ulState & RADIO_SCAN);
    DosResetEventSem(pRadio->hEvtFreqChange, &ulCnt);
  }
  DosCloseEventSem(pRadio->hEvtFreqChange);
}

/* ******************************************************************
 *
 *   Widget settings dialog
 *
 ********************************************************************/

// None currently. To see how a setup dialog can be done,
// see the window list widget (w_winlist.c).

/* ******************************************************************
 *
 *   Callbacks stored in XCENTERWIDGETCLASS
 *
 ********************************************************************/

// If you implement a settings dialog, you must write a
// "show settings dlg" function and store its function pointer
// in XCENTERWIDGETCLASS.

/* ******************************************************************
 *
 *   PM window class implementation
 *
 ********************************************************************/

/*
 *      This code has the actual PM window class.
 *
 */

/*
 *@@ WgtCreate:
 *      implementation for WM_CREATE.
 */

MRESULT WgtCreate(HWND hwnd,
                  PXCENTERWIDGET pWidget)
{
  MRESULT mrc = 0;
  APIRET rc;
  // PSZ p;
  PRADIOPRIVATE pRadio = (PRADIOPRIVATE)malloc(sizeof(RADIOPRIVATE));
  memset(pRadio, 0, sizeof(RADIOPRIVATE));
  // link the two together
  pWidget->pUser = pRadio;
  pRadio->pWidget = pWidget;

  // initialize binary setup structure from setup string
  WgtScanSetup(pWidget->pcszSetupString,
               &pRadio->Setup);

  rc = DosCreateEventSem(NULL,&pRadio->hEvtFreqChange,0,FALSE);
  if(!rc)
  {
    pRadio->ulState = RADIO_USBCHECK;
    pRadio->hFreqThread = _beginthread(FreqThread,NULL,8192,pRadio);
    LoadUSBDLL();
    if (g_USBFuncs.hDLL)
    {
      rc =DosCreateEventSem(NULL,&pRadio->hRadioPluged,0,FALSE);
      if (!rc)
      {
        rc = DosCreateEventSem(NULL,&pRadio->hRadioUnpluged,0,FALSE);
        if (rc)
          DosCloseEventSem(pRadio->hRadioPluged);
        else
        {
          SEMRECORD aSems[2];
          aSems[0].hsemCur = (HSEM)pRadio->hRadioPluged;
          aSems[0].ulUser  = 0;
          aSems[1].hsemCur = (HSEM)pRadio->hRadioUnpluged;
          aSems[1].ulUser  = 1;
          rc = DosCreateMuxWaitSem(NULL, &pRadio->hMuxWait, 2, (PSEMRECORD)&aSems, DCMW_WAIT_ANY);
          if (!rc)
          {
            pRadio->hMonThread = _beginthread(NotifyThread,NULL,8192,pRadio);
            rc= g_USBFuncs.pUsbRegisterDeviceNotification( &pRadio->NotifyID,
                                                           pRadio->hRadioPluged,
                                                           pRadio->hRadioUnpluged,
                                                           0x04b4, 0x1002, USB_ANY_PRODUCTVERSION
                                                         );
          }
          else
          {
            DosCloseEventSem(pRadio->hRadioPluged);
            DosCloseEventSem(pRadio->hRadioUnpluged);
          }
        }
      }
      
    }
  }
  HPS hps =WinGetPS(hwnd);
  LoadSkin(hps,NULL, pRadio);
  WinReleasePS(hwnd);
  // if you want the context menu help to be enabled,
  // add your help library here; if these fields are
  // left NULL, the "Help" context menu item is disabled

  // pWidget->pcszHelpLibrary = pcmnQueryHelpLibrary();
  // pWidget->ulHelpPanelID = ID_XSH_WIDGET_WINLIST_MAIN;

  return(mrc);
}

/*
 *@@ MwgtControl:
 *      implementation for WM_CONTROL.
 *
 *      The XCenter communicates with widgets thru
 *      WM_CONTROL messages. At the very least, the
 *      widget should respond to XN_QUERYSIZE because
 *      otherwise it will be given some dumb default
 *      size.
 *
 *@@added V0.9.7 (2000-12-14) [umoeller]
 */

char szTTip[200];

BOOL WgtControl(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
  BOOL brc = FALSE;

  // get widget data from QWL_USER (stored there by WM_CREATE)
  PXCENTERWIDGET pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER);
  if (pWidget)
  {
    // get private data from that widget data
    PRADIOPRIVATE pRadio = (PRADIOPRIVATE)pWidget->pUser;
    if (pRadio)
    {
      USHORT  usID = SHORT1FROMMP(mp1),
                     usNotifyCode = SHORT2FROMMP(mp1);

      // is this from the XCenter client?
      switch (usID)
      {
        case ID_XCENTER_CLIENT:
          {
            // yes:

            switch (usNotifyCode)
            {
              /*
               * XN_QUERYSIZE:
               *      XCenter wants to know our size.
               */
              
              case XN_QUERYSIZE:
                {
                  PSIZEL pszl = (PSIZEL)mp2;
                  // @@ToDo Get the size from the bitmapdimensions.
                  pszl->cx = 118;      // desired width
                  pszl->cy = 31;      // desired minimum height
                  brc = TRUE;
                  break;}

                /*
                 * XN_SETUPCHANGED:
                 *      XCenter has a new setup string for
                 *      us in mp2.
                 *
                 *      NOTE: This only comes in with settings
                 *      dialogs. Since we don't have one, this
                 *      really isn't necessary.
                 */

              case XN_SETUPCHANGED:
                {
                  const char *pcszNewSetupString = (const char*)mp2;

                  // reinitialize the setup data
                  WgtClearSetup(&pRadio->Setup);
                  WgtScanSetup(pcszNewSetupString, &pRadio->Setup);

                  WinInvalidateRect(pWidget->hwndWidget, NULL, FALSE);
                  break;
                }
            }
          }
        case ID_XCENTER_TOOLTIP:
          {
            switch (usNotifyCode)
            {
              case TTN_NEEDTEXT:
              {
                POINTL ptlTTip;
                WinQueryPointerPos(HWND_DESKTOP,&ptlTTip);
                WinMapWindowPoints( HWND_DESKTOP,
                                    pRadio->pWidget->hwndWidget,
                                    &ptlTTip,1);
                switch (GetSkinItemFromPos(&ptlTTip,pRadio))
                {
                  case Power:     
                    strcpy(szTTip,"Power");
                    break;
                  case Stereo:    
                    strcpy(szTTip,"Stereo Indicator\nOn,Radio is Tuned");
                    break;
                  case UsbPres:   
                    strcpy(szTTip,"USB Support Indicator");
                    break;
                  case ScanUp:    
                    strcpy(szTTip,"Scan Up");
                    break;
                  case ScanDown:  
                    strcpy(szTTip,"Scan Down");
                    break;
                  case TuneUp:    
                    strcpy(szTTip,"Tune Up");
                    break;
                  case TuneDown:  
                    strcpy(szTTip,"Tune Down");
                    break;
                  case Station1:  
                  case Station2:  
                  case Station3:  
                  case Station4:  
                    strcpy(szTTip,"Station Storage");
                    break;
                  case Station1Set:
                  case Station2Set:
                  case Station3Set:
                  case Station4Set:
                    strcpy(szTTip,"Station Store usage");
                    break;
                  case MHz:
                    strcpy(szTTip,"Radio presence indicator");
                    break;
                  default:
                    strcpy(szTTip,"USB Radio");
                    break;
                }
                PTOOLTIPTEXT pttt = (PTOOLTIPTEXT)mp2;
                pttt->pszText = szTTip;
                pttt->ulFormat = TTFMT_PSZ;
              }
              break;
              case TTN_SHOW:
              {
                pRadio->fTooltipShowing = TRUE;
              }
              break;
              case TTN_POP:
                pRadio->fTooltipShowing = FALSE;
              break;
            }
          }
      }
    } // end if (pRadio)
  } // end if (pWidget)

  return(brc);
}

VOID WgtPaint(HWND hwnd,
              PXCENTERWIDGET pWidget);

BOOL WgtButtonclick(HWND hwnd, MPARAM mp1, MPARAM mp2)
{
  BOOL brc = FALSE;
  POINTL ptlPos;
// get widget data from QWL_USER (stored there by WM_CREATE)
  PXCENTERWIDGET pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER);
  if(pWidget)
  {
    PRADIOPRIVATE pRadio = (PRADIOPRIVATE)pWidget->pUser;
    if(pRadio)
    {
      WinQueryPointerPos( HWND_DESKTOP, &ptlPos);
      WinMapWindowPoints( HWND_DESKTOP,
                          pRadio->pWidget->hwndWidget,
                          &ptlPos,1);
      LONG lItem = GetSkinItemFromPos(&ptlPos,pRadio);
      switch (lItem)
      {
        case Power: 
          if(pRadio->ulNumRadios)
          {
            if(pRadio->Setup.fTurnOn)
              RadioPower(FALSE);
            pRadio->Setup.fTurnOn = !pRadio->Setup.fTurnOn;
            DosPostEventSem(pRadio->hEvtFreqChange);
          }
          brc = TRUE;
          break;
        case UsbPres:   
          if(!g_USBFuncs.hDLL && LoadUSBDLL())
          {
            WinInvalidateRect(hwnd, NULL, TRUE);
          }
          brc = TRUE;
          break;
        case ScanUp:    
          if(pRadio->Setup.fTurnOn)
          {
            if(pRadio->ulState & RADIO_TUNE)
            {
              DosBeep(440,10);
            }
            else
            {
              if(pRadio->ulState & RADIO_SCAN)
              {
                pRadio->ulState &= ~RADIO_SCAN;
              }
              pRadio->ulState |= RADIO_SCANUP;
              pRadio->ulScanStart = pRadio->Setup.ulCurrentFreq;
              DosPostEventSem(pRadio->hEvtFreqChange);
            }
          }
          brc = TRUE;
          break;
        case ScanDown:  
          if(pRadio->Setup.fTurnOn)
          {
            if(pRadio->ulState & RADIO_TUNE)
            {
              DosBeep(440,10);
            }
            else
            {
              if(pRadio->ulState & RADIO_SCAN)
              {
                pRadio->ulState &= ~RADIO_SCAN;
              }
              pRadio->ulState |= RADIO_SCANDOWN;
              pRadio->ulScanStart = pRadio->Setup.ulCurrentFreq;
              DosPostEventSem(pRadio->hEvtFreqChange);
            }
          }
          brc = TRUE;
          break;
        case TuneUp:    
          if(pRadio->Setup.fTurnOn)
          {
            if(pRadio->ulState & RADIO_SCAN)
            {
              pRadio->ulState &= ~RADIO_SCAN;
            }
            pRadio->ulState |= RADIO_TUNEUP;
            DosPostEventSem(pRadio->hEvtFreqChange);
          }
          brc = TRUE;
          break;
        case TuneDown:  
          if(pRadio->Setup.fTurnOn)
          {
            if(pRadio->ulState & RADIO_SCAN)
            {
              pRadio->ulState &= ~RADIO_SCAN;
            }
            pRadio->ulState |= RADIO_TUNEDOWN;
            DosPostEventSem(pRadio->hEvtFreqChange);
          }
          brc = TRUE;
          break;
        case Station1:  
        case Station2:  
        case Station3:  
        case Station4:  
          if(pRadio->Setup.fTurnOn)
          {
            if( (pRadio->Setup.ulStoredFreq[lItem-Station1]>=8800) &&
                (pRadio->Setup.ulStoredFreq[lItem-Station1]<=10800) )
            {
              pRadio->Setup.ulCurrentFreq =
                pRadio->Setup.ulStoredFreq[lItem-Station1];
              DosPostEventSem(pRadio->hEvtFreqChange);
            }
          }
          break;
        case Station1Set:
        case Station2Set:
        case Station3Set:
        case Station4Set:
          if(pRadio->Setup.fTurnOn)
          {
            pRadio->Setup.ulStoredFreq[lItem-Station1Set] = 
              pRadio->Setup.ulCurrentFreq;
            WinInvalidateRect(hwnd, NULL, TRUE);
          }
          break;
        case MHz:
          break;
        default:
          break;
      }
    }
  }
  return brc;
}

VOID DrawFreq(HPS hps, POINTL *ptlStart, PRADIOPRIVATE pRadio)
{
  // Freq Display
  ULONG ulFreq;
  POINTL ptlPos;
  ldiv_t calc;
  ptlPos.x = ptlStart->x + pRadio->pSkin->HundretDigit.ptlPos.x;
  ptlPos.y = ptlStart->y + pRadio->pSkin->HundretDigit.ptlPos.y;

  ulFreq = (pRadio->Setup.ulCurrentFreq>10800)?8800:
           (pRadio->Setup.ulCurrentFreq<8800)?8800:
           pRadio->Setup.ulCurrentFreq;

  WinDrawBitmap( hps, 
                 pRadio->pSkin->HundretDigit.hBmp[ulFreq>10000?ON:OFF],
                 NULL, &ptlPos,0,0, DBM_NORMAL);
  if (ulFreq>10000)
    ulFreq -=10000;

  ptlPos.x = ptlStart->x + pRadio->pSkin->ptlNormalDigitPos[0].x;
  ptlPos.y = ptlStart->y + pRadio->pSkin->ptlNormalDigitPos[0].y;
  calc = ldiv(ulFreq,1000);
  WinDrawBitmap( hps, 
                 pRadio->pSkin->NormalDigits.hDigit[calc.quot],
                 NULL, &ptlPos,0,0, DBM_NORMAL);

  ulFreq -= calc.quot*1000;
  ptlPos.x = ptlStart->x + pRadio->pSkin->ptlNormalDigitPos[1].x;
  ptlPos.y = ptlStart->y + pRadio->pSkin->ptlNormalDigitPos[1].y;
  calc = ldiv(ulFreq,100);
  WinDrawBitmap( hps, 
                 pRadio->pSkin->NormalDigits.hDigit[calc.quot],
                 NULL, &ptlPos,0,0, DBM_NORMAL);

  ulFreq -= calc.quot*100;
  ptlPos.x = ptlStart->x + pRadio->pSkin->ptlSmallDigitPos[0].x;
  ptlPos.y = ptlStart->y + pRadio->pSkin->ptlSmallDigitPos[0].y;
  calc = ldiv(ulFreq,10);
  WinDrawBitmap( hps, 
                 pRadio->pSkin->SmallDigits.hDigit[calc.quot],
                 NULL, &ptlPos,0,0, DBM_NORMAL);

  ulFreq -= calc.quot*10;
  ptlPos.y = ptlStart->y + pRadio->pSkin->ptlSmallDigitPos[1].y;
  ptlPos.x = ptlStart->x + pRadio->pSkin->ptlSmallDigitPos[1].x;
  WinDrawBitmap( hps, 
                 pRadio->pSkin->SmallDigits.hDigit[ulFreq],
                 NULL, &ptlPos,0,0, DBM_NORMAL);

  ptlPos.y = ptlStart->y + pRadio->pSkin->SItems[MHz].ptlPos.y;
  ptlPos.x = ptlStart->x + pRadio->pSkin->SItems[MHz].ptlPos.x;
  WinDrawBitmap( hps, 
                 pRadio->pSkin->SItems[MHz].hBmp[ON],
                 NULL, &ptlPos,0,0, DBM_NORMAL);
}

void DrawStereo(HPS hps, POINTL *ptlStart ,PRADIOPRIVATE pRadio)
{
  POINTL ptlPos;
  ptlPos.x = ptlStart->x + pRadio->pSkin->SItems[Stereo].ptlPos.x;
  ptlPos.y = ptlStart->y + pRadio->pSkin->SItems[Stereo].ptlPos.y;
  WinDrawBitmap( hps, 
                 pRadio->pSkin->SItems[Stereo].hBmp[(pRadio->ulState & RADIO_STEREO)?ON:OFF],
                 NULL, &ptlPos,0,0, DBM_NORMAL);
}

void DrawStations(HPS hps, POINTL *ptlStart ,PRADIOPRIVATE pRadio)
{
  POINTL ptlPos;
  USHORT i;
  for (i=0;i<4;i++)
  {
    ptlPos.x = ptlStart->x + pRadio->pSkin->SItems[Station1+i].ptlPos.x;
    ptlPos.y = ptlStart->y + pRadio->pSkin->SItems[Station1+i].ptlPos.y;
    WinDrawBitmap( hps, 
                   pRadio->Setup.fTurnOn?
                     pRadio->pSkin->SItems[Station1+i].hBmp[(pRadio->Setup.ulStoredFreq[i] == pRadio->Setup.ulCurrentFreq)?ON:OFF]:
                     pRadio->pSkin->SItems[Station1+i].hBmp[OFF],
                   NULL, &ptlPos,0,0, DBM_NORMAL);
    ptlPos.x = ptlStart->x + pRadio->pSkin->SItems[Station1Set+i].ptlPos.x;
    ptlPos.y = ptlStart->y + pRadio->pSkin->SItems[Station1Set+i].ptlPos.y;
    WinDrawBitmap( hps, 
                   pRadio->Setup.fTurnOn?
                     pRadio->pSkin->SItems[Station1Set+i].hBmp[(pRadio->Setup.ulStoredFreq[i] != 0)?ON:OFF]:
                     pRadio->pSkin->SItems[Station1Set+i].hBmp[OFF],
                   NULL, &ptlPos,0,0, DBM_NORMAL);
  }
}

void DrawControls(HPS hps, POINTL *ptlStart ,PRADIOPRIVATE pRadio)
{
  POINTL ptlPos;
  ptlPos.x = ptlStart->x + pRadio->pSkin->SItems[Power].ptlPos.x;
  ptlPos.y = ptlStart->y + pRadio->pSkin->SItems[Power].ptlPos.y;
  WinDrawBitmap( hps, 
                 pRadio->pSkin->SItems[Power].hBmp[ON],
                 NULL, &ptlPos,0,0, DBM_NORMAL);
  ptlPos.x = ptlStart->x + pRadio->pSkin->SItems[ScanDown].ptlPos.x;
  ptlPos.y = ptlStart->y + pRadio->pSkin->SItems[ScanDown].ptlPos.y;
  WinDrawBitmap( hps, 
                 pRadio->pSkin->SItems[ScanDown].hBmp[ON],
                 NULL, &ptlPos,0,0, DBM_NORMAL);
  ptlPos.x = ptlStart->x + pRadio->pSkin->SItems[TuneDown].ptlPos.x;
  ptlPos.y = ptlStart->y + pRadio->pSkin->SItems[TuneDown].ptlPos.y;
  WinDrawBitmap( hps, 
                 pRadio->pSkin->SItems[TuneDown].hBmp[ON],
                 NULL, &ptlPos,0,0, DBM_NORMAL);
  ptlPos.x = ptlStart->x + pRadio->pSkin->SItems[TuneUp].ptlPos.x;
  ptlPos.y = ptlStart->y + pRadio->pSkin->SItems[TuneUp].ptlPos.y;
  WinDrawBitmap( hps, 
                 pRadio->pSkin->SItems[TuneUp].hBmp[ON],
                 NULL, &ptlPos,0,0, DBM_NORMAL);
  ptlPos.x = ptlStart->x + pRadio->pSkin->SItems[ScanUp].ptlPos.x;
  ptlPos.y = ptlStart->y + pRadio->pSkin->SItems[ScanUp].ptlPos.y;
  WinDrawBitmap( hps, 
                 pRadio->pSkin->SItems[ScanUp].hBmp[ON],
                 NULL, &ptlPos,0,0, DBM_NORMAL);
}

/*
 *@@ WgtPaint:
 *      implementation for WM_PAINT.
 *
 *      This really does nothing, except painting a
 *      3D rectangle and printing a question mark.
 */

VOID WgtPaint(HWND hwnd,
              PXCENTERWIDGET pWidget)
{
  HPS hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);
  if (hps)
  {
    RECTL       rclWin;
    PRADIOPRIVATE pRadio = (PRADIOPRIVATE)pWidget->pUser;
    WinQueryWindowRect(hwnd,
                       &rclWin);        // exclusive
    if (pRadio->pSkin)
    {
      POINTL ptlPos;
      ptlPos.x = 0;
      ptlPos.y = 0;
      WinDrawBitmap( hps, 
                     pRadio->pSkin->hBackground, 
                     NULL, &ptlPos, 0,0, DBM_NORMAL);
      ptlPos.x = rclWin.xLeft   + pRadio->pSkin->SItems[UsbPres].ptlPos.x;
      ptlPos.y = rclWin.yBottom + pRadio->pSkin->SItems[UsbPres].ptlPos.y;
      WinDrawBitmap( hps, 
                     pRadio->pSkin->SItems[UsbPres].hBmp[g_USBFuncs.hDLL!=NULL?ON:OFF],
                     NULL, &ptlPos,0,0, DBM_NORMAL);

      if (pRadio->Setup.fTurnOn)
      {
        DrawFreq(hps,(PPOINTL)&rclWin,pRadio);
        DrawStereo(hps,(PPOINTL)&rclWin,pRadio);
        DrawStations(hps,(PPOINTL)&rclWin,pRadio);
        DrawControls(hps,(PPOINTL)&rclWin,pRadio);
      }
      else
      {
        if (pRadio->ulNumRadios)
        {
          DrawStations(hps,(PPOINTL)&rclWin,pRadio);
          DrawControls(hps,(PPOINTL)&rclWin,pRadio);
        }
      }
    }
    else
    {
      WinFillRect(hps,&rclWin,CLR_WHITE);
      WinDrawText(hps, 0, "Init Error", &rclWin, CLR_BLACK, CLR_WHITE,DT_VCENTER|DT_CENTER);
    }
    WinEndPaint(hps);
  }
}

/*
 *@@ WgtPresParamChanged:
 *      implementation for WM_PRESPARAMCHANGED.
 *
 *      While this isn't exactly required, it's a nice
 *      thing for a widget to react to colors and fonts
 *      dropped on it. While we're at it, we also save
 *      these colors and fonts in our setup string data.
 *
 *@@changed V0.9.13 (2001-06-21) [umoeller]: changed XCM_SAVESETUP call for tray support
 */

VOID WgtPresParamChanged(HWND hwnd,
                         ULONG ulAttrChanged,
                         PXCENTERWIDGET pWidget)
{
  PRADIOPRIVATE pRadio = (PRADIOPRIVATE)pWidget->pUser;
  if (pRadio)
  {
    BOOL fInvalidate = TRUE;
    switch (ulAttrChanged)
    {
      case 0:     // layout palette thing dropped
      case PP_BACKGROUNDCOLOR:    // background color (no ctrl pressed)
      case PP_FOREGROUNDCOLOR:    // foreground color (ctrl pressed)
        // update our setup data; the presparam has already
        // been changed, so we can just query it
#if 0
        pRadio->Setup.lcolBackground
        = pwinhQueryPresColor(hwnd,
                              PP_BACKGROUNDCOLOR,
                              FALSE,
                              SYSCLR_DIALOGBACKGROUND);
        pRadio->Setup.lcolForeground
        = pwinhQueryPresColor(hwnd,
                              PP_FOREGROUNDCOLOR,
                              FALSE,
                              SYSCLR_WINDOWSTATICTEXT);
#endif
        break;

      case PP_FONTNAMESIZE:       // font dropped:
        break;

      default:
        fInvalidate = FALSE;
    }
    fInvalidate = FALSE;

    if (fInvalidate)
    {
      // something has changed:
      XSTRING strSetup;

      // repaint
      WinInvalidateRect(hwnd, NULL, FALSE);

      // recompose our setup string
      WgtSaveSetup(&strSetup,
                   &pRadio->Setup);
      if (strSetup.ulLength)
        // we have a setup string:
        // tell the XCenter to save it with the XCenter data
        // changed V0.9.13 (2001-06-21) [umoeller]:
        // post it to parent instead of fixed XCenter client
        // to make this trayable
        WinSendMsg(WinQueryWindow(hwnd, QW_PARENT), // pRadio->pWidget->pGlobals->hwndClient,
                   XCM_SAVESETUP,
                   (MPARAM)hwnd,
                   (MPARAM)strSetup.psz);
      pxstrClear(&strSetup);
    }
  } // end if (pRadio)
}

/*
 *@@ WgtDestroy:
 *      implementation for WM_DESTROY.
 *
 *      This must clean up all allocated resources.
 */

VOID WgtDestroy(HWND hwnd,
                PXCENTERWIDGET pWidget)
{
  PRADIOPRIVATE pRadio = (PRADIOPRIVATE)pWidget->pUser;
  if (pRadio)
  {
    WgtClearSetup(&pRadio->Setup);
    pRadio->ulState = 0;
    DosPostEventSem(pRadio->hRadioPluged);
    DosWaitThread(&pRadio->hMonThread, DCWW_WAIT);
    DosPostEventSem(pRadio->hEvtFreqChange);
    DosWaitThread(&pRadio->hFreqThread, DCWW_WAIT);
    if (g_USBFuncs.hDLL)
      DosFreeModule(g_USBFuncs.hDLL);
    free(pRadio);
    // pWidget is cleaned up by DestroyWidgets
  }
}

/*
 *@@ fnwpSampleWidget:
 *      window procedure for the winlist widget class.
 *
 *      There are a few rules which widget window procs
 *      must follow. See XCENTERWIDGETCLASS in center.h
 *      for details.
 *
 *      Other than that, this is a regular window procedure
 *      which follows the basic rules for a PM window class.
 */

MRESULT EXPENTRY fnwpSampleWidget(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  MRESULT mrc = 0;
  // get widget data from QWL_USER (stored there by WM_CREATE)
  PXCENTERWIDGET pWidget = (PXCENTERWIDGET)WinQueryWindowPtr(hwnd, QWL_USER);
  // this ptr is valid after WM_CREATE

  switch (msg)
  {
    /*
     * WM_CREATE:
     *      as with all widgets, we receive a pointer to the
     *      XCENTERWIDGET in mp1, which was created for us.
     *
     *      The first thing the widget MUST do on WM_CREATE
     *      is to store the XCENTERWIDGET pointer (from mp1)
     *      in the QWL_USER window word by calling:
     *
     *          WinSetWindowPtr(hwnd, QWL_USER, mp1);
     *
     *      We use XCENTERWIDGET.pUser for allocating
     *      SAMPLEPRIVATE for our own stuff.
     */
    
    case WM_CREATE:
      WinSetWindowPtr(hwnd, QWL_USER, mp1);
      pWidget = (PXCENTERWIDGET)mp1;
      if ((pWidget) && (pWidget->pfnwpDefWidgetProc))
        mrc = WgtCreate(hwnd, pWidget);
      else
        // stop window creation!!
        mrc = (MPARAM)TRUE;
      break;

      /*
       * WM_CONTROL:
       *      process notifications/queries from the XCenter.
       */

    case WM_CONTROL:
      mrc = (MPARAM)WgtControl(hwnd, mp1, mp2);
      break;

      /*
       * WM_PAINT:
       *
       */

    case WM_PAINT:
      WgtPaint(hwnd, pWidget);
      break;

      /*
       * WM_PRESPARAMCHANGED:
       *
       */

    case WM_PRESPARAMCHANGED:
      if (pWidget)
        // this gets sent before this is set!
        WgtPresParamChanged(hwnd, (ULONG)mp1, pWidget);
      break;

      /*
       * WM_DESTROY:
       *      clean up. This _must_ be passed on to
       *      ctrDefWidgetProc.
       */
    case WM_BUTTON1CLICK:
      mrc = (MPARAM)WgtButtonclick(hwnd, mp1, mp2);
      break;
    case WM_DESTROY:
      WgtDestroy(hwnd, pWidget);
      // we _MUST_ pass this on, or the default widget proc
      // cannot clean up.
      mrc = pWidget->pfnwpDefWidgetProc(hwnd, msg, mp1, mp2);
      break;

    default:
      mrc = pWidget->pfnwpDefWidgetProc(hwnd, msg, mp1, mp2);
  } // end switch(msg)

  return(mrc);
}

/* ******************************************************************
 *
 *   Exported procedures
 *
 ********************************************************************/

/*
 *@@ WgtInitModule:
 *      required export with ordinal 1, which must tell
 *      the XCenter how many widgets this DLL provides,
 *      and give the XCenter an array of XCENTERWIDGETCLASS
 *      structures describing the widgets.
 *
 *      With this call, you are given the module handle of
 *      XFLDR.DLL. For convenience, you may resolve imports
 *      for some useful functions which are exported thru
 *      src\shared\xwp.def. See the code below.
 *
 *      This function must also register the PM window classes
 *      which are specified in the XCENTERWIDGETCLASS array
 *      entries. For this, you are given a HAB which you
 *      should pass to WinRegisterClass. For the window
 *      class style (4th param to WinRegisterClass),
 *      you should specify
 *
 +          CS_PARENTCLIP | CS_SIZEREDRAW | CS_SYNCPAINT
 *
 *      Your widget window _will_ be resized, even if you're
 *      not planning it to be.
 *
 *      This function only gets called _once_ when the widget
 *      DLL has been successfully loaded by the XCenter. If
 *      there are several instances of a widget running (in
 *      the same or in several XCenters), this function does
 *      not get called again. However, since the XCenter unloads
 *      the widget DLLs again if they are no longer referenced
 *      by any XCenter, this might get called again when the
 *      DLL is re-loaded.
 *
 *      There will ever be only one load occurence of the DLL.
 *      The XCenter manages sharing the DLL between several
 *      XCenters. As a result, it doesn't matter if the DLL
 *      has INITINSTANCE etc. set or not.
 *
 *      If this returns 0, this is considered an error, and the
 *      DLL will be unloaded again immediately.
 *
 *      If this returns any value > 0, *ppaClasses must be
 *      set to a static array (best placed in the DLL's
 *      global data) of XCENTERWIDGETCLASS structures,
 *      which must have as many entries as the return value.
 */

ULONG EXPENTRY WgtInitModule(HAB hab,         // XCenter's anchor block
                             HMODULE hmodPlugin, // module handle of the widget DLL
                             HMODULE hmodXFLDR,    // XFLDR.DLL module handle
                             PXCENTERWIDGETCLASS *ppaClasses,
                             PSZ pszErrorMsg)  // if 0 is returned, 500 bytes of error msg
{
  ULONG   ulrc = 0,
  ul = 0;
  BOOL    fImportsFailed = FALSE;
  g_hResDLL = hmodPlugin;
  // resolve imports from XFLDR.DLL (this is basically
  // a copy of the doshResolveImports code, but we can't
  // use that before resolving...)
  for (ul = 0;
      ul < sizeof(G_aImports) / sizeof(G_aImports[0]);
      ul++)
  {
    if (DosQueryProcAddr(hmodXFLDR,
                         0,               // ordinal, ignored
                         (PSZ)G_aImports[ul].pcszFunctionName,
                         G_aImports[ul].ppFuncAddress)
        != NO_ERROR)
    {
      sprintf(pszErrorMsg,
              "Import %s failed.",
              G_aImports[ul].pcszFunctionName);
      fImportsFailed = TRUE;
      break;
    }
  }

  if (!fImportsFailed)
  {
    // all imports OK:
    // register our PM window class
    if (!WinRegisterClass(hab,
                          WNDCLASS_WIDGET_RADIO,
                          fnwpSampleWidget,
                          CS_PARENTCLIP | CS_SIZEREDRAW | CS_SYNCPAINT,
                          sizeof(PRADIOPRIVATE))
        // extra memory to reserve for QWL_USER
       )
      strcpy(pszErrorMsg, "WinRegisterClass failed.");
    else
    {
      // no error:
      // return widget classes
      *ppaClasses = G_WidgetClasses;

      // return no. of classes in this DLL (one here):
      ulrc = sizeof(G_WidgetClasses) / sizeof(G_WidgetClasses[0]);
    }
  }

  return(ulrc);
}

/*
 *@@ WgtUnInitModule:
 *      optional export with ordinal 2, which can clean
 *      up global widget class data.
 *
 *      This gets called by the XCenter right before
 *      a widget DLL gets unloaded. Note that this
 *      gets called even if the "init module" export
 *      returned 0 (meaning an error) and the DLL
 *      gets unloaded right away.
 */

VOID EXPENTRY WgtUnInitModule(VOID)
{
}

/*
 *@@ WgtQueryVersion:
 *      this new export with ordinal 3 can return the
 *      XWorkplace version number which is required
 *      for this widget to run. For example, if this
 *      returns 0.9.10, this widget will not run on
 *      earlier XWorkplace versions.
 *
 *      NOTE: This export was mainly added because the
 *      prototype for the "Init" export was changed
 *      with V0.9.9. If this returns 0.9.9, it is
 *      assumed that the INIT export understands
 *      the new FNWGTINITMODULE_099 format (see center.h).
 *
 *@@added V0.9.9 (2001-02-06) [umoeller]
 */

VOID EXPENTRY WgtQueryVersion(PULONG pulMajor,
                              PULONG pulMinor,
                              PULONG pulRevision)
{
  // report 0.9.9
  *pulMajor = 0;
  *pulMinor = 9;
  *pulRevision = 9;
}

