/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  JOYTYPE.H                                             */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB joystick driver typedefs                          */
/*                                                                            */
/*   FUNCTION: This module is the USB Joystick driver TYPEDEF include file.   */
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
/*          00/01/04  MM                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

#define  MAX_INIT_MESSAGE_COUNT     2
/*
    Initialization time message IDs
*/
#define  INIT_MESSAGE_LOADED        0
#define  INIT_MESSAGE_NO_HID        1
#define  INIT_MESSAGE_INVNUMERIC    2
#define  INIT_MESSAGE_UNKNOWNKWD    3
#define  INIT_MESSAGE_NO_LEGACY     4

#define MAX_JOYS          16
#define MAX_BUFFER_LENGTH 32

#define LegJOYIDC 20

/*
    USB keyboard client IRQ switch values
*/
#define JOY_IRQ_STATUS_IDLESET  1
#define JOY_IRQ_STATUS_DURATION 2
#define JOY_IRQ_STATUS_INTPIPE  3
#define JOY_IRQ_STATUS_SETACK   4
#define JOY_IRQ_STATUS_STALLED  5

/*
   Joystick definitions
*/

#define MAX_BUTTONS 128
#define MAX_POVS    4
#define MAX_SLIDERS 2

typedef struct _JOYITEM
{
  UCHAR        bReport;
  USHORT       usOffset;     // In Bits
  USHORT       usReportSize; // In Bits
} JOYITEM, *PJOYITEM;

typedef struct _JOYAXEUNIT
{
   LONG        logMin;           // 20 logical minimum for this item
   LONG        logMax;           // 24 logical maximum for this item
   LONG        phyMin;           // 28 physical value minimum
   LONG        phyMax;           // 32 physical value maximum
   ULONG       unit;             // 36 units of measurement
   UCHAR       unitExponent;     // 40 exponent value
} JOYAXEUNIT, PJOYAXEUNIT;

typedef struct _JOYSTATE  // sames as Direct Input type DIJOYSTATE2;
{
  LONG lX;
  LONG lY;
  LONG lZ;
  LONG lRx;
  LONG lRy;
  LONG lRz;
  LONG rglSlider[MAX_SLIDERS];
  DWORD rgdwPOV[MAX_POVS];
  BYTE rgbButtons[MAX_BUTTONS];
  LONG    lVX;
  LONG    lVY;
  LONG    lVZ;
  LONG    lVRx;
  LONG    lVRy;
  LONG    lVRz;
  LONG    rglVSlider[2];
  LONG    lAX;
  LONG    lAY;
  LONG    lAZ;
  LONG    lARx;
  LONG    lARy;
  LONG    lARz;
  LONG    rglASlider[2];
  LONG    lFX;
  LONG    lFY;
  LONG    lFZ;
  LONG    lFRx;
  LONG    lFRy;
  LONG    lFRz;
  LONG    rglFSlider[2];
} JOYSTATE, *PJOYSTATE;

typedef struct DEVCAPS
{
  ULONG ulSize;
  ULONG ulFlags;
  ULONG ulDevType;
  ULONG ulAxes;
  ULONG ulButtons;
  ULONG ulSliders;
  ULONG ulPOVs;
  ULONG ulFFSamplePeriod;
  ULONG ulFFMinTimeResolution;
  ULONG ulFirmwareRevision;
  ULONG ulHardwareRevision;
  ULONG ulFFDriverVersion;
} DEVCAPS, *PDEVCAPS;

typedef struct DZSATAXE
{
  LONG lDeadZoneMin;
  LONG lDeadZoneMax;
  LONG lSaturationMin;
  LONG lSaturationMax;
} DZSATAXE;

#define DZ_VALID_X    0x00000001
#define SAT_VALID_X   0x00000002
#define DZ_VALID_Y    0x00000004
#define SAT_VALID_Y   0x00000008
#define DZ_VALID_Z    0x00000010
#define SAT_VALID_Z   0x00000020
#define DZ_VALID_RX   0x00000100
#define SAT_VALID_RX  0x00000200
#define DZ_VALID_RY   0x00000400
#define SAT_VALID_RY  0x00000800
#define DZ_VALID_RZ   0x00001000
#define SAT_VALID_RZ  0x00002000
#define DZ_VALID_SL1  0x0001
#define SAT_VALID_SL1 0x0002
#define DZ_VALID_SL2  0x0004
#define SAT_VALID_SL2 0x0008

typedef struct DZSATCAPS
{
  ULONG ulSize;
  ULONG ulFlags;
  DZSATAXE dzsX;
  DZSATAXE dzsY;
  DZSATAXE dzsZ;
  DZSATAXE dzsRX;
  DZSATAXE dzsRY;
  DZSATAXE dzsRZ;
  DZSATAXE dzsLiders[MAX_SLIDERS];
} DZSATCAPS, *PDZSATCAPS, FAR *PFDZSATCAPS;

#define JOYHAS_X   0x00000001
#define JOYHAS_Y   0x00000002
#define JOYHAS_Z   0x00000004
#define JOYHAS_VX  0x00000010
#define JOYHAS_VY  0x00000020
#define JOYHAS_VZ  0x00000040
#define JOYHAS_AX  0x00000100
#define JOYHAS_AY  0x00000200
#define JOYHAS_AZ  0x00000400
#define JOYHAS_FX  0x00001000
#define JOYHAS_FY  0x00002000
#define JOYHAS_FZ  0x00004000
#define JOYHAS_RX  0x00010000
#define JOYHAS_RY  0x00020000
#define JOYHAS_RZ  0x00040000
#define JOYHAS_VRX 0x00100000
#define JOYHAS_VRY 0x00200000
#define JOYHAS_VRZ 0x00400000
#define JOYHAS_ARX 0x01000000
#define JOYHAS_ARY 0x02000000
#define JOYHAS_ARZ 0x04000000
#define JOYHAS_FRX 0x10000000
#define JOYHAS_FRY 0x20000000
#define JOYHAS_FRZ 0x40000000
#define JOYHAS_VS0 0x00000001
#define JOYHAS_AS0 0x00000002
#define JOYHAS_FS0 0x00000003
#define JOYHAS_VS1 0x00000010
#define JOYHAS_AS1 0x00000020
#define JOYHAS_FS1 0x00000030

#define JOYOFS_X           0
#define JOYOFS_Y           1
#define JOYOFS_Z           2
#define JOYOFS_RX          3
#define JOYOFS_RY          4
#define JOYOFS_RZ          5
#define JOYOFS_SLIDER0     6
#define JOYOFS_SLIDER1     7
#define JOYOFS_POV0        8
#define JOYOFS_POV1        9
#define JOYOFS_POV2       10
#define JOYOFS_POV3       11
  #define JOYMAX_AXES JOYOFS_POV3 +1
#define JOYOFS_BUTTON0    12
#define JOYOFS_BUTTON1    13
#define JOYOFS_BUTTON2    14
#define JOYOFS_BUTTON3    15
#define JOYOFS_BUTTON4    16
#define JOYOFS_BUTTON5    17
#define JOYOFS_BUTTON6    18
#define JOYOFS_BUTTON7    19
#define JOYOFS_BUTTON8    20
#define JOYOFS_BUTTON9    21
#define JOYOFS_BUTTON10   22
#define JOYOFS_BUTTON11   23
#define JOYOFS_BUTTON12   24
#define JOYOFS_BUTTON13   25
#define JOYOFS_BUTTON14   26
#define JOYOFS_BUTTON15   27
#define JOYOFS_BUTTON16   28
#define JOYOFS_BUTTON17   29
#define JOYOFS_BUTTON18   30
#define JOYOFS_BUTTON19   31
#define JOYOFS_BUTTON20   32
#define JOYOFS_BUTTON21   33
#define JOYOFS_BUTTON22   34
#define JOYOFS_BUTTON23   35
#define JOYOFS_BUTTON24   36
#define JOYOFS_BUTTON25   37
#define JOYOFS_BUTTON26   38
#define JOYOFS_BUTTON27   39
#define JOYOFS_BUTTON28   40
#define JOYOFS_BUTTON29   41
#define JOYOFS_BUTTON30   42
#define JOYOFS_BUTTON31   43
#define JOYOFS_BUTTON32   44
#define JOYOFS_BUTTON33   45
#define JOYOFS_BUTTON34   46
#define JOYOFS_BUTTON35   47
#define JOYOFS_BUTTON36   48
#define JOYOFS_BUTTON37   49
#define JOYOFS_BUTTON38   50
#define JOYOFS_BUTTON39   51
#define JOYOFS_BUTTON40   52
#define JOYOFS_BUTTON41   53
#define JOYOFS_BUTTON42   54
#define JOYOFS_BUTTON43   55
#define JOYOFS_BUTTON44   56
#define JOYOFS_BUTTON45   57
#define JOYOFS_BUTTON46   58
#define JOYOFS_BUTTON47   59
#define JOYOFS_BUTTON48   60
#define JOYOFS_BUTTON49   61
#define JOYOFS_BUTTON50   62
#define JOYOFS_BUTTON51   63
#define JOYOFS_BUTTON52   64
#define JOYOFS_BUTTON53   65
#define JOYOFS_BUTTON54   66
#define JOYOFS_BUTTON55   67
#define JOYOFS_BUTTON56   68
#define JOYOFS_BUTTON57   69
#define JOYOFS_BUTTON58   70
#define JOYOFS_BUTTON59   71
#define JOYOFS_BUTTON60   72
#define JOYOFS_BUTTON61   73
#define JOYOFS_BUTTON62   74
#define JOYOFS_BUTTON63   75
#define JOYOFS_BUTTON64   76
#define JOYOFS_BUTTON65   77
#define JOYOFS_BUTTON66   78
#define JOYOFS_BUTTON67   79
#define JOYOFS_BUTTON68   80
#define JOYOFS_BUTTON69   81
#define JOYOFS_BUTTON70   82
#define JOYOFS_BUTTON71   83
#define JOYOFS_BUTTON72   84
#define JOYOFS_BUTTON73   85
#define JOYOFS_BUTTON74   86
#define JOYOFS_BUTTON75   87
#define JOYOFS_BUTTON76   88
#define JOYOFS_BUTTON77   89
#define JOYOFS_BUTTON78   90
#define JOYOFS_BUTTON79   91
#define JOYOFS_BUTTON80   92
#define JOYOFS_BUTTON81   93
#define JOYOFS_BUTTON82   94
#define JOYOFS_BUTTON83   95
#define JOYOFS_BUTTON84   96
#define JOYOFS_BUTTON85   97
#define JOYOFS_BUTTON86   98
#define JOYOFS_BUTTON87   99
#define JOYOFS_BUTTON88  100
#define JOYOFS_BUTTON89  101
#define JOYOFS_BUTTON90  102
#define JOYOFS_BUTTON91  103
#define JOYOFS_BUTTON92  104
#define JOYOFS_BUTTON93  105
#define JOYOFS_BUTTON94  106
#define JOYOFS_BUTTON95  107
#define JOYOFS_BUTTON96  108
#define JOYOFS_BUTTON97  109
#define JOYOFS_BUTTON98  110
#define JOYOFS_BUTTON99  111
#define JOYOFS_BUTTON100 112
#define JOYOFS_BUTTON101 113
#define JOYOFS_BUTTON102 114
#define JOYOFS_BUTTON103 115
#define JOYOFS_BUTTON104 116
#define JOYOFS_BUTTON105 117
#define JOYOFS_BUTTON106 118
#define JOYOFS_BUTTON107 119
#define JOYOFS_BUTTON108 120
#define JOYOFS_BUTTON109 121
#define JOYOFS_BUTTON110 122
#define JOYOFS_BUTTON111 123
#define JOYOFS_BUTTON112 124
#define JOYOFS_BUTTON113 125
#define JOYOFS_BUTTON114 126
#define JOYOFS_BUTTON115 127
#define JOYOFS_BUTTON116 128
#define JOYOFS_BUTTON117 129
#define JOYOFS_BUTTON118 130
#define JOYOFS_BUTTON119 131
#define JOYOFS_BUTTON120 132
#define JOYOFS_BUTTON121 133
#define JOYOFS_BUTTON122 134
#define JOYOFS_BUTTON123 135
#define JOYOFS_BUTTON124 136
#define JOYOFS_BUTTON125 137
#define JOYOFS_BUTTON126 138
#define JOYOFS_BUTTON127 139
#define JOYOFS_VX        140
#define JOYOFS_VY        141
#define JOYOFS_VZ        142
#define JOYOFS_AX        143
#define JOYOFS_AY        144
#define JOYOFS_AZ        145
#define JOYOFS_FX        146
#define JOYOFS_FY        147
#define JOYOFS_FZ        148
#define JOYOFS_VRX       149
#define JOYOFS_VRY       150
#define JOYOFS_VRZ       151
#define JOYOFS_ARX       152
#define JOYOFS_ARY       153
#define JOYOFS_ARZ       154
#define JOYOFS_FRX       155
#define JOYOFS_FRY       156
#define JOYOFS_FRZ       157
#define JOYOFS_VS0       158
#define JOYOFS_AS0       159
#define JOYOFS_FS0       160
#define JOYOFS_VS1       161
#define JOYOFS_AS1       162
#define JOYOFS_FS1       163
#define JOYMAXITEMS JOYOFS_FS1+1


typedef struct _JOYList
{
   UCHAR       active;
   UCHAR       joyAddr;
   UCHAR       controllerID;
   SetupPacket setITpack;
   UCHAR       interruptPipeAddress;
   UCHAR       inInterface;
   USHORT      ReportLength;
   ULONG       ulCapsAxes;
   ULONG       ulCapsSliders;
   DEVCAPS     DevCapsJoy;
   DeviceDescriptor *pDevDesc;
   JOYITEM     Items[JOYMAXITEMS];
   JOYAXEUNIT  AxeUnits[JOYMAX_AXES];
   JOYSTATE    joyState;
   UCHAR       prevBuff[MAX_BUFFER_LENGTH];
   UCHAR       buffer[MAX_BUFFER_LENGTH];
}     JOYList;

typedef USHORT (FAR *PJOYDDIDC) (USHORT Func, USHORT Var1, USHORT Var2);

