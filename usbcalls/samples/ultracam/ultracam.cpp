/* -*- linux-c -*-
 *
 * USB NB (Ultraport) Camera driver
 *
 * Original source by Dmitri
 * Updated by Karl Gutwin <karl@gutwin.org>
 * Updated by Jakob Lichtenberg <jl@itu.dk>
 * Thanks to axel grossklaus <axel@nomaden.org>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wrapper.h>
#include <linux/module.h>
#include <linux/init.h>
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#define INCL_DOS
#include <os2.h>
#include "..\..\usbcalls.h"
#include "usbvideo.h"


#define	ULTRACAM_VENDOR_ID	0x0461
#define	ULTRACAM_PRODUCT_ID	0x0813

#define MAX_CAMERAS		4	/* How many devices we allow to connect */

/*
 * This structure lives in uvd_t->user field.
 */
typedef struct
{
  int initialized;  /* Had we already sent init sequence? */
  int camera_model; /* What type of IBM camera we got? */
  int has_hdr;
  int bytes_in;         /* bytes remaining to read before drop */
  int invert;           /* shall we invert the picture? */

  int rbar, gbar, bbar; /* mean vals for rgb, used for auto wb */
  int projected_whitebal;
  int frames_since_adjust;
  int ybar;


} ultracam_t;
#define	ULTRACAM_T(uvd)	((ultracam_t *)((uvd)->user_data))

usbvideo_t *cams = NULL;

static int debug = 9;

static int ultracam_is_backward(uvd_t *uvd);
static void ultracam_set_white_balance(uvd_t *uvd);

static int flags = 0; /* FLAGS_DISPLAY_HINTS | FLAGS_OVERLAY_STATS; */

static const int min_canvasWidth  = 8;
static const int min_canvasHeight = 4;

static int whitebal = -1; /* auto whitebal */
static int wbmaxdelt = 1;

#define SHARPNESS_MIN	0
#define SHARPNESS_MAX	4
static int sharpness = 3; /* Low noise, good details */

/*
 * Here we define several initialization variables. They may
 * be used to automatically set color, hue, brightness and
 * contrast to desired values. This is particularly useful in
 * case of webcams (which have no controls and no on-screen
 * output) and also when a client V4L software is used that
 * does not have some of those controls. In any case it's
 * good to have startup values as options.
 *
 * These values are all in [0..255] range. This simplifies
 * operation. Note that actual values of V4L variables may
 * be scaled up (as much as << 8). User can see that only
 * on overlay output, however, or through a V4L client.
 */
static int init_brightness = 128;
static int init_contrast = 192;
static int init_sat = 10;
static int init_hue = 180;


#define MODULE_PARM(X,Y)
#define MODULE_PARM_DESC(X,Y)

static int brite = 0;
MODULE_PARM(brite, "i");
MODULE_PARM_DESC(brite, "y adjust: (-255)-(255) (default=0)");

static unsigned short hs_0b = 0x002e;
static unsigned short hs_0c = 0x00d6;
static unsigned short hs_0d = 0x00fc;
static unsigned short hs_0e = 0x00f1;
static unsigned short hs_0f = 0x00da;
static unsigned short hs_10 = 0x0036;

MODULE_PARM(hs_0b, "h");
MODULE_PARM(hs_0c, "h");
MODULE_PARM(hs_0d, "h");
MODULE_PARM(hs_0e, "h");
MODULE_PARM(hs_0f, "h");
MODULE_PARM(hs_10, "h");

static unsigned short wb_51 = 0xff00;
static unsigned short wb_52 = 0xff00;
static unsigned short wb_53 = 0xff00;
static unsigned short wb_11 = 0xff00;
static unsigned short wb_12 = 0xff00;
static unsigned short wb_13 = 0xff00;

MODULE_PARM(wb_51, "h");
MODULE_PARM(wb_52, "h");
MODULE_PARM(wb_53, "h");
MODULE_PARM(wb_11, "h");
MODULE_PARM(wb_12, "h");
MODULE_PARM(wb_13, "h");

static int yuv = 0;
MODULE_PARM(yuv, "i");

MODULE_PARM(debug, "i");
MODULE_PARM_DESC(debug, "Debug level: 0-9 (default=0)");
MODULE_PARM(flags, "i");
MODULE_PARM_DESC(flags,
                 "Bitfield: 0=VIDIOCSYNC, "
                 "1=B/W, "
                 "2=show hints, "
                 "3=show stats, "
                 "4=test pattern, "
                 "5=separate frames, "
                 "6=clean frames");
MODULE_PARM(whitebal, "i");
MODULE_PARM_DESC(whitebal, "White balance: 0-48 (0:bluish, 48:reddish) or -1: automatic(default)");
MODULE_PARM(wbmaxdelt, "i");
MODULE_PARM_DESC(wbmaxdelt, "White balance maximum delta: 1-48 (1:slow, 48:fast) (default=1)");
MODULE_PARM(sharpness, "i");
MODULE_PARM_DESC(sharpness, "Sharpness: 0=smooth, 4=sharp (default=3)");

MODULE_PARM(init_brightness, "i");
MODULE_PARM_DESC(init_brightness, "Brightness preconfiguration: 0-255 (default=128) or -1: automatic");
MODULE_PARM(init_contrast, "i");
MODULE_PARM_DESC(init_contrast, "Contrast preconfiguration: 0-255 (default=192)");
MODULE_PARM(init_sat, "i");
MODULE_PARM_DESC(init_sat, "Saturation preconfiguration: 0-20 (default=10)");
MODULE_PARM(init_hue, "i");
MODULE_PARM_DESC(init_hue, "Hue preconfiguration: 0-360 (default=180)");


#if 0



/*
 * ultracam_postProcess()
 *
 * Does various and sundried things to the image once it's been
 * read in.
 *
 */
static void ultracam_postProcess(uvd_t *uvd, usbvideo_frame_t *frame)
{
  ultracam_t *ucam;
  char tmp[6];
  int delt;

  if ((uvd == NULL) || (frame == NULL))
    return;

  if ((VIDEOSIZE_X(frame->request) == 0) || (VIDEOSIZE_Y(frame->request) == 0))
    return;

  ucam = ULTRACAM_T(uvd);

  ucam->rbar /= (VIDEOSIZE_X(frame->request)*VIDEOSIZE_Y(frame->request));
  ucam->gbar /= (VIDEOSIZE_X(frame->request)*VIDEOSIZE_Y(frame->request));
  ucam->bbar /= (VIDEOSIZE_X(frame->request)*VIDEOSIZE_Y(frame->request));
  ucam->ybar /= (VIDEOSIZE_X(frame->request)*VIDEOSIZE_Y(frame->request));

  ucam->frames_since_adjust++;

  if (ucam->frames_since_adjust > 10)
  { /* module param! */
    ucam->frames_since_adjust = 0;

    if (whitebal < 0)
    {
      /* Auto white balance:
       * I noticed that on one end of the white balance scale, 
       * everything looks very blue (low, 0 end). On the other
       * end, everything looks very red (high, 48 end). Therefore,
       * this simple algorithm takes the difference between the
       * average red and blue values, divides that by two,
       * and changes the white balance by that much.
       *
       * delt is the estimated distance between where we are
       * and where we need to be, white-balancy speaking. Since
       * always jumping by size delt can cause radical color
       * shifts (and because I'm a patient guy) we restrict the
       * upper range of delt to wbmaxdelt, a module parameter.
       * For quicker responses, increase the value of wbmaxdelt.
       * 
       * -Karl
       * 
       * PS. I know next to nothing about color or imaging... if
       * someone out there knows a better solution than this, let
       * me know! Thanks...
       */

      delt = ((ucam->bbar - ucam->rbar)>>1);

      if (delt > wbmaxdelt)
        delt = wbmaxdelt;
      else if ((delt < 0) && (-delt > wbmaxdelt))
        delt = -wbmaxdelt;

      ucam->projected_whitebal += delt;
      RESTRICT_TO_RANGE(ucam->projected_whitebal, 0, 48);

      if (debug > 0)
        printf("Autoadjusting whitebal to %d",ucam->projected_whitebal);
      ultracam_set_white_balance(uvd);
    }

  }

  /* Optionally display statistics on the screen */
  if (uvd->flags & FLAGS_OVERLAY_STATS)
  {
    usbvideo_OverlayStats(uvd, frame);

    sprintf(tmp, "wb%3d", ucam->projected_whitebal);
    usbvideo_OverlayString(uvd, frame, (VIDEOSIZE_X(frame->request) - 30), 100, tmp);

    sprintf(tmp, "r %3d", ucam->rbar);
    usbvideo_OverlayString(uvd, frame, (VIDEOSIZE_X(frame->request) - 30), 116, tmp);
    sprintf(tmp, "g %3d", ucam->gbar);
    usbvideo_OverlayString(uvd, frame, (VIDEOSIZE_X(frame->request) - 30), 124, tmp);
    sprintf(tmp, "b %3d", ucam->bbar);
    usbvideo_OverlayString(uvd, frame, (VIDEOSIZE_X(frame->request) - 30), 132, tmp);

    sprintf(tmp, "y %3d", ucam->ybar);
    usbvideo_OverlayString(uvd, frame, (VIDEOSIZE_X(frame->request) - 30), 148, tmp);
  }


}





/*
 * ultracam_find_header()
 *
 * Locate one of supported header markers in the queue.
 * Once found, remove all preceding bytes AND the marker (8 bytes)
 * from the data pump queue. Whatever follows must be video lines.
 *
 * History:
 * 5/16/01  Created.
 */
static ParseState_t ultracam_find_header(uvd_t *uvd, usbvideo_frame_t *frame)
{
  ultracam_t *ucam;

  if ((uvd->curframe) < 0 || (uvd->curframe >= USBVIDEO_NUMFRAMES))
  {
    err("ibmcam_find_header: Illegal frame %d.", uvd->curframe);
    return scan_EndParse;
  }

  ucam = ULTRACAM_T(uvd);
  assert(ucam != NULL);
  ucam->has_hdr = 0;
  /* and start looking for header */

  while (RingQueue_GetLength(&uvd->dp) >= 8)
  {
    if ((RING_QUEUE_PEEK(&uvd->dp, 0) == 0x00) &&
        (RING_QUEUE_PEEK(&uvd->dp, 1) == 0x01) &&
        (RING_QUEUE_PEEK(&uvd->dp, 2) == 0x19) &&
        (RING_QUEUE_PEEK(&uvd->dp, 3) == 0x00) &&
        (RING_QUEUE_PEEK(&uvd->dp, 5) == 0x00) &&
        (RING_QUEUE_PEEK(&uvd->dp, 6) == 0x00) &&
        (RING_QUEUE_PEEK(&uvd->dp, 7) == 0x40))
    {
  #if 0				/* This code helps to detect new frame markers */
      printf("Header sig: 00 01 19 00 %02X", RING_QUEUE_PEEK(&uvd->dp, 4));
  #endif
      frame->header = RING_QUEUE_PEEK(&uvd->dp, 7);
      if ((frame->header == 0x40))
      {
  #if 0
        printf("Header found.");
  #endif
        RING_QUEUE_DEQUEUE_BYTES(&uvd->dp, 8);
        ucam->bytes_in = 7;
        ucam->has_hdr = 1;
        ucam->invert = ultracam_is_backward(uvd);
        ucam->rbar = ucam->gbar = ucam->bbar = 0;
        break;
      }
    }
    /* If we are still here then this doesn't look like a header */
    RING_QUEUE_DEQUEUE_BYTES(&uvd->dp, 1);
  }

  if (!ucam->has_hdr)
  {
    if (uvd->debug > 2)
      printf("Skipping frame, no header");
    return scan_EndParse;
  }

  /* Header found */
  ucam->has_hdr = 1;
  uvd->stats.header_count++;
  frame->scanstate = ScanState_Lines;
  frame->curline = 0;

  if (flags & FLAGS_FORCE_TESTPATTERN)
  {
    usbvideo_TestPattern(uvd, 1, 1);
    return scan_NextFrame;
  }
  return scan_Continue;
}

/*
 * For some unknown reason, the Ultraport camera sticks a fluke byte into
 * the bytestream every 640 bytes. This produces dotted lines on the
 * picture, and when left unadjusted, skewed frames.
 * This routine simply keeps track of how much we've dequeued so far,
 * and drops the 640th byte of the stream.
 * -Karl
 */
static int ultracam_Dequeue(uvd_t *uvd, unsigned char *buf, int req)
{
  ultracam_t *ucam;
  int left;
  ucam = ULTRACAM_T(uvd);

  left = (639 - ucam->bytes_in);
  if (req > left)
  {
    RingQueue_Dequeue(&uvd->dp, buf, left);
    RING_QUEUE_DEQUEUE_BYTES(&uvd->dp, 1);
    ucam->bytes_in = 0;
    /* mmmm... recursion... <drool> */
    ultracam_Dequeue(uvd, (buf + left), (req - left));
  }
  else
  {
    ucam->bytes_in += req;
    RingQueue_Dequeue(&uvd->dp, buf, req);
  } 

  return req;
}

/*
 * ultracam_parse_lines()
 *
 * Parse one line (interlaced) from the buffer, put
 * decoded RGB value into the current frame buffer
 * and add the written number of bytes (RGB) to
 * the *pcopylen.
 *
 * History:
 * 16-May-2001 Created.
 */

static ParseState_t ultracam_parse_lines(uvd_t *uvd, usbvideo_frame_t *frame)
{
  /*
   * Okay. Data seems to look like this:
   *
   * |----luma ----|-chroma -|
   * |-------------+---------|
   * |YYYYYYYY(320)|UUUU(160)|\
   * |YYYYYYYY(320)|VVVV(160)| 240 lines
   * |....     ... |  ...    |/
   * |_____________|_________|
   *
   * The chroma is overlapped over four pixels to fill out
   * the data.
   */

  #if 1
  unsigned char buf[(320*3) + 10];
  unsigned char *yl[2], *ul, *vl;
  int tX, tY;
  int i, j;
  unsigned char r, g, b, y, u, v;

  tX = 320;
  tY = 240;

  /* We're actually going to process two lines at once.
   * Each chroma byte has to be duplicated over four luma
   * bytes. However, in the process of getting the entire 
   * chroma set, we end up reading in -two- luma lines.
   * So... we process them all - it's quicker.
   * (on my system, this is faster than Windows! :)
   * -Karl
   */

  if (RingQueue_GetLength(&uvd->dp) < ((tX * 3)))
    return scan_Out;

  ultracam_Dequeue(uvd, buf, (tX * 3)); 
  /*  RingQueue_Dequeue(&uvd->dp, buf, (tX * 3)); */

  if ((frame->curline + 2) >= VIDEOSIZE_Y(frame->request))
    return scan_NextFrame;

  assert(frame->data != NULL);
  yl[0] = buf;
  vl    = (yl[0] + tX);
  yl[1] = (vl    + (tX >> 1));
  ul    = (yl[1] + tX);

  for (j = 0; j < 2; j++)
  {
    for (i = 0; i < VIDEOSIZE_X(frame->request); i++)
    {
      /* At some point, I would like to scale the video to fit
       * the requested window, rather than ignoring the size
       * request.
       */
      if (i < tX)
      {
        y = yl[j][i];
        u = ul[(i>>1)];
        v = vl[(i>>1)];

        /* invert around 128 (signed/unsigned conflict?) */
        y += ((y<128)?128:-128);
        u += ((u<128)?128:-128);
        v += ((v<128)?128:-128);

        y += brite;
      }
      else
        y = u = v = 0;

      if (flags & FLAGS_MONOCHROME)
      {
        switch (yuv)
        {
          case 2:
            r = g = b = v;
            break;
          case 1:
            r = g = b = u;
            break;
          case 0:
          default:
            r = g = b = y;
        }
      }
      else
      {
        YUV_TO_RGB_BY_THE_BOOK(y, u, v, r, g, b);
      }
      /* this is the slow way to put pixels. if you need speed,
       * check out how the IBM cam driver does it.
       */
      if (ULTRACAM_T(uvd)->invert)
      {
        RGB24_PUTPIXEL(frame, (VIDEOSIZE_X(frame->request)-i-1), (VIDEOSIZE_Y(frame->request)-frame->curline-3), r, g, b);
      }
      else
      {
        RGB24_PUTPIXEL(frame, i, frame->curline, r, g, b);
      }

      ULTRACAM_T(uvd)->ybar += y;
      ULTRACAM_T(uvd)->rbar += r;
      ULTRACAM_T(uvd)->gbar += g;
      ULTRACAM_T(uvd)->bbar += b;
    }
    frame->curline++;
  }

  if (frame->curline >= VIDEOSIZE_Y(frame->request))
    return scan_NextFrame;
  else
    return scan_Continue;

  #else

  int n;
  unsigned char buf;

  /* Try to move data from queue into frame buffer */
  n = RingQueue_GetLength(&uvd->dp);
  if (n > 0)
  {
    int m;
    /* See how much spare we have left */
    m = uvd->max_frame_size - frame->seqRead_Length;
    if (n > (m/3))
      n = (m/3);
    /* Now move that much data into frame buffer */
    while (n > 0)
    {
      ultracam_Dequeue(uvd, &buf, 1);
      n--;
      frame->data[frame->seqRead_Length] = buf;
      frame->seqRead_Length++;
      frame->data[frame->seqRead_Length] = buf;
      frame->seqRead_Length++;
      frame->data[frame->seqRead_Length] = buf;
      frame->seqRead_Length++;
    }
  }
  /* See if we filled the frame */
  if (frame->seqRead_Length >= uvd->max_frame_size)
  {
    frame->frameState = FrameState_Done;
    uvd->curframe = -1;
    uvd->stats.frame_num++;
    return scan_NextFrame;
  }

  return scan_Continue;

  #endif
}



/*
 * ultracam_ProcessIsocData()
 *
 * Generic routine to parse the ring queue data. It employs either
 * ultracam_find_header() or ultracam_parse_lines() to do most
 * of work.
 *
 * 02-Nov-2000 First (mostly dummy) version.
 * 06-Nov-2000 Rewrote to dump all data into frame.
 * 16-May-2001 Rewrote to use find_header and parse_lines.
 */
void ultracam_ProcessIsocData(uvd_t *uvd, usbvideo_frame_t *frame)
{
  /*  int n; */
  ParseState_t pstate;

  assert(uvd != NULL);
  assert(frame != NULL);

  while (1)
  {
    pstate = scan_Out;
    if (frame->scanstate == ScanState_Scanning)
    {
      pstate = ultracam_find_header(uvd, frame);
    }
    else if (frame->scanstate == ScanState_Lines)
    {
      pstate = ultracam_parse_lines(uvd, frame);
    }

    if (pstate == scan_Continue)
      continue;
    else if ((pstate == scan_NextFrame) || (pstate == scan_Out))
      break;
    else
      return;
  }

  if (pstate == scan_NextFrame)
  {
    frame->frameState = FrameState_Done;
    uvd->curframe = -1;
    uvd->stats.frame_num++;
  }

}
#endif

/*
 * ultracam_veio()
 *
 * History:
 * 1/27/00  Added check for dev == NULL; this happens if camera is unplugged.
 */

#define HZ 1000
#define usb_rcvctrlpipe(X,Y) Y
#define usb_control_msg(h,P,R,Rt,V,I,D,L,T) UsbCtrlMessage( h, Rt, R, V, I, L, D, T)

static int ultracam_veio(
                        uvd_t *uvd,
                        unsigned char req,
                        unsigned short value,
                        unsigned short index,
                        unsigned char *is_out)
{
  static const char proc[] = "ultracam_veio";
  #if 0
  unsigned char cp[8] /* = { 0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef } */;
  #endif
  int i;

  if (!CAMERA_IS_OPERATIONAL(uvd))
    return 0;

  if (is_out)
  {
    i = usb_control_msg(
                       uvd->dev,
                       usb_rcvctrlpipe(uvd->dev, 0),
                       req,
                       USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
                       value,
                       index,
                       is_out,
                       2,
                       HZ);
  #if 0
    printf("USB => %02x%02x%02x%02x%02x%02x%02x%02x "
         "(req=$%02x val=$%04x ind=$%04x)",
         cp[0],cp[1],cp[2],cp[3],cp[4],cp[5],cp[6],cp[7],
         req, value, index);
  #endif
  }
  else
  {
    i = usb_control_msg(
                       uvd->dev,
                       usb_sndctrlpipe(uvd->dev, 0),
                       req,
                       USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
                       value,
                       index,
                       NULL,
                       0,
                       HZ);
  }
  if (i )
  {
    printf("%s: ERROR=%d. Camera stopped; Reconnect or reload driver.",
        proc, i);
    uvd->last_error = i;
  }
  return i;
}

#define usb_set_interface(H,I,S) UsbInterfaceSetAltSetting(H,I,S)

static int ultracam_alternateSetting(uvd_t *uvd, int setting)
{
  static const char proc[] = "ultracam_alternateSetting";
  int i;
  i = usb_set_interface(uvd->dev, uvd->iface, setting);
  if (i )
  {
    printf("%s: usb_set_interface error\n", proc);
    uvd->last_error = i;
    return -1;//EBUSY;
  }
  return 0;
}

/*
 * ultracam_calculate_fps()
 */
static int ultracam_calculate_fps(uvd_t *uvd)
{
  /* camera does not tell me, so I guess, no? */
  return 30; /* YEAH RIGHT! more like 5... */
}

/*
 * ultracam_adjust_contrast()
 */
static void ultracam_adjust_contrast(uvd_t *uvd)
{
}

/*
 * ultracam_set_white_balance()
 */
static void ultracam_set_white_balance(uvd_t *uvd)
{
  int wb;
  /* white balance codes to send to cam (sniffed from Windows) */
  unsigned short *wbp;
  unsigned short whitebalcodes[49 * 6] = {
    0x0034, 0x003f, 0x00da, 0x00ff, 0x0002, 0x00f5, 
    0x0035, 0x003f, 0x00d6, 0x00ff, 0x0002, 0x00f5, 
    0x0036, 0x003f, 0x00d3, 0x00ff, 0x0002, 0x00f5, 
    0x0037, 0x003f, 0x00cf, 0x00ff, 0x0002, 0x00f5, 
    0x0038, 0x003f, 0x00cc, 0x00ff, 0x0002, 0x00f5, 
    0x0038, 0x003f, 0x00c8, 0x00ff, 0x0002, 0x00f5, 
    0x0039, 0x003f, 0x00c5, 0x00ff, 0x0002, 0x00f5, 
    0x003a, 0x003f, 0x00c1, 0x00ff, 0x0002, 0x00f5, 
    0x003b, 0x003f, 0x00be, 0x0000, 0x0002, 0x00f6, 
    0x003c, 0x003f, 0x00ba, 0x0000, 0x0002, 0x00f6, 
    0x003d, 0x003f, 0x00b6, 0x0000, 0x0002, 0x00f6, 
    0x003e, 0x003f, 0x00b3, 0x0000, 0x0002, 0x00f6, 
    0x003f, 0x003f, 0x00af, 0x0001, 0x0002, 0x00f6, 
    0x003f, 0x003f, 0x00ac, 0x0001, 0x0002, 0x00f6, 
    0x0040, 0x003f, 0x00a8, 0x0001, 0x0002, 0x00f6, 
    0x0041, 0x003f, 0x00a5, 0x0001, 0x0002, 0x00f6, 
    0x0042, 0x003f, 0x00a1, 0x0001, 0x0002, 0x00f6, 
    0x0045, 0x003f, 0x009f, 0x0001, 0x0002, 0x00f6, 
    0x0049, 0x003f, 0x009d, 0x0001, 0x0002, 0x00f6, 
    0x004c, 0x003f, 0x009b, 0x0000, 0x0002, 0x00f7, 
    0x004f, 0x003f, 0x009a, 0x0000, 0x0003, 0x00f7, 
    0x0053, 0x003f, 0x0098, 0x00ff, 0x0003, 0x00f7, 
    0x0056, 0x003f, 0x0096, 0x00ff, 0x0003, 0x00f7, 
    0x0059, 0x003f, 0x0094, 0x00ff, 0x0003, 0x00f7, 
    0x005d, 0x003f, 0x0092, 0x00ff, 0x0003, 0x00f8, 
    0x0060, 0x003f, 0x0090, 0x00ff, 0x0003, 0x00f8, 
    0x0063, 0x003f, 0x008e, 0x00ff, 0x0003, 0x00f8, 
    0x0066, 0x003f, 0x008c, 0x00fe, 0x0003, 0x00f8, 
    0x006a, 0x003f, 0x008b, 0x00fe, 0x0004, 0x00f8, 
    0x006d, 0x003f, 0x0089, 0x00fe, 0x0004, 0x00f8, 
    0x0070, 0x003f, 0x0087, 0x00fe, 0x0004, 0x00f9, 
    0x0074, 0x003f, 0x0085, 0x00fd, 0x0004, 0x00f9, 
    0x0077, 0x003f, 0x0083, 0x00fd, 0x0004, 0x00f9, 
    0x0078, 0x003f, 0x007f, 0x00fd, 0x0004, 0x00f9, 
    0x007a, 0x003f, 0x007b, 0x00fd, 0x0004, 0x00fa, 
    0x007b, 0x003f, 0x0077, 0x00fc, 0x0004, 0x00fa, 
    0x007d, 0x003f, 0x0073, 0x00fc, 0x0004, 0x00fa, 
    0x007e, 0x003f, 0x006f, 0x00fc, 0x0003, 0x00fa, 
    0x007f, 0x003f, 0x006b, 0x00fc, 0x0003, 0x00fb, 
    0x0081, 0x003f, 0x0067, 0x00fb, 0x0003, 0x00fb, 
    0x0082, 0x0040, 0x0064, 0x00fb, 0x0003, 0x00fb, 
    0x0083, 0x0040, 0x0060, 0x00fb, 0x0003, 0x00fb, 
    0x0085, 0x0040, 0x005c, 0x00fb, 0x0003, 0x00fc, 
    0x0086, 0x0040, 0x0058, 0x00fa, 0x0003, 0x00fc, 
    0x0088, 0x0040, 0x0054, 0x00fa, 0x0003, 0x00fc, 
    0x0089, 0x0040, 0x0050, 0x00fa, 0x0002, 0x00fc, 
    0x008a, 0x0040, 0x004c, 0x00fa, 0x0002, 0x00fd, 
    0x008c, 0x0040, 0x0048, 0x00f9, 0x0002, 0x00fd, 
    0x008d, 0x0040, 0x0044, 0x00f9, 0x0002, 0x00fd 
  };
  /* I'm trying to figure out what
   * these codes actually mean. Some hints may
   * include rgb or yuv gain within the camera
   * itself, but it doesn't make a whole lot of
   * sense, since some values change a lot, some
   * hardly change at all, and none is precisely
   * linear or even suggests a coordinating function
   * (Yes, I plotted the values out. They're not
   * linear.)
   *
   * Any ideas?
   * -Karl
   */

  RESTRICT_TO_RANGE(whitebal, -1, 48);

  if (whitebal >= 0)
    wb = whitebal;
  else
    wb = ULTRACAM_T(uvd)->projected_whitebal;

  wbp = whitebalcodes + (wb*6);

  ultracam_veio(uvd, 0x01, ((wb_51==0xff00)?*wbp:wb_51), 0x0051, NULL); wbp++;
  ultracam_veio(uvd, 0x01, ((wb_52==0xff00)?*wbp:wb_52), 0x0052, NULL); wbp++;
  ultracam_veio(uvd, 0x01, ((wb_53==0xff00)?*wbp:wb_53), 0x0053, NULL); wbp++;
  ultracam_veio(uvd, 0x01, ((wb_11==0xff00)?*wbp:wb_11), 0x0011, NULL); wbp++;
  ultracam_veio(uvd, 0x01, ((wb_12==0xff00)?*wbp:wb_12), 0x0012, NULL); wbp++;
  ultracam_veio(uvd, 0x01, ((wb_13==0xff00)?*wbp:wb_13), 0x0013, NULL);

}

/*
 * ultracam_set_sharpness()
 *
 * Cameras have internal smoothing feature. It is controlled by value in
 * range [0..4], where 0 is most smooth and 4 is most sharp (raw image, I guess).
 * Recommended value is 3. 
 */
static void ultracam_set_sharpness(uvd_t *uvd)
{
  unsigned char buf[2];
  unsigned short sharps[SHARPNESS_MAX + 1] = { 0x0004, 0x0024, 0x0064, 0x00a4, 0x00e4};

  RESTRICT_TO_RANGE(sharpness, SHARPNESS_MIN, SHARPNESS_MAX);
  ultracam_veio(uvd, 0x01, 0x0000, 0x0001, buf);
  ultracam_veio(uvd, 0x01, sharps[sharpness], 0x0001, NULL);

}

/*
 * ultracam_set_brightness()
 *
 * This procedure changes brightness of the picture.
 */
static void ultracam_set_brightness(uvd_t *uvd)
{
}

static void ultracam_set_hue(uvd_t *uvd)
{
}




/*
 * ultracam_adjust_picture()
 *
 * This procedure gets called from V4L interface to update picture settings.
 * Here we change brightness and contrast.
 */
static void ultracam_adjust_picture(uvd_t *uvd)
{
  ultracam_adjust_contrast(uvd);
  ultracam_set_brightness(uvd);
  ultracam_set_hue(uvd);
}

/*
 * ultracam_video_stop()
 *
 * This code tells camera to stop streaming. The interface remains
 * configured and bandwidth - claimed.
 */
static void ultracam_video_stop(uvd_t *uvd)
{
  /* I think it's this... */
  ultracam_veio(uvd, 0x02, 0x0000, 0x0001, NULL);
  ultracam_alternateSetting(uvd, 0x00);
  ultracam_veio(uvd, 0x01, 0x0000, 0x0001, NULL);
  ultracam_veio(uvd, 0x02, 0x0003, 0x0000, NULL);
  ultracam_veio(uvd, 0x02, 0x0000, 0x0005, NULL);

}

/*
 * ultracam_reinit_iso()
 *
 * This procedure sends couple of commands to the camera and then
 * resets the video pipe. This sequence was observed to reinit the
 * camera or, at least, to initiate ISO data stream.
 */
static void ultracam_reinit_iso(uvd_t *uvd, int do_stop)
{
}

static void ultracam_video_start(uvd_t *uvd)
{
  //	ultracam_change_lighting_conditions(uvd);
  /* this is doing no good here... looks like 
   * sharpness is set on camera init.
        ultracam_set_sharpness(uvd);
   */

  ultracam_reinit_iso(uvd, 0);
}

#define usb_clear_halt(H,E) UsbEndpointClearHalt(H,E)
static int ultracam_resetPipe(uvd_t *uvd)
{
  usb_clear_halt(uvd->dev, uvd->video_endp);
  return 0;
}


static int ultracam_is_backward(uvd_t *uvd)
{
  unsigned char buf[2];
  buf[0] = 0x00;
  buf[1] = 0x00;
  ultracam_veio(uvd, 0x02, 0x0000, 0x000b, buf);
  if (buf[1] == 0x05)
  {
    if (debug > 2)
      printf("Camera is facing forward  [%02x %02x]\n",buf[0],buf[1]);
    return 0;
  }
  else
  {
    if (debug > 2)
      printf("Camera is facing backward [%02x %02x]\n",buf[0],buf[1]);
    return 1;
  }
}

/*
 * Return negative code on failure, 0 on success.
 */
static int ultracam_setup_on_open(uvd_t *uvd)
{
  unsigned char buf[2];

  int setup_ok = 0; /* Success by default */

  ULTRACAM_T(uvd)->projected_whitebal = 20;

  /* Send init sequence only once, it's large! */
  if (!ULTRACAM_T(uvd)->initialized)
  {
    ultracam_alternateSetting(uvd, 0x04);
    ultracam_alternateSetting(uvd, 0x00);
    ultracam_veio(uvd, 0x02, 0x0004, 0x000b, NULL);
    ultracam_veio(uvd, 0x02, 0x0001, 0x0005, NULL);
    ultracam_veio(uvd, 0x02, 0x8000, 0x0000, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x0000, NULL);
    ultracam_veio(uvd, 0x00, 0x00b0, 0x0001, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x0002, NULL);
    ultracam_veio(uvd, 0x00, 0x000c, 0x0003, NULL);
    ultracam_veio(uvd, 0x00, 0x000b, 0x0004, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x0005, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x0006, NULL);
    ultracam_veio(uvd, 0x00, 0x0079, 0x0007, NULL);
    ultracam_veio(uvd, 0x00, 0x003b, 0x0008, NULL);
    ultracam_veio(uvd, 0x00, 0x0002, 0x000f, NULL);
    ultracam_veio(uvd, 0x00, 0x0001, 0x0010, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x0011, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00bf, NULL);
    ultracam_veio(uvd, 0x00, 0x0001, 0x00c0, NULL);
    ultracam_veio(uvd, 0x00, 0x0010, 0x00cb, NULL);
    ultracam_veio(uvd, 0x01, 0x00a4, 0x0001, NULL);
    ultracam_veio(uvd, 0x01, 0x0010, 0x0002, NULL);
    ultracam_veio(uvd, 0x01, 0x0066, 0x0007, NULL);
    ultracam_veio(uvd, 0x01, 0x000b, 0x0008, NULL);
    ultracam_veio(uvd, 0x01, 0x0034, 0x0009, NULL);
    ultracam_veio(uvd, 0x01, 0x0000, 0x000a, NULL);
    ultracam_veio(uvd, 0x01, 0x002e, 0x000b, NULL);
    ultracam_veio(uvd, 0x01, 0x00d6, 0x000c, NULL);
    ultracam_veio(uvd, 0x01, 0x00fc, 0x000d, NULL);
    ultracam_veio(uvd, 0x01, 0x00f1, 0x000e, NULL);
    ultracam_veio(uvd, 0x01, 0x00da, 0x000f, NULL);
    ultracam_veio(uvd, 0x01, 0x0036, 0x0010, NULL);
    ultracam_veio(uvd, 0x01, 0x000b, 0x0011, NULL);
    ultracam_veio(uvd, 0x01, 0x0001, 0x0012, NULL);
    ultracam_veio(uvd, 0x01, 0x0000, 0x0013, NULL);
    ultracam_veio(uvd, 0x01, 0x0000, 0x0014, NULL);
    ultracam_veio(uvd, 0x01, 0x0087, 0x0051, NULL);
    ultracam_veio(uvd, 0x01, 0x0040, 0x0052, NULL);
    ultracam_veio(uvd, 0x01, 0x0058, 0x0053, NULL);
    ultracam_veio(uvd, 0x01, 0x0040, 0x0054, NULL);
    ultracam_veio(uvd, 0x01, 0x0000, 0x0040, NULL);
    ultracam_veio(uvd, 0x01, 0x0010, 0x0041, NULL);
    ultracam_veio(uvd, 0x01, 0x0020, 0x0042, NULL);
    ultracam_veio(uvd, 0x01, 0x0030, 0x0043, NULL);
    ultracam_veio(uvd, 0x01, 0x0040, 0x0044, NULL);
    ultracam_veio(uvd, 0x01, 0x0050, 0x0045, NULL);
    ultracam_veio(uvd, 0x01, 0x0060, 0x0046, NULL);
    ultracam_veio(uvd, 0x01, 0x0070, 0x0047, NULL);
    ultracam_veio(uvd, 0x01, 0x0080, 0x0048, NULL);
    ultracam_veio(uvd, 0x01, 0x0090, 0x0049, NULL);
    ultracam_veio(uvd, 0x01, 0x00a0, 0x004a, NULL);
    ultracam_veio(uvd, 0x01, 0x00b0, 0x004b, NULL);
    ultracam_veio(uvd, 0x01, 0x00c0, 0x004c, NULL);
    ultracam_veio(uvd, 0x01, 0x00d0, 0x004d, NULL);
    ultracam_veio(uvd, 0x01, 0x00e0, 0x004e, NULL);
    ultracam_veio(uvd, 0x01, 0x00f0, 0x004f, NULL);
    ultracam_veio(uvd, 0x01, 0x00ff, 0x0050, NULL);
    ultracam_veio(uvd, 0x01, 0x0000, 0x0056, NULL);
    ultracam_veio(uvd, 0x00, 0x0080, 0x00c1, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c2, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00ca, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0080, 0x00c1, NULL);
    ultracam_veio(uvd, 0x00, 0x0004, 0x00c2, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00ca, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0002, 0x00c1, NULL);
    ultracam_veio(uvd, 0x00, 0x0020, 0x00c2, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00ca, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c3, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c4, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c5, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c6, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c7, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c8, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c3, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c4, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c5, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c6, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c7, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c8, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0040, 0x00c1, NULL);
    ultracam_veio(uvd, 0x00, 0x0017, 0x00c2, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00ca, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c3, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c4, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c5, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c6, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c7, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c8, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c3, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c4, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c5, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c6, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c7, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c8, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c9, NULL);
    ultracam_veio(uvd, 0x00, 0x00c0, 0x00c1, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00c2, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00ca, NULL);

    /* command mode? */
    ultracam_veio(uvd, 0x02, 0xc040, 0x0001, NULL);

    /* read in hue/sat? */
    ultracam_veio(uvd, 0x01, 0x0000, 0x0008, buf);
    printf("hs_read: 0x08 -- 0x%x\n", buf[0]);
    ultracam_veio(uvd, 0x01, 0x0000, 0x0009, buf);
    printf("hs_read: 0x09 -- 0x%x\n", buf[0]);
    ultracam_veio(uvd, 0x01, 0x0000, 0x000a, buf);
    printf("hs_read: 0x0a -- 0x%x\n", buf[0]);
    ultracam_veio(uvd, 0x01, 0x0000, 0x000b, buf);
    printf("hs_read: 0x0b -- 0x%x\n", buf[0]);
    ultracam_veio(uvd, 0x01, 0x0000, 0x000c, buf);
    printf("hs_read: 0x0c -- 0x%x\n", buf[0]);
    ultracam_veio(uvd, 0x01, 0x0000, 0x000d, buf);
    printf("hs_read: 0x0d -- 0x%x\n", buf[0]);
    ultracam_veio(uvd, 0x01, 0x0000, 0x000e, buf);
    printf("hs_read: 0x0e -- 0x%x\n", buf[0]);
    ultracam_veio(uvd, 0x01, 0x0000, 0x000f, buf);
    printf("hs_read: 0x0f -- 0x%x\n", buf[0]);
    ultracam_veio(uvd, 0x01, 0x0000, 0x0010, buf);
    printf("hs_read: 0x10 -- 0x%x\n", buf[0]);

    /* hue/saturation */
    ultracam_veio(uvd, 0x01, 0x000b, 0x0008, NULL);
    ultracam_veio(uvd, 0x01, 0x0034, 0x0009, NULL);
    ultracam_veio(uvd, 0x01, 0x0000, 0x000a, NULL);

    ultracam_veio(uvd, 0x01, hs_0b, 0x000b, NULL);
    ultracam_veio(uvd, 0x01, hs_0c, 0x000c, NULL);
    ultracam_veio(uvd, 0x01, hs_0d, 0x000d, NULL);
    ultracam_veio(uvd, 0x01, hs_0e, 0x000e, NULL);
    ultracam_veio(uvd, 0x01, hs_0f, 0x000f, NULL);
    ultracam_veio(uvd, 0x01, hs_10, 0x0010, NULL);

    /* sharpness
       ultracam_veio(uvd, 0x01, 0x0000, 0x0001, buf);
       ultracam_veio(uvd, 0x01, 0x0064, 0x0001, NULL);
    */
    ultracam_set_sharpness(uvd);

    /* white balance */
    ultracam_set_white_balance(uvd);
    /*
      ultracam_veio(uvd, 0x01, wb_51, 0x0051, NULL);
      ultracam_veio(uvd, 0x01, wb_52, 0x0052, NULL);
      ultracam_veio(uvd, 0x01, wb_53, 0x0053, NULL);
      ultracam_veio(uvd, 0x01, wb_11, 0x0011, NULL);
      ultracam_veio(uvd, 0x01, wb_12, 0x0012, NULL);
      ultracam_veio(uvd, 0x01, wb_13, 0x0013, NULL);
    */


    /* here I have >>>>>>    0x0007  (unknown setting?) */
    ultracam_veio(uvd, 0x00, 0x0009, 0x0011, NULL);

    ultracam_veio(uvd, 0x00, 0x0000, 0x0001, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x0000, NULL);

    /* on my system, these look like this:
     *                 0x00  0x001b  0x00c1
     *                 0x00  0x00e0  0x00c2
     *                 0x00  0x0000  0x00ca
     * some unknown values?
     * -Karl
     */
    ultracam_veio(uvd, 0x00, 0x0020, 0x00c1, NULL);
    ultracam_veio(uvd, 0x00, 0x0010, 0x00c2, NULL);
    ultracam_veio(uvd, 0x00, 0x0000, 0x00ca, NULL);

    ultracam_alternateSetting(uvd, 0x04);
    ultracam_veio(uvd, 0x02, 0x0000, 0x0001, NULL);
    ultracam_veio(uvd, 0x02, 0x0000, 0x0001, NULL);
    ultracam_veio(uvd, 0x02, 0x0000, 0x0006, NULL);

    /* resolution */
    ultracam_veio(uvd, 0x02, 0x9000, 0x0007, NULL);

    /* turn on camera? */
    ultracam_veio(uvd, 0x02, 0x0042, 0x0001, NULL);

    ultracam_is_backward(uvd);
    ultracam_resetPipe(uvd);
    ULTRACAM_T(uvd)->initialized = (setup_ok != 0);

  }
  return setup_ok;
}

static void ultracam_configure_video(uvd_t *uvd)
{
  if (uvd == NULL)
    return;

  RESTRICT_TO_RANGE(init_brightness, 0, 255);
  RESTRICT_TO_RANGE(init_contrast, 0, 255);
  RESTRICT_TO_RANGE(init_sat, 0, 20);
  RESTRICT_TO_RANGE(init_hue, 0, 360);
  RESTRICT_TO_RANGE(wbmaxdelt, 1, 48);

  memset(&uvd->vpic, 0, sizeof(uvd->vpic));
  memset(&uvd->vpic_old, 0x55, sizeof(uvd->vpic_old));

  uvd->vpic.colour = init_sat << 8;
  uvd->vpic.hue = init_hue << 8;
  uvd->vpic.brightness = init_brightness << 8;
  uvd->vpic.contrast = init_contrast << 8;
  uvd->vpic.whiteness = 105 << 8; /* This one isn't used */
  uvd->vpic.depth = 24;
  uvd->vpic.palette = VIDEO_PALETTE_RGB24;

  memset(&uvd->vcap, 0, sizeof(uvd->vcap));
  strcpy(uvd->vcap.name, "IBM Ultraport Camera");
  uvd->vcap.type = VID_TYPE_CAPTURE;
  uvd->vcap.channels = 1;
  uvd->vcap.audios = 0;
  uvd->vcap.maxwidth = VIDEOSIZE_X(uvd->canvas);
  uvd->vcap.maxheight = VIDEOSIZE_Y(uvd->canvas);
  uvd->vcap.minwidth = min_canvasWidth;
  uvd->vcap.minheight = min_canvasHeight;

  memset(&uvd->vchan, 0, sizeof(uvd->vchan));
  uvd->vchan.flags = 0;
  uvd->vchan.tuners = 0;
  uvd->vchan.channel = 0;
  uvd->vchan.type = VIDEO_TYPE_CAMERA;
  strcpy(uvd->vchan.name, "Camera");
}

/*
 * ultracam_probe()
 *
 * This procedure queries device descriptor and accepts the interface
 * if it looks like our camera.
 *
 * History:
 * 12-Nov-2000 Reworked to comply with new probe() signature.
 * 23-Jan-2001 Added compatibility with 2.2.x kernels.
 */



#if 0
static void *ultracam_probe(struct usb_device *dev, unsigned int ifnum
#else
#define  MOD_INC_USE_COUNT
#define  MOD_DEC_USE_COUNT

uvd_t *usbvideo_AllocateDevice(usbvideo_t *cams)
{
  uvd_t* Dev;
  Dev = (uvd_t*)malloc(sizeof(uvd_t));
  memset(Dev, 0, sizeof(uvd_t));
  Dev->user_data = (ultracam_t*) malloc(sizeof(ultracam_t));
  memset(Dev->user_data, 0, sizeof(ultracam_t));

  return Dev;
}

static void *ultracam_os2init(USBHANDLE dev, unsigned int ifnum
#endif
  #if defined(usb_device_id_ver)
                            ,const struct usb_device_id *devid
  #endif
                           )
{
  uvd_t *uvd = NULL;
  int i, nas;
  int actInterface=7, inactInterface=0, maxPS=0x3ff; // Hardcode values from report
  unsigned char video_ep = 0x81;

  if (debug >= 1)
    printf("ultracam_probe(%p,%u.)", dev, ifnum);

#if 0 

  /* We don't handle multi-config cameras */
  if (dev->descriptor.bNumConfigurations != 1)
    return NULL;

  /* Is it an IBM camera? */
  if ((dev->descriptor.idVendor != ULTRACAM_VENDOR_ID) ||
      (dev->descriptor.idProduct != ULTRACAM_PRODUCT_ID))
    return NULL;

  printf("IBM Ultra camera found (rev. 0x%04x)", dev->descriptor.bcdDevice);

  /* Validate found interface: must have one ISO endpoint */
  nas = dev->actconfig->interface[ifnum].num_altsetting;
  if (debug > 0)
    printf("Number of alternate settings=%d.", nas);
  if (nas < 8)
  {
    err("Too few alternate settings for this camera!");
    return NULL;
  }

  /* Validate all alternate settings */
  for (i=0; i < nas; i++)
  {
    const struct usb_interface_descriptor *interface;
    const struct usb_endpoint_descriptor *endpoint;

    interface = &dev->actconfig->interface[ifnum].altsetting[i];
    if (interface->bNumEndpoints != 1)
    {
      printf("Interface %d. has %u. endpoints!",
          ifnum, (unsigned)(interface->bNumEndpoints));
      return NULL;
    }
    endpoint = &interface->endpoint[0];
    if (video_ep == 0)
      video_ep = endpoint->bEndpointAddress;
    else if (video_ep != endpoint->bEndpointAddress)
    {
      printf("Alternate settings have different endpoint addresses!");
      return NULL;
    }
    if ((endpoint->bmAttributes & 0x03) != 0x01)
    {
      printf("Interface %d. has non-ISO endpoint!", ifnum);
      return NULL;
    }
    if ((endpoint->bEndpointAddress & 0x80) == 0)
    {
      printf("Interface %d. has ISO OUT endpoint!", ifnum);
      return NULL;
    }
    if (endpoint->wMaxPacketSize == 0)
    {
      if (inactInterface < 0)
        inactInterface = i;
      else
      {
        printf("More than one inactive alt. setting!");
        return NULL;
      }
    }
    else
    {
      if (actInterface < 0)
      {
        actInterface = i;
        maxPS = endpoint->wMaxPacketSize;
        if (debug > 0)
          printf("Active setting=%d. maxPS=%d.", i, maxPS);
      }
      else
      {
        /* Got another active alt. setting */
        if (maxPS < endpoint->wMaxPacketSize)
        {
          /* This one is better! */
          actInterface = i;
          maxPS = endpoint->wMaxPacketSize;
          if (debug > 0)
          {
            printf("Even better ctive setting=%d. maxPS=%d.",
                 i, maxPS);
          }
        }
      }
    }
  }
  if ((maxPS <= 0) || (actInterface < 0) || (inactInterface < 0))
  {
    printf("Failed to recognize the camera!");
    return NULL;
  }

  /* Code below may sleep, need to lock module while we are here */
#endif
  MOD_INC_USE_COUNT;
  uvd = usbvideo_AllocateDevice(cams);

  if (uvd != NULL)
  {
    /* Here uvd is a fully allocated uvd_t object */
    uvd->flags = flags;
    uvd->debug = debug;
    uvd->dev = dev;
    uvd->iface = ifnum;
    uvd->ifaceAltInactive = inactInterface;
    uvd->ifaceAltActive = actInterface;
    uvd->video_endp = video_ep;
    uvd->iso_packet_len = maxPS;
    uvd->paletteBits = 1L << VIDEO_PALETTE_RGB24;
    uvd->defaultPalette = VIDEO_PALETTE_RGB24;
    uvd->canvas = VIDEOSIZE(320, 240);  /* FIXME */
    uvd->videosize = uvd->canvas; /* ultracam_size_to_videosize(size);*/

    /* Initialize ibmcam-specific data */
    assert(ULTRACAM_T(uvd) != NULL);
    ULTRACAM_T(uvd)->camera_model = 0; /* Not used yet */
    ULTRACAM_T(uvd)->initialized = 0;
    ULTRACAM_T(uvd)->bytes_in = 0;

    ultracam_configure_video(uvd);

    i = 0;//usbvideo_RegisterVideoDevice(uvd);
    if (i != 0)
    {
      printf("usbvideo_RegisterVideoDevice() failed.");
      uvd = NULL;
    }
  }
  MOD_DEC_USE_COUNT;
  return uvd;
}


#if 0
/*
 * ultracam_init()
 *
 * This code is run to initialize the driver.
 */
static int __init ultracam_init(void)
{
  usbvideo_cb_t cbTbl;
  memset(&cbTbl, 0, sizeof(cbTbl));
  cbTbl.probe = ultracam_probe;
  cbTbl.setupOnOpen = ultracam_setup_on_open;
  cbTbl.videoStart = ultracam_video_start;
  cbTbl.videoStop = ultracam_video_stop;
  cbTbl.processData = ultracam_ProcessIsocData;
  cbTbl.postProcess = ultracam_postProcess;
  cbTbl.adjustPicture = ultracam_adjust_picture;
  cbTbl.getFPS = ultracam_calculate_fps;
  return usbvideo_register(
                          &cams,
                          MAX_CAMERAS,
                          sizeof(ultracam_t),
                          "ultracam",
                          &cbTbl,
                          THIS_MODULE);
}

static void __exit ultracam_cleanup(void)
{
  usbvideo_Deregister(&cams);
}

  #if defined(usb_device_id_ver)

static __devinitdata struct usb_device_id id_table[] = {
  { USB_DEVICE(ULTRACAM_VENDOR_ID, ULTRACAM_PRODUCT_ID)},
  {}  /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, id_table);

  #endif /* defined(usb_device_id_ver) */

module_init(ultracam_init);
module_exit(ultracam_cleanup);

#endif

//////////////////////////////////////////
////////// OS/2 RING 3 Test APP //////////
//////////////////////////////////////////

void main(int argc, char *argv[])
{
  USBHANDLE hDev;
  APIRET rc;
  uvd_t *pCam;

  rc = UsbOpen( &hDev,	
                ULTRACAM_VENDOR_ID,
	            ULTRACAM_PRODUCT_ID,
                USB_ANY_PRODUCTVERSION, // or try 0x0103
                USB_OPEN_FIRST_UNUSED);
  if(!rc)
  {
    printf("Open OK\n");
    pCam = (uvd_t*)ultracam_os2init(hDev, 0);
    if(pCam)
    {
      char ch;
      printf( "Chose Action:\n"
              " Get Camera (P)osition\n"
              " (S)etup Camera\n"
              " Sta(r)t Video\n"
              " S(t)op Video\n"
              " E(x)it\n");
      while(1)
      {
        ch = getch();
        switch(ch)
        {
          case 'p':
          case 'P':
            ultracam_is_backward(pCam);
            break;
          case 's':
          case 'S':
            ultracam_setup_on_open(pCam);
            break;
          case 'r':
          case 'R':
            ultracam_video_start(pCam);
            break;
          case 't':
          case 'T':
            ultracam_video_stop(pCam);
            break;
        }
        if((ch=='x') || (ch=='X'))
          break;
      }
    }
    UsbClose(hDev);
  }
  else
    printf("Camera Not found (rc=%d)\r\n",rc);

}

