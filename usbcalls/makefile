#
# makefile:
#       makefile for USBCALLS DLL
#       For use with IBM NMAKE, which comes with the IBM compilers,
#       the Developer's Toolkit, and the DDK.
#
#       This compiles the usbcalls DLL Ring3 interface for USB devices
#
#       Notes:
#
#       1)
#
#
#       Input:          ./*.cpp
#
#       Output:         ./DLL/*.obj
#
#

# COMPILER OPTIONS
# ----------------

TKBASE=f:\toolkit
VACBASE=f:\IBMCPP

# Compiler macro. This is used for each .OBJ file to be created.
# If the XWP_DEBUG environment variable was set to anything, we
# will use debugging compiler options.
# XWP_DEBUG is set by MAKE.CMD or above.
#
# No need to change this. Change the variables above instead.

CC = error_in_makefile_do_not_use_CC_directly

# CC_WARNINGS  = /w3 /Wcmp+ /Wcnd- /Wcns+ /Wcnv+ /Wgen /Wcns /Wcpy /Wobs /Word- /Wpro /Wrea /Wret /Wtru
CC_WARNINGS  = /w3 /Wcmp+ /Wcnd- /Wcns+ /Wcnv+ /Wgen /Wcns /Wcpy /Wobs /Word- /Wpro /Wrea /Wret /Wtru
# /Wuse+ /Wuni+

# compiler options which are the same with debug and release code
CC_SHARED    = /c /gd- /gi+ /q /se /ss+ /g5

!ifdef DEBUG
# debug code
CC_TEMP      = $(CC_SHARED) /o- /oi- /ol- /ti+ /D__DEBUG__=1
# /tm+
!else
# release code
CC_TEMP      = $(CC_SHARED) /o+ /oi- /ol- /tn+
!endif

# DLL code (multi-threaded)
CC_DLL_MT = $(CC_TEMP) /ge- /gm+
#$(CC_WARNINGS)
# DLL code (single-threaded, subsystem libraries)
CC_DLL_SUBSYS = $(CC_TEMP) /ge- /rn $(CC_WARNINGS)
# EXE code (multi-threaded)
CC_EXE_MT = $(CC_TEMP) /ge+ /gm+ $(CC_WARNINGS)
# EXE code (single-threaded)
CC_EXE_ST = $(CC_TEMP) /ge+ /gm- $(CC_WARNINGS)

# Some VisualAge C++ compiler options explained [default in brackets]:
# /c:   compile only, no link (we'll call the linker explicitly)
# /fi+: precompile header files                         [/fe-]
# /g3|4|5: optimize for 386/486/Pentium                 [/g3]
# /gd-: link runtime statically                         [/gd-]
# /ge-: create DLL code                                 [/ge+]
#           This switches between EXE and DLL initialization code
#           for the .OBJ file. /ge+ is only needed when the object
#           file contains a main() function. For libraries to be
#           used either with EXE's or DLL's, use /ge+.
# /gh+: generate profiling hooks for VAC profiler
# /gi+: fast integer execution
# /Gl+: remove unreferenced functions (when comp.+link in 1 step)
# /gm+: multithread libraries
# /gm+: disable stack probes (single-thread only!)
# /kc+: produce preprocessor warnings
# /o+:  optimization (inlining etc.)
# /oi-: no inlining (?)
# /ol+: use intermediate linker; do _not_ use with debug code
# /q+:  suppress icc logo
# /Re : don't use subsystem libraries (!= Rn)
# /se:  all language extensions
# /si+: allow use of precompiled header files
# /ss:  allow double slashes comments in C too
# /ti+: debug code
# /tdp: compile everything as C++, even if it has a .C suffix
# /tm:  use debug memory management (malloc etc.)
# /tn:  add source line numbers to object files (for mapfile); a subset of /ti+
# /Wcls: class problems
# /Wcnd: conditional exprs problems (= / == etc.)
# /Wcmp: possible unsigned comparison redundancies
# /Wcns: operations involving constants
# /Wcnv: conversions
# /Wcpy: copy constructor problems
# /Weff: statements with no effect (annoying)
# /Wgen: generic debugging msgs
# /Wobs: obsolete features
# /Word: unspecified evaluation order
# /Wpar: list non-referenced parameters (annoying)
# /Wppc: list possible preprocessor problems (.h dependencies)
# /Wpro: warn if funcs have not been prototyped
# /Wrea: mark code that cannot be reached
# /Wret: check consistency of return levels
# /Wuni: uninitialized variables
# /Wuse: unused variables
# /w2:   produce error and warning messages, but no infos


# LINK OPTIONS
# ------------
#
# Link macro. This is used for final linking.
# If we're supposed to do the release version, we'll turn
# off debugging and optimize and pack the resulting files.
#
# No need to change this. Change the variables above instead.

LIBDIRS = $(TKBASE)\lib\ $(TKBASE)\SOM\LIB\ "$(VACBASE)\lib\"

LINK_BASE = ilink /nologo /noe /map /linenumbers $(LIBDIRS)

!ifdef DEBUG
LINK =  $(LINK_BASE) /debug
LINK_ALWAYSPACK = $(LINK_BASE) /exepack:2
!else
LINK =  $(LINK_BASE) /exepack:2
LINK_ALWAYSPACK = $(LINK)
#/packcode /packdata
# /optfunc
!endif

# Some LINK386 cmd line options
# (Visual Age ILINK understands these too if /nofree is specified):
# /align:  executable pages align factor (in bytes)
# /noe:    no extended dictionary. Required for replacing _DLL_InitTerm.
#          Can slightly increase executable's size.
# /noi:    no ignore case (necc. for C)
# /map:    list public symbols (create MAP file); we always need this,
#          because we produce a .SYM file later.
# /linenumbers: include linenumbers in MAP file
# /nod:    no default library search (explicitly specify libs)
# /nol:    no logo (link386 startup)
# /packcode: group neighboring code segments (enabled per default)
# /packdata: group neighboring data segments (disabled per default)
# /debug:  include debug code
# /optfunc: optimize code arrangement (takes some extra time)
# /exepack:2 use Warp 3 executable compression


# RESOURCE COMPILER
# -----------------

RC      = rc -p -x2 -n -i $(TKBASE)\h -i $(PROJECT_BASE_DIR)\include

# RC options:
# -p       pack (64K boundaries)
# -x2      Warp 3 compression
# -n       no logo




# FIXED MACROS
# ------------
#
# You probably need not change the following.
#

# Define the suffixes for files which NMAKE will work on.
# .SUFFIXES is a reserved NMAKE keyword ("pseudotarget") for
# defining file extensions that NMAKE will recognize in inference
# rules.

.SUFFIXES: .cpp .obj .dll .h .ih .rc .res

# OUTPUTDIR specifies the directory where all the output .OBJ
# 1) all build files are in a common dir tree and the entire
#    tree can be removed completely;
# 2) the build tree can be on a different physical drive for
#    speed.
OUTPUTDIR = .\DLL
!if [@echo       OUTPUTDIR is $(OUTPUTDIR)]
!endif

# create output directory
!if [@md $(OUTPUTDIR) 2> NUL]
!endif

{.\}.cpp{$(OUTPUTDIR)\}.obj:
    icc $(CC_DLL_MT) -Fo$(OUTPUTDIR)\$(@B).obj -c    $<




# The OBJS macro contains all the .OBJ files which need to be
# created from the files in this directory.
# These will be put into BIN\.
# If you add a new source, add the corresponding .OBJ file here.
OBJS = \
$(OUTPUTDIR)\usbcalls.obj \
$(OUTPUTDIR)\usbrexx.obj

#    $(OUTPUTDIR)\usblink.obj
#    $(OUTPUTDIR)\testio.obj
#    $(OUTPUTDIR)\testide2.obj
#    $(OUTPUTDIR)\testbulk.obj



$(OUTPUTDIR)\USBCALLS.DLL: $(OBJS)
#all:
  $(LINK) /DLL $(OUTPUTDIR)\usbcalls.obj $(OUTPUTDIR)\usbrexx.obj \
   $(TKBASE)\lib\REXX.LIB \
    usbcalls.def /O:$(OUTPUTDIR)\usbcalls.dll
  IMPLIB .\LIB\USBCALLS.LIB .\DLL\USBCALLS.DLL




