#/* $ID$ */

# Copyright 2000-2002 Markus Montkowski                                      
# This code is released under the Aladdin Free Public License                
#                    (Version 9, September 18, 2000)                         
#
#/***********************************************************************/
#/*                                                                     */
#/* Driver Name: USBRESMG.SYS                                           */
#/*                                                                     */
#/* Source File Name: MAKEFILE                                          */
#/*                                                                     */
#/* Descriptive Name: MAKEFILE for the USB Resource Manager Driver      */
#/*                                                                     */
#/* Function:                                                           */
#/*                                                                     */
#/*---------------------------------------------------------------------*/
#/*                                                                     */
#/* Change Log                                                          */
#/*                                                                     */
#/* Mark    Date      Programmer  Comment                               */
#/* ----    --------  ----------  -------                               */
#/*         00/08/14  MM                                                */
#/*                                                                     */
#/*                                                                     */
#/***********************************************************************/

#
#       This makefile creates the OS/2 USB Resource Manager Driver
#
#       You can optionally generate the listing files for the device driver.
#
#          make  [option]
#
#            option:     list     -> create listings
#                        usbprt.sys -> create device driver
#
#            default:  create usbresmg.sys
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
USBLIB  = ..\misc

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

AFLAGS = -L -MX -T -Z $(ENV)
AINC   = -I. -I$(DOSINC) -I$(INIT) -I$(TASK) -I$(INC)
CINC   = -I$(H) -I$(USBH) -I$(DISKH) -I$(MAKEDIR)
# environment variable DEBUG is used to switch between release and debug builds.
# use SET DEBUG=1 to build debug version of module.
!ifndef DEBUG
CFLAGS = /c /Zp /G2s /Answ /W0 /Fc /nologo /Osegli /Od
!else
CFLAGS = /c /Zp /G2s /Answ /W0 /Fc /nologo /Osegli /DDEBUG /Zi /Od
!endif
!ifndef DEBUG
LFLAGS = /map /nod /packd /a:16 /far
!else
LFLAGS = /map /nod /packd /a:16 /far /DEBUG /CO
!endif
RFLAGS = -r

LIBS  =  $(USBLIB)\usbmisc.lib $(DHLIB)\dhcalls.lib $(RMLIB)\rmcalls.lib $(LIB)\slibcep.lib
DEF   = usbres.def

#****************************************************************************
# Set up Macros that will contain all the different dependencies for the
# executables and dlls etc. that are generated.
#****************************************************************************

#
#       OBJ Files
#
OBJ1 = ressegs.obj
OBJ2 = resstrat.obj resioctl.obj
OBJ3 = residc.obj resirq.obj resreq.obj
OBJ4 = resdata.obj resconst.obj resinit.obj

OBJS = $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4)

#
#       LIST Files
#
LIST=   ressegs.lst resdata.lst resirq.lst resreq.lst                      \
        resstr1.lst resqueue.lst resioctl.lst residc.lst resmisc.lst       \
        dprintf.lst resinit.lst resconst.lst resutil.lst

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

!include      "version.mak"

#
# Should be the default tag for all general processing
#

all:    usbresmg.sys

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
        if exist *.cod  del *.cod

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

usbresmg.sys:  $(OBJS) $(LIBS)  makefile
        Rem Create DEF file <<$(DEF)
LIBRARY USBRESMG
DESCRIPTION '$(FILEVER)  OS/2 USB Resource Manager'
PROTMODE

SEGMENTS
 DDHEADER       CLASS 'DDHEADER'
 CONST          CLASS 'CONST'
 _BSS           CLASS 'BSS'
 _DATA          CLASS 'DATA'
 RMCode         CLASS 'CODE' IOPL
 MSCode         CLASS 'CODE' IOPL
 'Code'         CLASS 'CODE' IOPL
 _TEXT          CLASS 'CODE' IOPL
IMPORTS
 _DOSIODELAYCNT = DOSCALLS.427

<<keep
        $(LINK) $(LFLAGS) @<<$(@B).lnk
$(OBJ1) +
$(OBJ2) +
$(OBJ3) +
$(OBJ4)
$*.sys
$*.map
$(LIBS)
$(DEF)
<<keep
             $(MAPSYM) $*.map

#****************************************************************************
