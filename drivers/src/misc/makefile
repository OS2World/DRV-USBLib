#/* SCCSID = src/dev/usb/MISC/MAKEFILE, usb, c.basedd 98/07/10
#/*
#*   Licensed Material -- Property of IBM
#*
#*   (c) Copyright IBM Corp. 1997, 1998  All Rights Reserved
#*/
#/***********************************************************************/
#/*                                                                     */
#/* Driver Name: USBMISC.LIB                                            */
#/*                                                                     */
#/* Source File Name: MAKEFILE                                          */
#/*                                                                     */
#/* Descriptive Name: MAKEFILE for the USB debug &                      */
#/*                              miscelenious routine library.          */
#/*                                                                     */
#/* Function:                                                           */
#/*                                                                     */
#/*---------------------------------------------------------------------*/
#/*                                                                     */
#/* Copyright (C) 1992 IBM Corporation                                  */
#/*                                                                     */
#/* DISCLAIMER OF WARRANTIES.  The following [enclosed] code is         */
#/* provided to you solely for the purpose of assisting you in          */
#/* the development of your applications. The code is provided          */
#/* "AS IS", without warranty of any kind. IBM shall not be liable      */
#/* for any damages arising out of your use of this code, even if       */
#/* they have been advised of the possibility of such damages.          */
#/*                                                                     */
#/*---------------------------------------------------------------------*/
#/*                                                                     */
#/* Change Log                                                          */
#/*                                                                     */
#/* Mark    Date      Programmer  Comment                               */
#/* ----    ----      ----------  -------                               */
#/* @nnnn   mm/dd/yy  NNN                                               */
#/*                                                                     */
#/*                                                                     */
#/***********************************************************************/

#
#       This makefile creates the OS/2 USB debug & misc libary
#       You can optionally generate the listing files for the device driver.
#
#          make  [option]
#
#            option:     list     -> create listings
#                        usbmisc.lib -> create library
#            default:  create usbmisc.lib
#
# ******  NOTE  ******
#
#        If you are using a SED command with TAB characters, many editors
#        will expand tabs causing unpredictable results in other programs.
#
#        Documentation:
#
#        Using SED command with TABS. Besure to invoke set tab save option
#        on your editor. If you don't, the program 'xyz' will not work
#        correctly.
#

#****************************************************************************
#  Dot directive definition area (usually just suffixes)
#****************************************************************************

.SUFFIXES:
.SUFFIXES: .com .sys .exe .obj .mbj .asm .inc .def .lnk .lrf .crf .ref
.SUFFIXES: .lst .sym .map .c .h .lib

#****************************************************************************
#  Environment Setup for the component(s).
#****************************************************************************

#
# Conditional Setup Area and User Defined Macros
#

#
# Compiler Location w/ includes, libs and tools
#

DOSINC = ..\..\..\dos\dosinc
INIT   = ..\..\..\dos\init
TASK   = ..\..\..\dos\task
INC    = ..\..\..\..\inc
H      = ..\..\..\..\h
USBH   = ..\include
LIB    = ..\..\..\..\lib
TOOLSPATH = ..\..\..\..\tools
DISKH  = ..\..\dasd\diskh
RMH    = ..\..\resource\rsm_h
DHLIB  = ..\..\dasd\devhelp
RMLIB  = ..\..\resource\rmcalls

#
# Since the compiler/linker and other tools use environment
# variables ( INCLUDE, LIB, etc ) in order to get the location of files,
# the following line will check the environment for the LIFE of the
# makefile and will be specific to this set of instructions. All MAKEFILES
# are requested to use this format to insure that they are using the correct
# level of files and tools.
#

!if [set INCLUDE=$(DOSINC);$(INIT);$(TASK);$(INC);$(RMH)] || \
   [set LIB=$(LIB);$(DHLIB)] || [set PATH=$(TOOLSPATH);$(DK_TOOLS)]
!endif


#
# Compiler/tools Macros
#

AS=masm
CC=cl
IMPLIB=implib
IPF=ipfc
LIBUTIL=lib
LINK=link
MAPSYM=mapsym
RC=rc

#
# Compiler and Linker Options
#

AFLAGS = -MX -T -Z $(ENV)
AINC   = -I. -I$(DOSINC) -I$(INIT) -I$(TASK) -I$(INC)
CINC   = -I$(H) -I$(USBH) -I$(DISKH) -I$(MAKEDIR)
# environment variable DEBUG is used to switch between release and debug builds.
# use SET DEBUG=1 to build debug version of module.
!ifndef DEBUG
CFLAGS = /c /Zp /G2s /Answ /W3 /nologo /Osegli
!else
CFLAGS = /c /Zp /G2s /Answ /W3 /Fc /nologo /Osegli /DDEBUG /Zi /Od
!endif
LFLAGS = usbmisc.lib /nologo
LIBS  =  $(DHLIB)\dhcalls.lib $(RMLIB)\rmcalls.lib $(LIB)\slibcep.lib

#****************************************************************************
# Set up Macros that will contain all the different dependencies for the
# executables and dlls etc. that are generated.
#****************************************************************************

#
#       OBJ Files
#
OBJ1 =  dprintf.obj
OBJ2 =  usbdebug.obj
OBJ3 =  usbmisc.obj
OBJ4 =  usbcidc.obj
OBJ5 =  initcmm.obj
OBJ6 =  usbcmn.obj
OBJ7 =  usbreprt.obj
OBJ8 =  memutil.obj
OBJ9 =  usblog.obj

OBJS = $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7) $(OBJ8) $(OBJ9)
OBJL = -+$(OBJ1) -+$(OBJ2) -+$(OBJ3) -+$(OBJ4) -+$(OBJ5) -+$(OBJ6) -+$(OBJ7) -+$(OBJ8) -+ $(OBJ9)
#
#       LIST Files
#
LIST=   dprintf.lst usdebug.lst                                            \
        usbmisc.lst initcmm.lst usbcmn.lst usbreprt.lst usblog.lst

#****************************************************************************
#   Setup the inference rules for compiling and assembling source code to
#   object code.
#****************************************************************************


.asm.obj:
        $(AS) $(AFLAGS) $(AINC) $*.asm;

.asm.mbj:
        $(AS) $(AFLAGS) -DMMIOPH $(AINC) $*.asm $*.mbj;

.asm.lst:
        $(AS) -l -n $(AFLAGS) $(AINC) $*.asm;

.c.obj:
        $(CC) $(CFLAGS) $(CINC) $*.c

.c.lst:
        $(CC) $(CFLAGS) /Fc $(CINC) $*.c
        copy $*.cod $*.lst
        del $*.cod

#usb.msg:
#        mkmsgf usb.txt usb.msg

#****************************************************************************
#   Target Information
#****************************************************************************
#
# This is a very important step. The following small amount of code MUST
# NOT be removed from the program. The following directive will do
# dependency checking every time this component is built UNLESS the
# following is performed:
#                    A specific tag is used -- ie. all
#
# This allows the developer as well as the B & I group to perform incremental
# build with a degree of accuracy that has not been used before.
# There are some instances where certain types of INCLUDE files must be
# created first. This type of format will allow the developer to require
# that file to be created first. In order to achive that, all that has to
# be done is to make the DEPEND.MAK tag have your required target. Below is
# an example:
#
#    depend.mak:   { your file(s) } dephold
#
# Please DON'T remove the following line
#

#!include      "$(H)\version.mak"

#
# Should be the default tag for all general processing
#

all: usbmisc.lib
list: $(LIST)

clean:
        if exist *.lnk  del *.lnk
        if exist *.obj  del *.obj
        if exist *.mbj  del *.mbj
        if exist *.map  del *.map
        if exist *.old  del *.old
        if exist *.lst  del *.lst
        if exist *.lsd  del *.lsd
        if exist *.sym  del *.sym
        if exist *.sys  del *.sys
        if exist *.dmd  del *.dmd
        if exist *.tff  del *.tff
        if exist usbmisc.lib  del usbmisc.lib
#*****************************************************************************
#   Specific Description Block Information
#*****************************************************************************

# This section would only be for specific direction as to how to create
# unique elements that are necessary to the build process. This could
# be compiling or assembling, creation of DEF files and other unique
# files.
# If all compiler and assembly rules are the same, use an inference rule to
# perform the compilation.
#

usbmisc.lib: $(OBJS) $(LIBS)  makefile
        $(LIBUTIL) $(LFLAGS) @<<$(@B).lnk
$(OBJL),usbmisc.map,usbmisc.lib
<<keep

#             $(MAPSYM) $*.map

#****************************************************************************
