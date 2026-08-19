#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <QString>

class ServerConfig
{
public:

    static QString baseUrl();

    static QString apiVersion();

    static QString publishableKey();
};

#endif // SERVERCONFIG_H