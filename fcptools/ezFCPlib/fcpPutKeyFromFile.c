//
//  This code is part of FreeWeb - an FCP-based client for Freenet
//
//  Designed and implemented by David McNab, david@rebirthing.co.nz
//  CopyLeft (c) 2001 by David McNab
//
//  The FreeWeb website is at http://freeweb.sourceforge.net
//  The website for Freenet is at http://freenet.sourceforge.net
//
//  This code is distributed under the GNU Public Licence (GPL) version 2.
//  See http://www.gnu.org/ for further details of the GPL.
//

#ifndef WINDOWS
#include "unistd.h"
#endif

#include "stdlib.h"

#include "ezFCPlib.h"

extern char     _fcpID[];

#ifndef WINDOWS
#define _read(fd, buf, len) read(fd, buf, len)
#endif


//
// IMPORTED DECLARATIONS
//

extern int fcpSplitChunkSize;


//
// Function:    fcpPutKeyFromFile()
//
// Arguments:   hfcp
//
// Description:
//
//

int fcpPutKeyFromFile(HFCP *hfcp, char *key, char *file, char *metadata)
{
    int meta_len = 0;
    int fd;
    char buf[2048];
    int count;
    int filesize;
    int status;
    struct stat st;

    // can we open the file?
#ifdef WINDOWS
    if ((fd = _open(file, _O_BINARY)) < 0)
#else
    if ((fd = open(file, 0)) < 0)
#endif
        // failure - cannot open
        return -1;

    // how big's this file?
    //filesize = _filelength(fd);
    stat(file, &st);
    filesize = st.st_size;

	// if it's too big, insert it as splitfile
	if (filesize > fcpSplitChunkSize)
		return fcpInsSplitFile(hfcp, key, file, metadata);

	// connect to Freenet FCP
    if (_fcpSockConnect(hfcp) != 0)
        return -1;

    // save the key
    if ((hfcp->wr_info.uri = _fcpParseUri(key)) == 0)
    {
        _fcpSockDisconnect(hfcp);
        return -1;
    }

    //key = "KSK@xxxbug1";

    // create a put message
    if (metadata != NULL)
    {
        meta_len = strlen(metadata);
        sprintf(buf,
                "ClientPut\nURI=%s\nHopsToLive=%x\nDataLength=%x\nMetadataLength=%x\nData\n",
                key,
                hfcp->htl,
                filesize + meta_len,
                meta_len
                );
    }
    else
    {
        sprintf(buf,
                "ClientPut\nURI=%s\nHopsToLive=%x\nDataLength=%x\nData\n",
                key,
                hfcp->htl,
                filesize
                );
    }

    // send off client put command
    _fcpSockSend(hfcp, _fcpID, 4);
    count = strlen(buf);
    if (_fcpSockSend(hfcp, buf, count) < count)
    {
        // send of put command failed
        _fcpSockDisconnect(hfcp);
        return -1;
    }

    // Send metadata if there's any
    if (metadata)
    {
        if (_fcpSockSend(hfcp, metadata, meta_len) < meta_len)
        {
            _fcpSockDisconnect(hfcp);
            return -1;
        }
    }

    // Now send data
    while ((count = read(fd, buf, 2048)) > 0)
        if (_fcpSockSend(hfcp, buf, count) < 0)
        {
            _fcpLog(FCP_LOG_CRITICAL, "fcpPutKeyFromFile: socket send failed for %s", file);
            return -1;
        }
    close(fd);

    // expecting a success response
    status = _fcpRecvResponse(hfcp);

    switch (status)
    {
    case FCPRESP_TYPE_SUCCESS:
        _fcpLog(FCP_LOG_NORMAL, "fcpPutKeyFromFile: SUCCESS: %s", file);
        break;
    case FCPRESP_TYPE_KEYCOLLISION:
        // either of these are ok
        _fcpLog(FCP_LOG_NORMAL, "fcpPutKeyFromFile: KEYCOLLISON: %s", file);
        break;
    case FCPRESP_TYPE_FORMATERROR:
        _fcpLog(FCP_LOG_NORMAL, "fcpPutKeyFromFile: FORMATERROR: %s", file);
        break;
    case FCPRESP_TYPE_URIERROR:
        _fcpLog(FCP_LOG_NORMAL, "fcpPutKeyFromFile: URIERROR: %s", file);
        break;
    case FCPRESP_TYPE_ROUTENOTFOUND:
        _fcpLog(FCP_LOG_NORMAL, "fcpPutKeyFromFile: ROUTENOTFOUND: %s", file);
        break;
    case FCPRESP_TYPE_SIZEERROR:
        _fcpLog(FCP_LOG_NORMAL, "fcpPutKeyFromFile: SIZEERROR: %s", file);
        break;
    case FCPRESP_TYPE_FAILED:
        _fcpLog(FCP_LOG_NORMAL, "fcpPutKeyFromFile: FAILED: %s", file);
        _fcpLog(FCP_LOG_CRITICAL, "Reason = ", hfcp->conn.response.body.failed.reason);
        break;
    default:
        _fcpLog(FCP_LOG_CRITICAL, "fcpPutKeyFromFile: unknown response from node for %s", file);
        _fcpSockDisconnect(hfcp);
        return -1;
    }

    // finished with connection
    _fcpSockDisconnect(hfcp);

    if (status != FCPRESP_TYPE_SUCCESS && status != FCPRESP_TYPE_KEYCOLLISION)
        return -1;

    // seems successful
/*** OLD_LEAKY - no longer needed
    if (hfcp->conn.response.body.keypair.pubkey != NULL)
        strcpy(hfcp->pubkey, hfcp->conn.response.body.keypair.pubkey);
    if (hfcp->conn.response.body.keypair.privkey != NULL)
        strcpy(hfcp->privkey, hfcp->conn.response.body.keypair.privkey);
    if (hfcp->conn.response.body.keypair.uristr != NULL)
        strcpy(hfcp->created_uri, hfcp->conn.response.body.keypair.uristr);
***/
    return 0;

}       // 'fcpPutKeyFromFile()'