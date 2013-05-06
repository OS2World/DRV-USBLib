#define INCL_DOSERRORS
#define INCL_DOSMEMMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSPROCESS
#include <OS2.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>

#define LOGBUFSIZE 4096
void main(void)
{
  ULONG ulAction;
  APIRET rc;
  HFILE hDrv;
  FILE* fLog;
  char szBuffer[4096], *pBuf;
  ULONG ulRead,ulCnt;
  rc = DosOpen( "SYSLOG$ ",
                &hDrv,
                &ulAction,
                0,
                FILE_NORMAL,
                OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_ACCESS_READONLY |
                OPEN_FLAGS_NOINHERIT |
                OPEN_SHARE_DENYNONE,
                0 );
  if(!rc)
  {
//    printf( "syslog driver opened Press a key to start loging and any other key to end\r\n" );
//    _getch();
    printf( "Started...\r\n" );
    ulCnt = 120;

    while (!_kbhit() && ulCnt)
    {
       ulRead = 0;
       DosRead( hDrv, szBuffer, sizeof(szBuffer)-1, &ulRead);
       szBuffer[LOGBUFSIZE -1 ]=0x00;
       if( ulRead )
       {
         fLog = fopen("syslog.txt", "a" );

         for( pBuf = szBuffer;pBuf +(strlen(pBuf)+1) < &szBuffer[ulRead];pBuf += strlen(pBuf)+1 )
         {
           printf( "%s\r\n", pBuf );
           if(fLog != NULL)
             fprintf( fLog, "%s\r\n", pBuf );
         }
         fflush( fLog );
         fclose( fLog );
       }
       else
       {
         DosSleep( 1000 );
         ulCnt--;
       }
    }
    DosClose( hDrv );
    printf( "\r\nFinished...\r\n" );
  }
  else
    printf( "faild to open syslog driver\r\n" );
}

