/**
 *  Dan Edelstien
 *  daedel@ttu.ee
 *
 *  Adapted from code provided by Jiazi Yi and Pierre Pfister
 *  I read the following code but i'm fairly certain i didn't reuse any
 *  sections, but if i did i have included the link for due diligence:
 *  https://github.com/unimonkiez/c-linux-example/blob/master/src/wget.c
 *
 *  Other relevant attributions are included below.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "url.h"
#include "wgetX.h"

int main(int argc, char * argv[]) {

    urlInfo info;
    struct http_reply reply; // initialize http_reply
    char * response;

    int redirFlag = 0;
    int * redirPtr = & redirFlag;

    const char * file_name = "received_page";

    if (argc < 2) {
        fprintf(stderr, "Missing argument. Please enter URL.\n");
        return 1;
    }

    char * url = argv[1];

    if (argc > 2) {
        file_name = argv[2];
    }

    int ret = ParseUrl(url, & info);

    if (ret) {
        fprintf(stderr, "Could not parse URL '%s': %s\n", url, ParseUrlErrstr[ret]);
        return 2;
    }

    PrintUrlInfo( & info);

    ret = download_page( & info, & reply);
    if (ret) {
        return 3;
    }

    response = read_http_reply( & reply, redirPtr);
    if (response == NULL) {
        fprintf(stderr, "Could not parse http reply\n");
        return 4;
    }

    if (redirFlag) {
        redirFlag = 0;
        char * newUrl = response;
        free(reply.reply_buffer);

        ret = ParseUrl(newUrl, & info);

        if (ret) {
            fprintf(stderr, "Could not parse URL '%s': %s\n", url, ParseUrlErrstr[ret]);
            free(newUrl);
            return 2;
        }

        PrintUrlInfo( & info);

        ret = download_page( & info, & reply);
        if (ret) {
            free(newUrl);
            return 3;
        }

        response = read_http_reply( & reply, redirPtr);

        if (response == NULL) {
            fprintf(stderr, "Could not parse redirect HTTP reply.\n");
            free(newUrl);
            return 5;
        }
        free(newUrl);
    }

    write_data(file_name, response, reply.reply_buffer + reply.reply_buffer_length - response);

    free(reply.reply_buffer);

    return 0;
}

int download_page(urlInfo * info, http_reply * reply) {

    // addrinfo usage adapted from manpage and
    // https://www.delftstack.com/howto/c/getaddrinfo-in-c/
    // https://stackoverflow.com/questions/52727565/client-in-c-use-gethostbyname-or-getaddrinfo
    // https://linux.die.net/man/3/getaddrinfo

    int sendReturn;
    char * sendBuff;
    int recvReturn;
    int socket_int;
    char * hostAddr;

    struct addrinfo hints = {};
    struct addrinfo * pResultList;

    hints.ai_family = AF_UNSPEC; // allows for both IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int length = snprintf(NULL, 0, "%d", info->port);
    char * portStr = malloc(length + 1);
    snprintf(portStr, length + 1, "%d", info->port);


    if (0 != getaddrinfo(info->hostname, portStr, & hints, & pResultList)) {
        fprintf(stderr, "Error in resolving hostname %s\n", info->hostname);
        free(portStr);
        return 1;
    }

    free(portStr);

    socket_int = socket(pResultList->ai_family, SOCK_STREAM, 0);
    if (socket_int < 0) {
        fprintf(stderr, "Error opening socket\n");
        close(socket_int);
        return socket_int;
    }
    // inet pton/ntop usage from https://man7.org/linux/man-pages/man3/inet_pton.3.html
    // NB! -- I am aware that i need to implement a timeout function because it's possible to get stuck on a bad
    // call to connect(), but this is somewhat outside the scope of the lab prompt
    if (pResultList->ai_family == AF_INET) {
        struct sockaddr_in * sock_address;
        sock_address = (struct sockaddr_in * ) pResultList->ai_addr;
        hostAddr = inet_ntoa(sock_address->sin_addr);

        struct sockaddr_in client_addr;
        bzero( & client_addr, sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        //client_addr.sin_addr.s_addr = inet_addr(hostAddr); used memcpy instead
        memcpy( & client_addr.sin_addr, & (sock_address->sin_addr), pResultList->ai_addrlen);
        client_addr.sin_port = htons(info->port);
        puts("ipv4");
        printf("addr: %s\n", hostAddr);

        // NB! -- will hang on bad address
        if (connect(socket_int, (struct sockaddr * ) & client_addr, sizeof(client_addr)) < 0) {
            fprintf(stderr, "Error connecting socket\n");
            close(socket_int);
            return -1;
        }
    } else if (pResultList->ai_family == AF_INET6) {
        struct sockaddr_in6 * sock_address;
        char inet6str[INET6_ADDRSTRLEN];
        sock_address = (struct sockaddr_in6 * ) pResultList->ai_addr;
        inet_ntop(AF_INET6, & (sock_address->sin6_addr), inet6str, INET6_ADDRSTRLEN);
        puts("ipv6");
        printf("addr: %s\n", inet6str);

        struct sockaddr_in6 client_addr;
        bzero( & client_addr, sizeof(client_addr));
        client_addr.sin6_family = AF_INET6;
        memcpy( & client_addr.sin6_addr, & (sock_address->sin6_addr), pResultList->ai_addrlen);
        //inet_pton(AF_INET6, inet6str, &client_addr.sin6_addr); used memcpy instead
        client_addr.sin6_port = htons(info->port);

        // NB! -- will hang on bad address
        if (connect(socket_int, (struct sockaddr * ) & client_addr, sizeof(client_addr)) < 0) {
            fprintf(stderr, "Error connecting socket\n");
            close(socket_int);
            return -1;
        }
    }

    sendBuff = http_get_request(info);
    sendReturn = write(socket_int, sendBuff, strlen(sendBuff));
    free(sendBuff);

    if (sendReturn == -1) {
        fprintf(stderr, "Could not send: %s\n", strerror(errno));
        return -1;
    }

    shutdown(socket_int, SHUT_WR);

    reply->reply_buffer = malloc(sizeof(char) * 1025);
    reply->reply_buffer_length = 1025;

    int i = 0;
    while ((recvReturn = recv(socket_int, reply->reply_buffer + i, reply->reply_buffer_length, 0)) > 0) {
        i += recvReturn;
        reply->reply_buffer_length = i;
        reply->reply_buffer = realloc(reply->reply_buffer, (reply->reply_buffer_length) * 2);
    }

    int closereturn = close(socket_int);
    printf("close: %d\n", closereturn);

    return 0;
}

void write_data(const char * path,
    const char * data, int len) {

    FILE * foutput = fopen(path, "w");

    if (foutput == NULL) {
        fprintf(stderr, "Could Not Open Output File.\n");
        exit(EXIT_FAILURE);
    }

    fwrite(data, sizeof(char), len, foutput);

    fclose(foutput);

    fprintf(stderr, "the file is saved in %s.\n", path);
}

char * http_get_request(urlInfo * info) {
    char * request_buffer = (char * ) malloc(100 + strlen(info->path) + strlen(info->hostname));
    snprintf(request_buffer, 1024, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
        info->path, info->hostname);
    return request_buffer;
}

char * next_line(char * buff, int len) {
    if (len == 0) {
        return NULL;
    }

    char * last = buff + len - 1;
    while (buff != last) {
        if ( * buff == '\r' && * (buff + 1) == '\n') {
            return buff;
        }
        buff++;
    }
    return NULL;
}

char * read_http_reply(struct http_reply * reply, int * redirPtr) {

    char * status_line = next_line(reply->reply_buffer, reply->reply_buffer_length);
    int len = reply->reply_buffer_length - (status_line - reply->reply_buffer);

    if (status_line == NULL) {
        fprintf(stderr, "Could not find status\n");
        return NULL;
    }

    * status_line = '\0';

    int status;
    double http_version;

    int rv = sscanf(reply->reply_buffer, "HTTP/%lf %d", & http_version, & status);
    if (rv != 2) {
        fprintf(stderr, "Could not parse http response first line (rv=%d, %s)\n", rv, reply->reply_buffer);
        return NULL;
    }

    char * buf = status_line;

    if (status >= 300 && status <= 308) {
        buf = next_line(status_line, len);
        fprintf(stderr, "Status %d, REDIRECT\n", status);

        char * newUrl = malloc(sizeof(char) * 1025);
        int redirRet;

        buf = strstr(status_line + 2, "Location: ");
        redirRet = sscanf(buf + 10, "%s", newUrl);

        if (redirRet != 1) {
            fprintf(stderr, "Could not parse new URL\n");
            free(newUrl);
            exit(EXIT_FAILURE);
        }

        ( * redirPtr) = 1;
        return newUrl;
    } else if (status != 200) {
        fprintf(stderr, "Server returned status %d (should be 200 or 30X)\n", status);
        return NULL;
    }

    char * bufRetr;

    do {
        buf += 2;
        bufRetr = buf;
        len = reply->reply_buffer_length - (buf - reply->reply_buffer);
    } while ((buf = next_line(buf, len)) != NULL);

    return bufRetr;
}