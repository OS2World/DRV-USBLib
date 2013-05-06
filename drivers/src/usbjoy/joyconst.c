/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  JOYCONST.C                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB Joystick driver constants                         */
/*                                                                            */
/*   FUNCTION: This module allocates the global constants (strings) for the   */
/*             USB Joystick driver.                                           */
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
/*          00/01/04  MM                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

const char gDDName[]      = "USBJOY.SYS";
const char gDDDesc[]      = "USB Joystick driver";
const char gVendorID[]    = "Markus Montkowski for Netl@bs";
const char gAdapterName[] = "USB Joystick Client";

