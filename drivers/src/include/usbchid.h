#ifndef  _usbchid_h_
   #define  _usbchid_h_
/* SCCSID = "src/dev/usb/INCLUDE/USBCHID.H, usb, c.basedd 98/07/10" */
/*
*   Licensed Material -- Property of IBM
*
*   (c) Copyright IBM Corp. 1997, 1998  All Rights Reserved
*/
/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  USBCHID.H                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB HID data definitions                              */
/*                                                                            */
/*   FUNCTION: USB HID Class & Client driver common data structure            */
/*             definitions.                                                   */
/*                                                                            */
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
/*          98/03/05  MB                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

   #include "usbcmmon.h"


// USB HID device class definitions and data structures

// value used to identify last index in next index items
   #define  LAST_INDEX                       0xffff

   #define  BITS_IN_BYTE                     8
   #define  BITS_DIVIDER                     3
   #define  FULL_WORD                        0xffff
   #define  FULL_BYTE                        0xff

// HID device class definitions and structures
   #define  HID_CLASS_DESCRIPTORS_HID        0x21
   #define  HID_CLASS_DESCRIPTORS_REPORT     0x22
   #define  HID_CLASS_DESCRIPTORS_PHYSICAL   0x23

// HID request values
   #define  HID_REQUEST_GET_REPORT           0x01
   #define  HID_REQUEST_GET_IDLE             0x02
   #define  HID_REQUEST_GET_PROTOCOL         0x03
   #define  HID_REQUEST_SET_REPORT           0x09
   #define  HID_REQUEST_SET_IDLE             0x0a
   #define  HID_REQUEST_SET_PROTOCOL         0x0b

// HID protocol type values used in get/set protocol requests
   #define  HID_PROTOCOL_TYPE_BOOT           0
   #define  HID_PROTOCOL_TYPE_REPORT         1

// HID interface protocol types
   #define  HID_INTF_PROTOCOL_KBD            1
   #define  HID_INTF_PROTOCOL_MOUSE          2

// HID device interface subclases
   #define  DEV_SUBCLASS_HIDBOOTSUBCLASS     0x01

// HID device report types used in set/get report requests
   #define  HID_REPORT_TYPE_INPUT            1
   #define  HID_REPORT_TYPE_OUTPUT           2
   #define  HID_REPORT_TYPE_FEATURE          3

// HID version flags
   #define  HID_PREDRAFT3_DEVICE             0x0001   // device follows HID predraft3 requirements

typedef struct _HID_desc_info_
{
   UCHAR    bDescriptorType;        // descriptor class type
   USHORT   wDescriptorLength;      // descriptor length
}  HIDDescInfo;

typedef struct _HID_device_descriptor_
{
   UCHAR       bLength;             // Size of descriptor in bytes
   UCHAR       bDescriptorType;     // 0x21 - HID DEVICE Descriptor type
   USHORT      bcdHID;              // HID Class Specification Release Number
   UCHAR       bCountryCode;        // localized hardware country code
   UCHAR       bNumDescriptors;     // no of Class descriptors
   HIDDescInfo descriptorList[1];   // descriptor list (always includes REPORT descriptor)
}  HIDDeviceDescriptor;




// Class & Client driver common definitions and data structures

typedef  struct   _item_features
{
   UCHAR       reportID;         // 09 report ID item belongs to
   ULONG       reportSize;       // 10 data size for this item
   ULONG       reportCount;      // 14 element count for current item
   USHORT      usagePage;        // 18 item's usage page
   LONG        logMin;           // 20 logical minimum for this item
   LONG        logMax;           // 24 logical maximum for this item
   LONG        phyMin;           // 28 physical value minimum
   LONG        phyMax;           // 32 physical value maximum
   ULONG       unit;             // 36 units of measurement
   UCHAR       unitExponent;     // 40 exponent value
                                 // 41
}  ItemFeatures;

typedef  struct   _local_features
{
   // usage information
   USHORT         usagePage;        // 41 local (only for this item) usage page
   USHORT         usageMin;         // 43 usage minimum
   USHORT         usageMax;         // 45 usage maximum
   USHORT         indexToUsageList; // 47
   // physical data references
   USHORT         designatorMin;    // 49
   USHORT         designatorMax;    // 51
   USHORT         indexToDesignator;// 53
   // string data references
   UCHAR          stringMin;        // 55
   UCHAR          stringMax;        // 56
   USHORT         indexToStrings;   // 57
                                    // 59
}  LocalFeatures;

typedef struct _RepItemData
{
   UCHAR          used;             // 00 nonzero if allocated
   UCHAR          interface;        // 01 interface index
   UCHAR          mainType;         // 02 item type - input, output, feature, collection
   USHORT         itemFlags;        // 03 item flags
   USHORT         parColIndex;      // 05 parent collection index (LAST_INDEX - no parent collection)
   USHORT         indexToNextItem;  // 07 index to next main item for this report
   // item features
   ItemFeatures   itemFeatures;     // 09

   // item local data
   LocalFeatures  localFeatures;    // 41
                                    // 59
}  ReportItemData;

typedef struct _item_usage
{
   UCHAR          used;             // nonzero if allocated
   USHORT         indexToNextUsageData;
   USHORT         usagePage;        // local (only for this item) usage page
   USHORT         usageMin;         // usage minimum
   USHORT         usageMax;         // usage maximum
}  ItemUsage;

typedef struct _item_designator
{
   UCHAR          used;             // nonzero if allocated
   USHORT         indexToNextDesignatorData;
   USHORT         designatorMin;         // designator minimum
   USHORT         designatorMax;         // designator maximum
}  ItemDesignator;

typedef struct _item_strings
{
   UCHAR          used;             // nonzero if allocated
   USHORT         indexToNextStringData;
   UCHAR          stringMin;         // string minimum
   UCHAR          stringMax;         // string maximum
}  ItemString;

// report item types
   #define  HID_REPORT_ITYPE_MAIN         0
   #define  HID_REPORT_ITYPE_GLOBAL       1
   #define  HID_REPORT_ITYPE_LOCAL        2

// main item tags
   #define  HID_REPORT_TAGS_MAIN_INPUT    0x08
   #define  HID_REPORT_TAGS_MAIN_OUTPUT   0x09
   #define  HID_REPORT_TAGS_MAIN_FEATURE  0x0b
   #define  HID_REPORT_TAGS_MAIN_COLL     0x0a
   #define  HID_REPORT_TAGS_MAIN_ENDCOLL  0x0c

// global item tags
   #define  HID_REPORT_TAGS_GLOBAL_UPAGE  0x00
   #define  HID_REPORT_TAGS_GLOBAL_LMIN   0x01
   #define  HID_REPORT_TAGS_GLOBAL_LMAX   0x02
   #define  HID_REPORT_TAGS_GLOBAL_PMIN   0x03
   #define  HID_REPORT_TAGS_GLOBAL_PMAX   0x04
   #define  HID_REPORT_TAGS_GLOBAL_UEXP   0x05
   #define  HID_REPORT_TAGS_GLOBAL_UNIT   0x06
   #define  HID_REPORT_TAGS_GLOBAL_RSIZE  0x07
   #define  HID_REPORT_TAGS_GLOBAL_RID    0x08
   #define  HID_REPORT_TAGS_GLOBAL_RCOUNT 0x09
   #define  HID_REPORT_TAGS_GLOBAL_PUSH   0x0a
   #define  HID_REPORT_TAGS_GLOBAL_POP    0x0b

// local item tags
   #define  HID_REPORT_TAGS_LOCAL_USAGE   0x00
   #define  HID_REPORT_TAGS_LOCAL_UMIN    0x01
   #define  HID_REPORT_TAGS_LOCAL_UMAX    0x02
   #define  HID_REPORT_TAGS_LOCAL_DINDEX  0x03
   #define  HID_REPORT_TAGS_LOCAL_DMIN    0x04
   #define  HID_REPORT_TAGS_LOCAL_DMAX    0x05
   #define  HID_REPORT_TAGS_LOCAL_SINDEX  0x07
   #define  HID_REPORT_TAGS_LOCAL_SMIN    0x08
   #define  HID_REPORT_TAGS_LOCAL_SMAX    0x09
   #define  HID_REPORT_TAGS_LOCAL_DELIM   0x0a

// item type flags
   #define  HID_REPORT_ITEM_CONSTANT      0x0001   // 0 - data
   #define  HID_REPORT_ITEM_VARIABLE      0x0002   // 0 - Array
   #define  HID_REPORT_ITEM_RELATIVE      0x0004   // 0 - Absolute
   #define  HID_REPORT_ITEM_WRAP          0x0008   // 0 - no wrap
   #define  HID_REPORT_ITEM_NONLINEAR     0x0010   // 0 - linear
   #define  HID_REPORT_ITEM_NOPREFFERED   0x0020   // 0 - preferred state
   #define  HID_REPORT_ITEM_NULLSTATE     0x0040   // 0 - no null position
   #define  HID_REPORT_ITEM_VOLATILE      0x0080   // 0 - non volatile
   #define  HID_REPORT_ITEM_BUFFBYTES     0x0100   // 0 - bit field

// main item types
   #define  HID_REPORT_MAIN_INPUT         1
   #define  HID_REPORT_MAIN_OUTPUT        2
   #define  HID_REPORT_MAIN_FEATURE       3
   #define  HID_REPORT_MAIN_COLLECTION    4

// HID Usage Pages
   #define  HID_USAGE_PAGE_GDESKTOP       0x01  // generic desktop controls
   #define  HID_USAGE_PAGE_SIMCONTRL      0x02  // Simulation Controls
   #define  HID_USAGE_PAGE_VRCNTRLS       0x03  // VR Controls
   #define  HID_USAGE_PAGE_SPORTCNTRLS    0x04  // Sport Controls
   #define  HID_USAGE_PAGE_GAMECNTRLS     0x05  // Game Controls
   #define  HID_USAGE_PAGE_KEYBOARD       0x07  // Keyboard/Keypad
   #define  HID_USAGE_PAGE_LEDS           0x08  // LEDs
   #define  HID_USAGE_PAGE_BUTTON         0x09  // button
   #define  HID_USAGE_PAGE_ORDINAL        0x0a  // ordinal
   #define  HID_USAGE_PAGE_TELEPHONY      0x0b  // telephony
   #define  HID_USAGE_PAGE_CONSUMER       0x0c  // consumer
   #define  HID_USAGE_PAGE_DIGITIZER      0x0d  // digitizer
   // Reserved 0xe
   #define  HID_USAGE_PAGE_PID            0x0f  // Physical Interface Device (ForceFeedback)
   #define  HID_USAGE_PAGE_UNICODE        0x10  // unicode
   #define  HID_USAGE_PAGE_ALPHANUMDISP   0x14  // Alphanumerical Display
   // Reserved  0x15 - 0x79
   #define  HID_USAGE_PAGE_USBMONITOR       0x80  // USB Monitor
   #define  HID_USAGE_PAGE_USBENUMVALUES    0x81  // USB Enumerated Values
   #define  HID_USAGE_PAGE_VESAVIRTCONTROLS 0x82  // VESA Virtual Controls
   // Reserved 0x83
   #define  HID_USAGE_PAGE_POWERDEVICE    0x84  // Power Device (USV)
   #define  HID_USAGE_PAGE_BATTERYSYSTEM  0x85  // Battery System
   // Reserved 0x86 - 0x87
   #define  HID_USAGE_PAGE_BARCODESCANNER 0x88  // Barcode scanner
   // Reserved 0x89 - 0x8B
   #define  HID_USAGE_PAGE_POS1           0x8C  // Point of Sales pages 1-4
   #define  HID_USAGE_PAGE_POS2           0x8D  //
   #define  HID_USAGE_PAGE_POS3           0x8E  //
   #define  HID_USAGE_PAGE_POS4           0x8F  //

   #define  HID_USAGE_PAGE_CAMERACONTROL  0x90  // Camera Control
   #define  HID_USAGE_PAGE_ARCADE         0x91  // Arcade
   // Reserved 0x92 - 0xFEFF
   // 0xFF00 - 0xFFFF are Vendor defined

// generic desktop page usage IDs
   #define  HID_GDESKTOP_USAGE_POINTER    0x01  // pointer
   #define  HID_GDESKTOP_USAGE_MOUSE      0x02  // mouse
   #define  HID_GDESKTOP_USAGE_JOYSTICK   0x04  // joystick
   #define  HID_GDESKTOP_USAGE_GAMEPAD    0x05  // game pad
   #define  HID_GDESKTOP_USAGE_KEYBOARD   0x06  // keyboard
   #define  HID_GDESKTOP_USAGE_KEYPAD     0x07  // keypad
   #define  HID_GDESKTOP_USAGE_MULTIAXIS  0x08  // Multi-axis Controller
   // Reserved 0x09 - 0x2F
   #define  HID_GDESKTOP_USAGE_X          0x30  // X
   #define  HID_GDESKTOP_USAGE_Y          0x31  // Y
   #define  HID_GDESKTOP_USAGE_Z          0x32  // Z
   #define  HID_GDESKTOP_USAGE_RX         0x33  // Rx
   #define  HID_GDESKTOP_USAGE_RY         0x34  // RY
   #define  HID_GDESKTOP_USAGE_RZ         0x35  // Rz
   #define  HID_GDESKTOP_USAGE_SLIDER     0x36  // Slider
   #define  HID_GDESKTOP_USAGE_DIAL       0x37  // Dial
   #define  HID_GDESKTOP_USAGE_WHEEL      0x38  // Wheel
   #define  HID_GDESKTOP_USAGE_HATSWITCH  0x39  // Hat switch
   #define  HID_GDESKTOP_USAGE_CBUFFER    0x3A  // Counted Buffer
   #define  HID_GDESKTOP_USAGE_BYTECOUNT  0x3B  // Byte Count
   #define  HID_GDESKTOP_USAGE_MOTWAKEUP  0x3C  // Motion Wakeup
   #define  HID_GDESKTOP_USAGE_START      0x3D  // Start
   #define  HID_GDESKTOP_USAGE_SELECT     0x3E  // Select
   // Reserved 0x3F
   #define  HID_GDESKTOP_USAGE_VX         0x40  // Vx
   #define  HID_GDESKTOP_USAGE_VY         0x41  // Vy
   #define  HID_GDESKTOP_USAGE_VZ         0x42  // Vz
   #define  HID_GDESKTOP_USAGE_VBRX       0x43  // Vbrx
   #define  HID_GDESKTOP_USAGE_VBRY       0x44  // Vbry
   #define  HID_GDESKTOP_USAGE_VBRZ       0x45  // Vbrz
   #define  HID_GDESKTOP_USAGE_VNO        0x46  // Vno
   // Reserved 0x47 - 0x7F
   #define  HID_GDESKTOP_USAGE_SYS_CONTROL    0x80  // System Control
   #define  HID_GDESKTOP_USAGE_SYS_PDOWN      0x81  // System Power Down
   #define  HID_GDESKTOP_USAGE_SYS_SLEEP      0x82  // System Sleep
   #define  HID_GDESKTOP_USAGE_SYS_WAKEUP     0x83  // System Wake Up
   #define  HID_GDESKTOP_USAGE_SYS_CTXMENU    0x84  // System Context Menu
   #define  HID_GDESKTOP_USAGE_SYS_MAINMENU   0x85  // System Main Menu
   #define  HID_GDESKTOP_USAGE_SYS_APPMENU    0x86  // System App menu
   #define  HID_GDESKTOP_USAGE_SYS_MENUHELP   0x87  // System Menu Help
   #define  HID_GDESKTOP_USAGE_SYS_MENUEXIT   0x88  // System Menu Exit
   #define  HID_GDESKTOP_USAGE_SYS_MENUSLCT   0x89  // System Menu Select
   #define  HID_GDESKTOP_USAGE_SYS_MENURIGT   0x8A  // System Menu Right
   #define  HID_GDESKTOP_USAGE_SYS_MENULEFT   0x8B  // System Menu Left
   #define  HID_GDESKTOP_USAGE_SYS_MENUUP     0x8C  // System Menu Up
   #define  HID_GDESKTOP_USAGE_SYS_MENUDOWN   0x8D  // System Menu Down
   // Reserved 0x8E - 0x8F
   #define  HID_GDESKTOP_USAGE_DPAD_UP        0x90  // Direction Pad Up
   #define  HID_GDESKTOP_USAGE_DPAD_DOWN      0x91  // Direction Pad Down
   #define  HID_GDESKTOP_USAGE_DPAD_RIGHT     0x92  // Direction Pad Right
   #define  HID_GDESKTOP_USAGE_DPAD_LEFT      0x93  // Direction Pad Left
   // Reserved 0x94 - 0xFFFF


// LEDs page usage IDs
   #define  HID_LEDS_USAGE_NUMLOCK        0x01  // num lock
   #define  HID_LEDS_USAGE_CAPSLOCK       0x02  // Caps lock
   #define  HID_LEDS_USAGE_SCROLLLOCK     0x04  // scroll lock


// USB Monitor Usage Page usage IDs
   #define HID_USBMONITOR_USAGE_CONTROL  0x01
   #define HID_USBMONITOR_USAGE_EDIDINFO 0x02
   #define HID_USBMONITOR_USAGE_VDIFINFO 0x03
   #define HID_USBMONITOR_USAGE_VESAVERS 0x04

// data types used in HID Class driver and Client driver communications
typedef struct _USBHIDServe
{
   DeviceInfo FAR             *pDeviceInfo;     // far pointer to device data
   DeviceConfiguration FAR    *devConf;         // far pointer to device configuration data
   ReportItemData FAR         *itemData;        // ptr to report item data array
   ItemUsage FAR              *itemUsage;       // ptr to extra usage data array
   ItemDesignator FAR         *itemDesignator;  // ptr to extra designator data array
   ItemString FAR             *itemString;      // ptr to extra string data array
   USHORT                     reportItemIndex;  // starting report item index itemData
   USHORT                     versionFlags;     // specific version flags (HID drafts)
}     USBHIDServe;

typedef  struct _USBLegIO
{
   UINT  flags;                  // flags specifying request type
   PCHAR buffPtr;                // Pointer to data buffer
}     USBLegIO;


#endif

