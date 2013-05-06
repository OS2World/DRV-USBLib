/************************** START OF SPECIFICATIONS ***************************/
/*                                                                            */
/*   SOURCE FILE NAME:  COMPROTO.H                                            */
/*                                                                            */
/*   DESCRIPTIVE NAME:  USB Log Driver                                        */
/*                      function prototypes                                   */
/*                                                                            */
/*   FUNCTION: This module is the USB Log Driver                              */
/*             function prototype include file.                               */
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
/*                                                                            */
/**************************** END OF SPECIFICATIONS ***************************/

// TSTSTRAT.C
void FAR LogStrategy (void);
void     CmdError (PRPH pRP);
void     LogRead (PRPH pRP);
void     LogInStatus (PRPH pRP);
void     LogInFlush (PRPH pRP);
void     LogOpen (PRPH pRP);
void     LogClose (PRPH pRP);
void     LogInitComplete (PRPH pRP);

// TSTDATA.C
void (*gStratList[]) (PRPH pRP);

// TSTIDC.C
void FAR LogIDC (PLOG_LINE pLine);

void LogInfoInit (void);

// TSTINIT.C
void LogInit (RPH FAR *pRP);

