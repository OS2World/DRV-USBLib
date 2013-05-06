/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  JOYCONST.C                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:  Game devices management driver constants              */
/*                                                                            */
/*   FUNCTION: This module allocates the global constants (strings) for the   */
/*             Game devices management driver.                                */
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
/*          01/01/12  MM                                                      */
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

const char gDDName[]      = "GENGAME.SYS";
const char gDDDesc[]      = "Game devices management driver";
const char gVendorID[]    = "Markus Montkowski for Netl@bs";
const char gAdapterName[] = "Gamedevice Manager";

