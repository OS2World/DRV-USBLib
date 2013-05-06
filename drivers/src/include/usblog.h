

typedef CHAR FAR *PFARCHAR;

typedef struct
{
  USHORT usMajor;
  USHORT usMinor;
  char FAR * pszString;
}LOG_LINE, *NPLOG_LINE, FAR *PLOG_LINE;

typedef struct _MDDATA
{
  USHORT usLogState; // 0 = Not Init, 1 = Init logging 2= InitDone.
  IDCTABLE IdcSYSLOG;
  char  szLogDriver[9];
  char  szLogLine[400];
  LOG_LINE LogLine;
}MDDATA, FAR * PMDDATA;

void LoggingSetInitComplete(MDDATA *pLogging);
BOOL InitLogging(MDDATA *pLogging, USHORT usLevel);

void LogBuffer( MDDATA *pLogging,
                USHORT usLevel,
                const char *pszBufferName,
                const char far *pBuffer,
                USHORT usBufferLength );

void LogString( MDDATA *pLogging,
                USHORT usLevel,
                const char *fmtStr, ...);


#define LOGLEVEL_OFF       0
#define LOGLEVEL_ALL       1
#define LOGLEVEL_HLVLFLOW    2
#define LOGLEVEL_IRQFLOW   3
#define LOGLEVEL_DETAILED  4
#define LOGLEVEL_SPECIFIC  5
#define LOGLEVEL_CRITICAL  6

