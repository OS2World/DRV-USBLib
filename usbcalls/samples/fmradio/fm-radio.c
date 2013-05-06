/* fm-radio.c: A user-level program to set the Gemtek (D-Link) FM tuner. */
/*
  Written 2000 by Donald Becker, Scyld Computing Corporation.
  Released under the GPL.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>

/* This should be read from a usb-char.h header file instead. */
typedef unsigned char u8;
typedef unsigned short u16;
struct usb_op_request {
    u8  request_type;
    u8  request;
    u16 value;
    u16 index;
    u16 length;
    u8  data[8];
};

#define USB_IOCTL_OUT _IOW('U', 64, 72)
#define USB_IOCTL_IN  _IOWR('U', 65, 72)

static int do_usb_op(int fd, const char *op_name,
		     struct usb_op_request *usb_op);

/* The following are the known command of the radio. */
static struct usb_op_request get_status = {0xC0, 0x00, 0x00, 0x24, 1},
    set_freq = {0xC0, 0x01, 0, 0, 1},
    fm_mute = {0xC0, 0x02, 0 /* or 1 to unmute */, 0, 1},
    set_freq1 = {0xC0, 0x00, 0x96, 0xB7, 1},
    fm_on0 = {0xC0, 0x00, 0x00, 0xC7, 1},
    fm_off0 = {0xC0, 0x00, 0x16, 0x1C, 1},
    fm_unmute = {0xC0, 0x02, 0x01, 0x00, 1};

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

const int if_freq = 10700001;	/* +1 for rounding.  */
#define STERO_DELAY 240000

struct top {
    char *cmdopt;
    char *msg;
    struct usb_op_request *usb_op;
} cmp_ops[] = {
    {"off", "Turning tuner off", &fm_mute},
    {"mute", "Turning tuner off", &fm_mute},
    {"cmd1", "Cmd1", &fm_on0},
    {"cmd2", "Cmd2", &set_freq1},
    {"off1", "Off1", &fm_off0},
    {"unmute", "Unmuting radio", &fm_unmute},
    {0,}};

int main(int argc, char *argv[])
{
    int fd;
    int pll_div, saved_freq = 0;
    char *devname = "/dev/usb0";

    fd = open(devname, O_RDONLY);
    if (fd < 0) {
	fprintf(stderr, "Cannot open \"%s\"\n", devname);
	return (-1);
    }

    /* do_usb_op(fd, "Turn on power", &fm_on0);*/
    /* do_usb_op(fd, "Turn on sound", &fm_on1);*/
    if (argc >= 2  && argv[1]) {
	double freq_mhz = 99.1;
	int i;
	for (i = 0; cmp_ops[i].cmdopt; i++) {
	    if (strcasecmp(argv[1], cmp_ops[i].cmdopt) == 0) {
		do_usb_op(fd, cmp_ops[i].msg, cmp_ops[i].usb_op);
		return 0;
	    }
	}
	if ((freq_mhz = atof(argv[1])) == 0) {
	    fprintf(stderr, "The frequency must be a number 88.0 - 108.9.\n");
	    return 2;
	}
	pll_div = (freq_mhz*1000000 + if_freq) / 12500;
	set_freq.value = pll_div >> 8;
	set_freq.index = pll_div;
	do_usb_op(fd, NULL, &set_freq);
	printf("Setting tuner to %3.1f Mhz (%d).\n", freq_mhz, pll_div);
	/* do_usb_op(fd, NULL, &set_freq1);*/
	usleep(STERO_DELAY);
	printf(" Stero indicator is %s.\n",
	       do_usb_op(fd, NULL, &get_status) == 0xff ? "off" : "on");
	return 0;
    }

    /* Do a frequency scan. */
    pll_div = 7904;
    printf("Starting scan frequency is %3.3f (%d).\n",
	   (12500*pll_div - if_freq)/1000000.0, pll_div);
    for (; pll_div < 9576; pll_div += 16) {
	set_freq.value = (pll_div >> 8) & 0xff;
	set_freq.index = pll_div & 0xff;
	printf("\r%5.1f", (12500*pll_div - if_freq)/1000000.0);
	fflush(stdout);
	do_usb_op(fd, NULL, &set_freq);
	/*do_usb_op(fd, NULL, &set_freq1);*/
	usleep(STERO_DELAY);
	if (do_usb_op(fd, NULL, &get_status) != 0xff) {
	    printf(" Stero is on!\n");
	    saved_freq = pll_div;
	}
    }

    if (saved_freq) {
	set_freq.value = saved_freq >> 8;
	set_freq.index = saved_freq;
	do_usb_op(fd, NULL, &set_freq);
	printf("\rSetting tuner to last station, %3.1f.\n",
	       (12500*saved_freq - if_freq)/1000000.0);
    } else
	printf("\r Finished scan with no found stations.\n");

    return 0;
}

static int do_usb_op(int fd, const char *op_name, struct usb_op_request *usb_op)
{
    int i;
    int result;
    result = ioctl(fd, USB_IOCTL_IN, usb_op);
    if (op_name) {
	printf("%s:", op_name);
	for (i = 0; i < 8; i++)
	    printf(" %2.2x", usb_op->data[i]);
	printf(".\n");
    }
    return usb_op->data[0];
}

/*
 * Local variables:
 *  compile-command: "gcc -Wall -Wstrict-prototypes -O6 fm-radio.c -o fm-radio"
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */
