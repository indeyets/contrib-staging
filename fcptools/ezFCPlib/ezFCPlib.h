/*
  This code is part of FCPTools - an FCP-based client library for Freenet
	
  Designed and implemented by David McNab <david@rebirthing.co.nz>
  CopyLeft (c) 2001 by David McNab
	
	Currently maintained by Jay Oliveri <ilnero@gmx.net>
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _EZFCPLIB_H
#define _EZFCPLIB_H 

/* Yes? */
#define _GNU_SOURCE

/* Generic <sys/> includes here so they are first. */
#include <sys/types.h>
#include <sys/stat.h>

/**************************************************************************
  MS-WINDOWS specifics
**************************************************************************/
#ifdef WINDOWS
#include <malloc.h>
#include <process.h>
#include <winsock.h>
#include <io.h>

/* VERSION is defined by automake for non-Win platforms. */
#define VERSION "0.4.8"

#define write _write
#define open _open
#define read _read
#define close _close
#define mkstemp _mkstemp
#define snprintf _snprintf
#define vsnprintf _vsnprintf

#define strcasecmp strcmpi
#define strncasecmp strnicmp

#define S_IREAD _S_IREAD
#define S_IWRITE _S_IWRITE


/**************************************************************************
  UNIX specifics
**************************************************************************/
#else

#include <sys/socket.h>

/* UNIX includes that do not correspond on WINDOWS go here */
/* Keep 'sys' files first in include order */

#include <unistd.h>

#define OPEN_PERMS (S_IRUSR | S_IWUSR)

#endif

/**************************************************************************
  GENERIC (place anything that must happen after the above decl's here)
**************************************************************************/
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

/*************************************************************************/

/*
  Threshold levels for the user-provided fcpLogCallback() function
  fcpLogCallback will be called with a verbosity argument, which will
  be one of these values. This allows the client program to screen log
  messages according to importance.

	Let's start at 1, so that Verbosity=0 is *really* silent :)
*/
#define FCP_LOG_CRITICAL      1
#define FCP_LOG_NORMAL        2
#define FCP_LOG_VERBOSE       3
#define FCP_LOG_DEBUG         4
#define FCP_LOG_MESSAGE_SIZE  4096   /* Was 65K */

/*
  Lengths of allocated strings/arrays.
*/
#define L_BLOCK_SIZE        262144    /* default split part size (256*1024) */
#define L_FILE_BLOCKSIZE    8192
#define L_RESPONSE_BUFFER   2048
#define L_ERROR_MESSAGE     256
#define L_KEY               40

#define FCP_MAX_SPLIT_THREADS  8

#define CHUNK_STAT_LOCAL    0  /* not in freenet; local datastore */
#define CHUNK_STAT_QUEUED   1  /* waiting for mgr thread to pick up */
#define CHUNK_STAT_INPROG   2  /* in progress */
#define CHUNK_STAT_MANIFEST 4  /* inserting splitfile manifest */
#define CHUNK_STAT_SUCCESS  5  /* full insert completed successfully */
#define CHUNK_STAT_ERROR    3  /* failure - awaiting cleanup */
#define CHUNK_STAT_FAILED   6  /* insert failed somewhere */

#define KEY_TYPE_SSK  1
#define KEY_TYPE_CHK  2
#define KEY_TYPE_KSK  3

#define META_TYPE_DEFAULT   1
#define META_TYPE_REDIRECT  2
#define META_TYPE_DBR       3
#define META_TYPE_INFO      4
#define META_TYPE_EXTINFO   5
#define META_TYPE_04        6

/*
  cdoc type fields
*/
#define META_TYPE_04_REDIR  'r'
#define META_TYPE_04_DBR    'd'
#define META_TYPE_04_SPLIT  's'
#define META_TYPE_04_NONE   'n'


/*
  General FCP definitions
*/
#define EZFCP_DEFAULT_HOST       "127.0.0.1"
#define EZFCP_DEFAULT_PORT       8481
#define EZFCP_DEFAULT_HTL        3
#define EZFCP_DEFAULT_REGRESS    3
#define EZFCP_DEFAULT_RAWMODE    0
#define EZFCP_DEFAULT_VERBOSITY  FCP_LOG_NORMAL

/*
  flags for fcpOpenKey()
*/
#define _FCP_O_READ         0x100
#define _FCP_O_WRITE        0x200
#define _FCP_O_RAW          0x400   /* disable automatic metadata handling */


/***********************************************************************
	Connection handling structgures and definitions.
*/
typedef struct {
  int   protocol;  /* Protocol=<number: protocol version number.  Currently 1> */
  char *node;      /* Node=<string: freeform: Description of the nodes> */
} FCPRESP_NODEHELLO;

typedef struct {
	char *uri; /* URI=<string: fully specified URI, such as freenet:KSK@gpl.txt> */

	char  publickey[L_KEY+1];  /* PublicKey=<string: public Freenet key> */
	char  privatekey[L_KEY+1]; /* PrivateKey=<string: private Freenet key> */
} FCPRESP_SUCCESS;

typedef struct {
  int datalength;
  int metadatalength;
} FCPRESP_DATAFOUND;

typedef struct {
  int    length;  /* Length=<number: number of bytes in trailing field> */
  char  *data;    /* MetadataLength=<number: default=0: number of bytes of	metadata> */
} FCPRESP_DATACHUNK;

typedef struct {
	char *uri; /* URI=<string: fully specified URI, such as freenet:KSK@gpl.txt> */

	char  publickey[L_KEY+1];  /* PublicKey=<string: public Freenet key> */
	char  privatekey[L_KEY+1]; /* PrivateKey=<string: private Freenet key> */
} FCPRESP_KEYCOLLISION;

typedef struct {
	char *uri; /* URI=<string: fully specified URI, such as freenet:KSK@gpl.txt> */

	char  publickey[L_KEY+1];  /* PublicKey=<string: public Freenet key> */
	char  privatekey[L_KEY+1]; /* PrivateKey=<string: private Freenet key> */
} FCPRESP_PENDING;

typedef struct {
  char *reason;   /* [Reason=<descriptive string>] */
} FCPRESP_FAILED;

typedef struct {
  char *reason;   /* [Reason=<descriptive string>] */
} FCPRESP_FORMATERROR;

typedef struct {
	char  fec_algorithm[L_KEY+1];
 
	int   filelength;
	int   offset;
	int   block_count;
	int   block_size;
	int   checkblock_count;
	int   checkblock_size;
	int   segments;
	int   segment_num;
	int   blocks_required;
} FCPRESP_SEGMENTHEADER;

typedef struct {
	int     block_count;
	char  **blocks;
	int     checkblock_count;
	char  **checkblocks;
} FCPRESP_BLOCKMAP;

typedef struct {
	int   block_count;
	int   block_size;
} FCPRESP_BLOCKSENCODED;

typedef struct {
	int   block_count;
	int   block_size;
	int   data_length;
} FCPRESP_BLOCKSDECODED;

typedef struct {
  int datalength;
} FCPRESP_MADEMETADATA;


/**********************************************************************
  Now bundle all these together.
*/
typedef struct {
  int type;

	FCPRESP_SUCCESS         success;
	FCPRESP_DATAFOUND       datafound;
	FCPRESP_DATACHUNK       datachunk;
	FCPRESP_KEYCOLLISION    keycollision;
	FCPRESP_PENDING         pending;
	FCPRESP_FAILED          failed;
	FCPRESP_FORMATERROR     formaterror;

	/* FEC specific responses */
	FCPRESP_SEGMENTHEADER   segmentheader;
	FCPRESP_BLOCKMAP        blockmap;
	FCPRESP_BLOCKSENCODED   blocksencoded;
	FCPRESP_BLOCKSDECODED   blocksdecoded;
	FCPRESP_MADEMETADATA    mademetadata;

} FCPRESP;


/*
	Tokens for response types.
*/
#define FCPRESP_TYPE_NODEHELLO      1
#define FCPRESP_TYPE_SUCCESS        2
#define FCPRESP_TYPE_DATAFOUND      3
#define FCPRESP_TYPE_DATACHUNK      4
#define FCPRESP_TYPE_DATANOTFOUND   5
#define FCPRESP_TYPE_ROUTENOTFOUND  6
#define FCPRESP_TYPE_URIERROR       7
#define FCPRESP_TYPE_RESTARTED      8
#define FCPRESP_TYPE_KEYCOLLISION   9
#define FCPRESP_TYPE_PENDING        10
#define FCPRESP_TYPE_FAILED         11
#define FCPRESP_TYPE_FORMATERROR    12

#define FCPRESP_TYPE_SEGMENTHEADER  13
#define FCPRESP_TYPE_BLOCKMAP       14
#define FCPRESP_TYPE_BLOCKSENCODED  15
#define FCPRESP_TYPE_BLOCKSDECODED  16
#define FCPRESP_TYPE_MADEMETADATA   17

/* Tokens for receive states */
#define RECV_STATE_WAITING      0
#define RECV_STATE_GOTHEADER    1


/**********************************************************************
  Freenet Client Protocol Handle Definition Section :)
*/
typedef struct {
  int    type;

	char  *uri_str;
  char  *keyid;
	char  *path;
	char  *file;
} hURI;


typedef struct {
	char  *filename;  /* null terminated filename */
	FILE  *file;      /* stream pointer */
	int    fd;        /* corresponding file descriptor */

	int    fn_status; /* status relative to Freenet */
	int    size;      /* size of this chunk */

	hURI  *uri;       /* this block's CHK */

} hBlock;


typedef struct {
	char  *header_str;

	int    filelength;
	long   offset;
	int    block_count;
	int    block_size;
	int    checkblock_count;
	int    checkblock_size;
	int    segments;
	int    segment_num;
	int    blocks_required;

	int      db_count;
	hBlock **data_blocks;

	int      cb_count;
	hBlock **check_blocks;

} hSegment;


typedef struct {
	int    type;

	int    field_count;
	char **data;
	
} hDocument;


typedef struct {
	int  revision;
	int  encoding;

	int         cdoc_count;
	hDocument **cdocs;

} hMetadata;


typedef struct {
	int        type;

	hURI      *uri;

	int        openmode;
	char      *mimetype;
	int        size;

	hBlock    *tmpblock;
	hMetadata *metadata;

	int        segment_count;
	hSegment **segments;

} hKey;


typedef struct {
	char    *host;
	unsigned short port;

	int      htl;
	int      regress;
	int      rawmode;

	char    *description;
	int      protocol;
  int      socket;

  char    *error;

	hKey    *key;
		
  FCPRESP  response;
} hFCP;


/**********************************************************************/

/* Global variables */
extern char  _fcpID[4];

extern char *_fcpHost;
extern unsigned short _fcpPort;
extern int   _fcpHtl;
extern int   _fcpRawmode;

extern int   _fcpVerbosity;
extern int   _fcpRegress;
extern int   _fcpInsertAttempts;
extern char *_fcpTmpDir;


/* Function prototypes */
#ifdef __cplusplus
extern "C" {
#endif
	
	/* Handle management functions */
	hFCP   *_fcpCreateHFCP(void);
	void    _fcpDestroyHFCP(hFCP *);

	hURI   *_fcpCreateHURI(void);
	int     _fcpParseURI(hURI *uri, char *key);
	void    _fcpDestroyHURI(hURI *);

	hBlock *_fcpCreateHBlock(void);
	void    _fcpDestroyHBlock(hBlock *);

	hKey   *_fcpCreateHKey(void);
	void    _fcpDestroyHKey(hKey *);

	/* Some FEC definitions */
	hSegment  *_fcpCreateHSegment(void);
	void       _fcpDestroyHSegment(hSegment *);

	/* Socket functions */
	int   _fcpSockConnect(hFCP *hfcp);
	void  _fcpSockDisconnect(hFCP *hfcp);
	
	int   _fcpRecvResponse(hFCP *hfcp);

	/* fcpLog */
	void  _fcpLog(int level, char *format, ...);
	
	/* Startup and shutdown functions */
	int   fcpStartup(void);
	void  fcpTerminate(void);
	
	/* Freenet functions for operations between memory and freenet */
	int   fcpOpenKey(hFCP *hfcp, char *key, int mode);
	int   fcpCloseKey(hFCP *hfcp);

	int   fcpReadKey(hFCP *hfcp, char *buf, int len);
	int   fcpWriteKey(hFCP *hfcp, char *buf, int len);

	int   fcpWriteMetadata(hFCP *hfcp, char *buf, int len);
	int   fcpReadMetadata(hFCP *hfcp, char *buf, int len);
	
	/* Client functions for operations between files on disk and freenet */
	int   fcpPutKeyFromFile(hFCP * hfcp, char *key_filename, char *meta_filename);


	char *GetMimeType(char *pathname);
	char *str_reset(char *dest, char *src);
	
#ifdef __cplusplus
}
#endif

#endif /* EZFCPLIB_H */

