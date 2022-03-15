/**
 *  Dan Edelstein
 *  daedel@ttu.ee
 *
 *  Adapted from code provided by Jiazi Yi and Pierre Pfister
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "url.h"

 int ParseUrl(char *url, urlInfo *info)
 {
    // url format: [http://]<hostname>[:<port>]/<path>
	// e.g. https://www.polytechnique.edu:80/index.php

    char *colonSlashSlash, *hostNamePath, *protocol, *firstSlash, *colon;
    int tempPort;

    colonSlashSlash = strstr(url, "://");

    if (colonSlashSlash)
    {
        *colonSlashSlash = '\0'; // add string delimiter at end of protocol block
        hostNamePath = colonSlashSlash + 3; // index after '://' for hostName
        protocol = url; // protocol ptr aimed at 0'th url index
    } else { // colonSlashSlash not found in url
        hostNamePath = url; // skip protocol, aim hostName at 0'th url index
        protocol = "http"; // default to http protocol
    }

    info->protocol = protocol;

    if (strcmp("http", info->protocol))
        return PARSE_URL_PROTOCOL_UNKNOWN;      // Pre-defined error const.
    
    firstSlash = strstr(hostNamePath, "/");     // Look for '/' 
    if (firstSlash)
    {
        *firstSlash = '\0';             // Same behavior as above, add delim
        info->hostname = hostNamePath;
        info->path = firstSlash + 1;    // index to path field
    }
    else { // firstSlash not found
        return PARSE_URL_NO_SLASH;      // No valid default, return pre-def error
    }

    colon = strstr(hostNamePath, ":");
    if (colon)
    {
        *colon = '\0';                  // add delim
        colon += 1;
        if(sscanf(colon, "%d", &tempPort))
            info->port = tempPort;
        else
            return PARSE_URL_BAD_PORT;
    }
    else
        info->port = DEFAULT_PORT;
    
    return 0;
 }

 /**
 * print the url info to std output
 */
void PrintUrlInfo(urlInfo *info){
	printf("The URL contains following information: \n");
	printf("Protocol:\t%s\n", info->protocol);
	printf("Host name:\t%s\n", info->hostname);
	printf("Port No.:\t%d\n", info->port);
	printf("Path:\t\t/%s\n", info->path);
}
