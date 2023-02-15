#ifndef CONNECTION_H
#define CONNECTION_H
#include "includes.h"
#include <string>



class Connection
{
    struct sockaddr_in server;
    const int port;
    const std::string host;
    int sd;

public:

    Connection(int port, std::string host) : port{port}, host{host}
    {
        /* cream socketul */
        ASSERT((sd = socket(AF_INET, SOCK_STREAM, 0)) != -1, "Error at socket!!!");

        /* umplem structura folosita pentru realizarea conexiunii cu serverul */
        /* familia socket-ului */
        server.sin_family = AF_INET;
        /* adresa IP a serverului */
        server.sin_addr.s_addr = inet_addr(host.c_str());
        /* portul de conectare */
        server.sin_port = htons(port);

        /* ne conectam la server */
        ASSERT(connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) != -1, "[client]Eroare la connect().\n");
    }

    ~Connection()
    {
        close(sd);
    }

    int GetSd()
    {
        return sd;
    }
};

#endif
