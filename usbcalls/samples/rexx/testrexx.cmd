/*******************************************/
/* Test the REXX interface in USBCALLS.DLL */
/*******************************************/
call RxFuncAdd 'SysLoadFuncs', 'REXXUTIL', 'SysLoadFuncs'
call SysLoadFuncs
Say 'SysLoadFuncs is complete.'

call RxFuncAdd 'UsbLoadFuncs' , 'USBCALLS', 'USBLOADFUNCS'
call UsbLoadFuncs;
Say 'SysLoadFuncs is complete.'

Say 'Getting Number of USB devices in the System'

 if(RxUsbQueryNumberDevices(NumDevices)=0)Then
   Say NumDevices' USB devices attached to the system'
 else
   Say 'Error ' result ' while quering number of devices'


say 'TESTREXX demonstration program is complete.'
call SysPause 'Press Enter key to exit'
exit


SysPause:
parse arg prompt
  if prompt='' then
    prompt='Press Enter key when ready . . .'
  call SysSay prompt
  Pull .
  say
return



SysSay:
parse arg string
  call charout 'STDOUT', string
return
