/*
  This code is part of FCPTools - an FCP based client library for Freenet.

  Designed and implemented by David McNab, david@rebirthing.co.nz
  CopyLeft (c) 2001 by David McNab

  The FreeWeb website is at http://freeweb.sourceforge.net
  The website for Freenet is at http://freenet.sourceforge.net

  This code is distributed under the GNU Public Licence (GPL) version 2.
  See http://www.gnu.org/ for further details of the GPL.
*/

#include <sys/types.h>
//#include <sys/socket.h>

#include <string.h>
#include <stdlib.h>

#include "ezFCPlib.h"

extern long xtoi(char *);

/*
	There are 12 possible unique Node responses (2002-08-27)

	NodeHello
	Success

	DataFound
	DataChunk
	DataNotFound
	RouteNotFound

	URIError
	Restarted

	KeyCollision
	Pending

	Failed
	FormatError
*/

static int  getrespHello(hFCP *);
static int  getrespSuccess(hFCP *);

static int  getrespDataFound(hFCP *);
static int  getrespDataChunk(hFCP *);
static int  getrespDataNotFound(hFCP *);
static int  getrespRouteNotFound(hFCP *);

static int  getrespUriError(hFCP *);
static int  getrespRestarted(hFCP *);
static int  getrespKeycollision(hFCP *);
static int  getrespPending(hFCP *);

static int  getrespFailed(hFCP *);
static int  getrespFormatError(hFCP *);

/* FEC specific responses */
static int  getrespSegmentHeaders(hFCP *);
static int  getrespSegmentHeaders(hFCP *);

/*
static int  getresp
static int  getresp
static int  getresp
*/

/* Utility functions.. not directly part of the protocol */
static int  getrespblock(hFCP *, char *respblock, int bytesreqd);
static int  getrespline(hFCP *);

/* */
static char respline[FCPRESP_BUFSIZE + 1];

/*
  Function:    _fcpRecvResponse

  Arguments:   fcpconn

	Returns:     Negative integer if error condition.
	             Zero on success.
*/

int _fcpRecvResponse(hFCP *hfcp)
{
	int doAgain = 1;

	while (doAgain) {

		if (getrespline(hfcp) < 0) {
			return -1;
		}
		
		if (!strcmp(respline, "Restarted")) {
			getrespRestarted(hfcp);
		}
		else if (!strcmp(respline, "Pending")) {
			getrespPending(hfcp);
		}
		else break;
	}
	
	/*
		There are 12 possible unique Node responses:
		
		NodeHello
		Success
		
		DataFound
		DataChunk
		DataNotFound
		RouteNotFound
		
		UriError
		Restarted
		
		KeyCollision
		Pending
		
		Failed
		FormatError
	*/
	
  if (!strcmp(respline, "NodeHello")) {
		hfcp->response.type = FCPRESP_TYPE_NODEHELLO;
		return getrespHello(hfcp);
  }
  else if (!strcmp(respline, "Success")) {
		hfcp->response.type = FCPRESP_TYPE_SUCCESS;
		return getrespSuccess(hfcp);
  }

  else if (!strcmp(respline, "DataFound")) {
		hfcp->response.type = FCPRESP_TYPE_DATAFOUND;
		return getrespDataFound(hfcp);
  }
  else if (!strcmp(respline, "DataChunk")) {
		hfcp->response.type = FCPRESP_TYPE_DATACHUNK;
		return getrespDataChunk(hfcp);
  }
  else if (!strcmp(respline, "DataNotFound")) {
		hfcp->response.type = FCPRESP_TYPE_DATANOTFOUND;
		return getrespDataNotFound(hfcp);
  }
  else if (!strcmp(respline, "RouteNotFound")) {
		hfcp->response.type = FCPRESP_TYPE_ROUTENOTFOUND;
		return getrespRouteNotFound(hfcp);
  }

  else if (!strcmp(respline, "UriError")) {
		hfcp->response.type = FCPRESP_TYPE_URIERROR;
		return getrespUriError(hfcp);
  }
  else if (!strcmp(respline, "Restarted")) {
		hfcp->response.type = FCPRESP_TYPE_RESTARTED;
		return getrespRestarted(hfcp);
  }

  else if (!strcmp(respline, "KeyCollision")) {
		hfcp->response.type = FCPRESP_TYPE_KEYCOLLISION;
		return getrespKeycollision(hfcp);
  }
  else if (!strcmp(respline, "Pending")) {
		hfcp->response.type = FCPRESP_TYPE_PENDING;
		return getrespPending(hfcp);
  }

  else if (!strcmp(respline, "FormatError")) {
		hfcp->response.type = FCPRESP_TYPE_FORMATERROR;
		return getrespFormatError(hfcp);
  }
  else if (!strcmp(respline, "Failed")) {
		hfcp->response.type = FCPRESP_TYPE_FAILED;
		return getrespFailed(hfcp);
  }

	/* FEC specific */
  else if (!strcmp(respline, "SegmentHeader")) {
		hfcp->response.type = FCPRESP_TYPE_SEGMENTHEADER;
		return getrespSegmentHeaders(hfcp);
  }


	/* Else, who knows what the $#@! it is anyway? */
  else {
		_fcpLog(FCP_LOG_CRITICAL, "_fcpRecvResponse: bad reply header fron node");
		return -3;
  }
 
  return 0;
}


/*
	getrespHello()

	Read data from this message directly into final storage place
	('hfcp-> area', rather than the more generic 'hfcp->response.nodehello').
*/

static int getrespHello(hFCP *hfcp)
{
	int len;

	_fcpLog(FCP_LOG_DEBUG, "received NodeHello response");

	while (!getrespline(hfcp)) {

		if (!strncmp(respline, "Protocol=", 9)) {
			hfcp->protocol = xtoi(respline + 9);
		}

		else if (!strncmp(respline, "Node=", 5)) {
			if (hfcp->description) free(hfcp->description);

			len = strlen(respline) - 5; /* "Node=" is 5 bytes */
			hfcp->description = (char *)malloc(len + 1);
			strncpy(hfcp->description, respline+5, len);
		}

		else if (!strncmp(respline, "EndMessage", 10)) {
			_fcpLog(FCP_LOG_DEBUG, "successfully parsed NodeHello response");
			return FCPRESP_TYPE_NODEHELLO;
		}
	}

	return -1;
}


/*
 */

static int getrespSuccess(hFCP *hfcp)
{
	int len;

	_fcpLog(FCP_LOG_DEBUG, "received Success response");

  while (!getrespline(hfcp)) {

		if (!strncmp(respline, "URI=", 4)) {
			if (hfcp->response.success.uri) free(hfcp->response.success.uri);

			len = strlen(respline) - 4;
			hfcp->response.success.uri = (char *)malloc(len + 1);
			strncpy(hfcp->response.success.uri, respline + 4, len);
		}
		
		else if (!strncmp(respline, "PublicKey=", 10)) {
			strncpy(hfcp->response.success.publickey, respline + 10, 40);
		}
		
		else if (!strncmp(respline, "PrivateKey=", 11)) {
			strncpy(hfcp->response.success.privatekey, respline + 11, 40);
		}
		
		else if (!strncmp(respline, "EndMessage", 10)) {
			_fcpLog(FCP_LOG_DEBUG, "successfully parsed Success response");
			return FCPRESP_TYPE_SUCCESS;
		}
  }
	
  return -1;
	
}


/*
*/

static int getrespDataFound(hFCP *hfcp)
{
	_fcpLog(FCP_LOG_DEBUG, "received DataFound response");

	hfcp->response.datafound.datalength = 0;
	hfcp->response.datafound.metadatalength = 0;

	while (!getrespline(hfcp)) {

		if (!strncmp(respline, "DataLength=", 11))
			hfcp->response.datafound.datalength = xtoi(respline + 11);
		
		else if (strncmp(respline, "MetadataLength=", 15) == 0)
			hfcp->response.datafound.metadatalength = xtoi(respline + 15);
		
		else if (!strncmp(respline, "EndMessage", 10))
			return FCPRESP_TYPE_DATAFOUND;
	}
	
	return -1;
}


/*
	Function:    getrespDataChunk()

	Arguments    fcpconn - connection block

	Returns      FCPRESP_TYPE_SUCCESS if successful, -1 otherwise

	Description: reads in and processes details of DataFound response
*/

static int getrespDataChunk(hFCP *hfcp)
{
	_fcpLog(FCP_LOG_DEBUG, "received Datachunk response");

	while (!getrespline(hfcp)) {

		if (!strncmp(respline, "Length=", 7))
			hfcp->response.datachunk.length = xtoi(respline + 7);
		
		else if (!strncmp(respline, "Data", 4))	{
			int len;

			len = hfcp->response.datachunk.length;
			if (hfcp->response.datachunk.data) free(hfcp->response.datachunk.data);
			hfcp->response.datachunk.data = (char *)malloc(len);
			
			/* get len bytes of data */
			if (getrespblock(hfcp, hfcp->response.datachunk.data, len) != len)
				return -1;

			return FCPRESP_TYPE_DATACHUNK;
		}
	}

	return -1;
}


/*
	Function:    getrespDataNotFound()

	Arguments:   fcpconn - connection block

	Returns:     0 if successful, -1 otherwise

	Description: Gets the details of a RouteNotFound
*/

static int getrespDataNotFound(hFCP *hfcp)
{
	_fcpLog(FCP_LOG_DEBUG, "received DataNotFound response");

	while (!getrespline(hfcp)) {

		if (!strncmp(respline, "EndMessage", 10))
			return FCPRESP_TYPE_DATANOTFOUND;
	}

	return -1;
}


/*
	Function:    getrespRouteNotFound()

	Arguments:   fcpconn - connection block

	Returns:     0 if successful, -1 otherwise

	Description: Gets the details of a RouteNotFound
*/

static int getrespRouteNotFound(hFCP *hfcp)
{
	_fcpLog(FCP_LOG_DEBUG, "received RouteNotFound response");

	while (!getrespline(hfcp)) {

		if (!strncmp(respline, "EndMessage", 10))
			return FCPRESP_TYPE_ROUTENOTFOUND;
	}
	
	return -1;
}


/*
	Function:    getrespUriError()

	Arguments:   fcpconn - connection block

	Returns:     number of bytes read if successful, -1 otherwise

	Description: Reads an arbitrary number of bytes from connection
*/

static int getrespUriError(hFCP *hfcp)
{
	_fcpLog(FCP_LOG_DEBUG, "received UriError response");

	while (!getrespline(hfcp)) {

		if (!strncmp(respline, "EndMessage", 10))
			return FCPRESP_TYPE_URIERROR;
	}
	
	return -1;
}


/*
	Function:    getrespRestarted()

	Arguments:   fcpconn - connection block
*/

static int getrespRestarted(hFCP *hfcp)
{
	_fcpLog(FCP_LOG_DEBUG, "received Restarted response");

	return FCPRESP_TYPE_RESTARTED;
}


/*
	Function:    getrespKeycollision()

	Arguments:   fcpconn - connection block

	Returns:     number of bytes read if successful, -1 otherwise

	Description: Reads an arbitrary number of bytes from connection
*/

static int getrespKeycollision(hFCP *hfcp)
{
	int len;

	_fcpLog(FCP_LOG_DEBUG, "received KeyCollision response");

  while (!getrespline(hfcp)) {

		if (!strncmp(respline, "URI=", 4)) {
			if (hfcp->response.keycollision.uri) free(hfcp->response.keycollision.uri);

			len = strlen(respline) - 4;
			hfcp->response.keycollision.uri = (char *)malloc(len + 1);
			strncpy(hfcp->response.keycollision.uri, respline + 4, len);
		}
		
		else if (!strncmp(respline, "PublicKey=", 10)) {
			strncpy(hfcp->response.keycollision.publickey, respline + 10, 40);
		}
		
		else if (!strncmp(respline, "PrivateKey=", 11)) {
			strncpy(hfcp->response.keycollision.privatekey, respline + 11, 40);
		}
		
		else if (!strncmp(respline, "EndMessage", 10))
			return FCPRESP_TYPE_KEYCOLLISION;
  }
	
  return -1;
}


/*
  Function:    getrespPending()

  Arguments    hfcpconn - Freenet FCP handle
*/

static int getrespPending(hFCP *hfcp)
{
	int len;

	_fcpLog(FCP_LOG_DEBUG, "received KeyCollision response");

  while (!getrespline(hfcp)) {

		if (!strncmp(respline, "URI=", 4)) {
			if (hfcp->response.pending.uri) free(hfcp->response.pending.uri);

			len = strlen(respline) - 4;
			hfcp->response.pending.uri = (char *)malloc(len + 1);
			strncpy(hfcp->response.pending.uri, respline + 4, len);
		}
		
		else if (!strncmp(respline, "PublicKey=", 10)) {
			strncpy(hfcp->response.pending.publickey, respline + 10, 40);
		}
		
		else if (!strncmp(respline, "PrivateKey=", 11)) {
			strncpy(hfcp->response.pending.privatekey, respline + 11, 40);
		}
		
		else if (!strncmp(respline, "EndMessage", 10))
			return FCPRESP_TYPE_PENDING;
  }
	
  return -1;
}


/*
  Function:    getrespFailed()

  Arguments    hfcpconn - Freenet FCP handle

  Returns      FCPRESP_TYPE_SUCCESS if successful, -1 otherwise

  Description: reads in and processes details of Failed response
*/

static int getrespFailed(hFCP *hfcp)
{
	int len;

	_fcpLog(FCP_LOG_DEBUG, "received Failed response");

  while (!getrespline(hfcp)) {

		if (!strncmp(respline, "Reason=", 7)) {
			if (hfcp->response.failed.reason) free(hfcp->response.failed.reason);
			
			len = strlen(respline) - 7;
			hfcp->response.failed.reason = (char *)malloc(len + 1);
			strncpy(hfcp->response.failed.reason, respline + 7, len);
		}
		
		else if (strncmp(respline, "EndMessage", 10))
			return FCPRESP_TYPE_FAILED;
  }
  
  return -1;
}


/*
	Function:    getrespFormatError()
	
	Arguments:   fcpconn - connection block
	
	Returns:     0 if successful, -1 otherwise
	
	Description: Gets the details of a format error
*/

static int getrespFormatError(hFCP *hfcp)
{
	int len;

	_fcpLog(FCP_LOG_DEBUG, "received FormatError response");

  while (!getrespline(hfcp)) {

		if (!strncmp(respline, "Reason=", 7)) {
			if (hfcp->response.formaterror.reason) free(hfcp->response.formaterror.reason);
			
			len = strlen(respline) - 7;
			hfcp->response.formaterror.reason = (char *)malloc(len + 1);
			strncpy(hfcp->response.formaterror.reason, respline + 7, len);
		}
		
		else if (strncmp(respline, "EndMessage", 10))
			return FCPRESP_TYPE_FORMATERROR;
  }
  
  return -1;
}


static int  getrespSegmentHeaders(hFCP *hfcp)
{
	_fcpLog(FCP_LOG_DEBUG, "received SegmentHeader response");

  while (!getrespline(hfcp)) {

		if (!strncmp(respline, "FECAlgorithm=", 13)) {
			strncpy(hfcp->response.segmentheader.fec_algorithm, respline + 13, 40);
			_fcpLog(FCP_LOG_DEBUG, "FECAlgorithm: %s", hfcp->response.segmentheader.fec_algorithm);
		}

		else if (!strncmp(respline, "FileLength=", 11)) {
			hfcp->response.segmentheader.filelength = xtoi(respline + 11);
			_fcpLog(FCP_LOG_DEBUG, "FileLength: %d", hfcp->response.segmentheader.filelength);
		}

		else if (!strncmp(respline, "Offset=", 7)) {
			hfcp->response.segmentheader.offset = xtoi(respline + 7);
			_fcpLog(FCP_LOG_DEBUG, "Offset: %d", hfcp->response.segmentheader.offset);
		}

		else if (!strncmp(respline, "BlockCount=", 11)) {
			hfcp->response.segmentheader.block_count = xtoi(respline + 11);
			_fcpLog(FCP_LOG_DEBUG, "BlockCount: %d", hfcp->response.segmentheader.block_count);
		}

		else if (!strncmp(respline, "BlockSize=", 10)) {
			hfcp->response.segmentheader.block_size = xtoi(respline + 10);
			_fcpLog(FCP_LOG_DEBUG, "BlockSize: %d", hfcp->response.segmentheader.block_size);
		}

		else if (!strncmp(respline, "CheckBlockCount=", 16)) {
			hfcp->response.segmentheader.checkblock_count = xtoi(respline + 16);
			_fcpLog(FCP_LOG_DEBUG, "CheckBlockCount: %d", hfcp->response.segmentheader.checkblock_count);
		}

		else if (!strncmp(respline, "CheckBlockSize=", 15)) {
			hfcp->response.segmentheader.checkblock_size = xtoi(respline + 15);
			_fcpLog(FCP_LOG_DEBUG, "CheckBlockSize: %d", hfcp->response.segmentheader.checkblock_size);
		}

		else if (!strncmp(respline, "Segments=", 9)) {
			hfcp->response.segmentheader.segments = xtoi(respline + 9);
			_fcpLog(FCP_LOG_DEBUG, "Segments: %d", hfcp->response.segmentheader.segments);
		}

		else if (!strncmp(respline, "SegmentNum=", 11)) {
			hfcp->response.segmentheader.segment_num = xtoi(respline + 11);
			_fcpLog(FCP_LOG_DEBUG, "SegmentNum: %d", hfcp->response.segmentheader.segment_num);
		}

		else if (!strncmp(respline, "BlocksRequired=", 15)) {
			hfcp->response.segmentheader.blocks_required = xtoi(respline + 15);
			_fcpLog(FCP_LOG_DEBUG, "BlocksRequired: %d", hfcp->response.segmentheader.blocks_required);
		}

 		else if (!strncmp(respline, "EndMessage=", 10)) {
			_fcpLog(FCP_LOG_DEBUG, "response: %s", respline);
			return 0;
		}
  }
  
  return -1;
}


/**********************************************************************/

/*
	Function:    getrespblock()

	Arguments:   fcpconn - connection block

	Returns:     number of bytes read if successful, -1 otherwise

	Description: Reads an arbitrary number of bytes from connection
*/

static int getrespblock(hFCP *hfcp, char *respblock, int bytesreqd)
{
	int i = 0;
	char *cp = respblock;
	
	while (bytesreqd > 0) {
		/* now, try to get and return desired number of bytes */

		if (i = recv(hfcp->socket, cp, bytesreqd, 0)) {
			/* Increment current pointer (cp), and decrement bytes required
				 to read.
			*/
			cp += i;
			bytesreqd -= i;
		}
		else if (i == 0)
			break;      /* connection closed - got all we're gonna get */
		else
			return -1;  /* connection failure */
	}

	/* Return the bytes read */
	return cp - respblock;
}


/*
	Function:    getrespline()

	Arguments:   fcpconn - connection block

	Description: Reads a single line of text from response buffer
*/

static int getrespline(hFCP *hfcp)
{
	char *p = respline;
	int   i;
	int   j;

	for (i = 0; i<FCPRESP_BUFSIZE; i++) {
		if ((j = recv(hfcp->socket, p, 1, 0)) == -1) return -1;
		if (*p == '\n') break;

		/* Finally, point to the char after the most-recently read. */
		p++;
	}

	respline[i] = 0;

	return i > 0 ? 0 : -1;
}

