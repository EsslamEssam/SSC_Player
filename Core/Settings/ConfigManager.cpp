#include "ConfigManager.h"


ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
{

}


QString ConfigManager::appName() const
{
    return "SCPP Player";
}


QString ConfigManager::appVersion() const
{
    return "1.0.0";
}


QString ConfigManager::serverUrl() const
{
    return "https://localhost";
}