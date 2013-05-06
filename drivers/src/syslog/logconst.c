/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  TSTCONST.C                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB Log driver Constants                              */
/*                                                                            */
/*   FUNCTION: This module allocates the global constants (strings) for the   */
/*             USB Log Driver.                                                */
/*                                                                            */
/*   NOTES: This module exists so that the strings will be placed after       */
/*          the device driver header in the data segment.                     */
/*                                                                            */
/*      DEPENDENCIES: None                                                    */
/*      RESTRICTIONS: None                                                    */
/*                                                                            */
/*   ENTRY POINTS:                                                            */
/*             None                                                           */
/*                                                                            */
/*   EXTERNAL REFERENCES:                                                     */
/*             None                                                           */
/*                                                                            */
/* Change Log                                                                 */
/*                                                                            */
/*  Mark    yy/mm/dd  Programmer      Comment                                 */
/*  ----    --------  ----------      -------                                 */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

const char gDDName[] = "SYSLOG.SYS";
const char gDDDesc[] = "Driver to do logging";
const char gVendorID[] = "OS/2 Netlabs";
const char gAdapterName[] = "Log Driver";

