/**
 *  Dan Edelstein
 *  daedel@ttu.ee
 * 
 *  24.02.2022
 *
 *  Adapted from code provided by Jiazi Yi and Pierre Pfister
 */

#ifndef URL_HEADER
#define URL_HEADER

#define _XOPEN_SOURCE 600

// url container
struct urlInfo
{
	char * protocol;
	char * hostname;
	int port;
	char * path;
};

typedef struct urlInfo urlInfo;

#define DEFAULT_PORT 80

// error parser indices
#define PARSE_URL_OK 0
#define PARSE_URL_PROTOCOL_UNKNOWN 1
#define PARSE_URL_NO_SLASH 2
#define PARSE_URL_BAD_PORT 3

// parse_url associated error strings
static const char *ParseUrlErrstr[] = {"no error" , "unknown protocol",
									   "no trailing slash", "invalid port"};

// Takes url string and url container structure,
// returns success or enumerated error int
int ParseUrl(char * url, urlInfo * info);

// Takes url container and displays user-friendly info
void PrintUrlInfo(urlInfo * info);

#endif
