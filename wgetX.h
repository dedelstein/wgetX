/**
 *	Dan Edelstien
 *  daedel@ttu.ee
 *
 *  Adapted from code provided by Jiazi Yi and Pierre Pfister
 */

#ifndef WGETX_H_
#define WGETX_H_

#include "url.h"

// Structure used to store the buf/buf len when receiving reply from http server.
typedef struct http_reply
{
    char *reply_buffer; // Buffer address containing recieved info, dynamically allocated
    int reply_buffer_length; /* The total number of bytes in the reply */
} http_reply;

/**
 * \brief write the data to a file
 *
 * \param path -- char *, path and name of the file
 * \param data -- char *, pointer of the buffer that to be written.
 * \param len  -- int,    number of bytes to be written from the data buffer onto the file.
 */
void write_data(const char *path, const char *data, int len);

/**
 * \brief download a page using the http protocol
 * 
 * \param info -- urlInfo struct *, the url information
 * \param reply -- http_reply struct *, the output of the request
 *
 * \return 0 on success, an error code on failure
 */
int download_page(urlInfo *info, http_reply *reply);

/**
 * \brief return a string with a get http request
 *
 * \param info -- urlInfo struct *, the url information
 *
 * \return char* pointer to the get http request string. pointer must be freed using 'free'.
 */
char *http_get_request(urlInfo *info);

/**
 * \brief process the http reply from server
 *
 * \param reply -- http_reply *, the reply structure
 * \return char* pointer to the first data byte
 */
char *read_http_reply(struct http_reply *reply, int *redirPtr);

//char *QueryURL(urlInfo *info, char *url, struct http_reply *reply, int *redirPtr);

#endif /* WGETX_H_ */
