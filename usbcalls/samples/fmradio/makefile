# makefile
# Erstellt von IBM WorkFrame/2 MakeMake um 0:57:51 am 21 Okt 2001
#
# Diese Make-Datei enth„lt folgende Aktionen:
#  Compile::C++ Compiler
#  Link::Linker
#  Bind::Resource Bind

.SUFFIXES:

.SUFFIXES: \
    .cpp .obj

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /IJ:\netlabs\xworkplace\include;J:\netlabs\xwphelpers\include /Ti /Gm /Ge- /C %s

{F:\DDK\BASE\SRC\DEV\usb\USBRES\usbcalls\samples\fmradio}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /IJ:\netlabs\xworkplace\include;J:\netlabs\xwphelpers\include /Ti /Gm /Ge- /C %s

all: \
    .\widget.dll

.\widget.dll: \
    .\widget.obj \
    F:\DDK\BASE\SRC\DEV\usb\USBRES\usbcalls\samples\fmradio\widget.RES
    @echo " Link::Linker "
    @echo " Bind::Resource Bind "
    icc.exe @<<
     /B" /de "
     /Fewidget.dll
     .\widget.obj
    .\widget.def
<<
    rc.exe F:\DDK\BASE\SRC\DEV\usb\USBRES\usbcalls\samples\fmradio\widget.RES widget.dll

.\widget.obj: \
    F:\DDK\BASE\SRC\DEV\usb\USBRES\usbcalls\samples\fmradio\widget.cpp \
    {F:\DDK\BASE\SRC\DEV\usb\USBRES\usbcalls\samples\fmradio;J:\netlabs\xworkplace\include;J:\netlabs\xwphelpers\include;$(INCLUDE);}setup.h \
    {F:\DDK\BASE\SRC\DEV\usb\USBRES\usbcalls\samples\fmradio;J:\netlabs\xworkplace\include;J:\netlabs\xwphelpers\include;$(INCLUDE);}helpers\dosh.h \
    {F:\DDK\BASE\SRC\DEV\usb\USBRES\usbcalls\samples\fmradio;J:\netlabs\xworkplace\include;J:\netlabs\xwphelpers\include;$(INCLUDE);}helpers\gpih.h \
    {F:\DDK\BASE\SRC\DEV\usb\USBRES\usbcalls\samples\fmradio;J:\netlabs\xworkplace\include;J:\netlabs\xwphelpers\include;$(INCLUDE);}helpers\prfh.h \
    {F:\DDK\BASE\SRC\DEV\usb\USBRES\usbcalls\samples\fmradio;J:\netlabs\xworkplace\include;J:\netlabs\xwphelpers\include;$(INCLUDE);}helpers\winh.h \
    {F:\DDK\BASE\SRC\DEV\usb\USBRES\usbcalls\samples\fmradio;J:\netlabs\xworkplace\include;J:\netlabs\xwphelpers\include;$(INCLUDE);}helpers\xstring.h \
    {F:\DDK\BASE\SRC\DEV\usb\USBRES\usbcalls\samples\fmradio;J:\netlabs\xworkplace\include;J:\netlabs\xwphelpers\include;$(INCLUDE);}shared\center.h \
    {F:\DDK\BASE\SRC\DEV\usb\USBRES\usbcalls\samples\fmradio;J:\netlabs\xworkplace\include;J:\netlabs\xwphelpers\include;$(INCLUDE);}shared\common.h \
    {F:\DDK\BASE\SRC\DEV\usb\USBRES\usbcalls\samples\fmradio;J:\netlabs\xworkplace\include;J:\netlabs\xwphelpers\include;$(INCLUDE);}widget.h \
    ..\..\usbcalls.h \
    {F:\DDK\BASE\SRC\DEV\usb\USBRES\usbcalls\samples\fmradio;J:\netlabs\xworkplace\include;J:\netlabs\xwphelpers\include;$(INCLUDE);}helpers\pmprintf.h \
    {F:\DDK\BASE\SRC\DEV\usb\USBRES\usbcalls\samples\fmradio;J:\netlabs\xworkplace\include;J:\netlabs\xwphelpers\include;$(INCLUDE);}helpers\memdebug.h
