#include "ScreenCaptureManager.h"

#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <tlhelp32.h>
#endif

ScreenCaptureManager::ScreenCaptureManager(QObject *parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);

    m_workerThread = new QThread(this);

    m_captureDetected = false;

    connect(
        m_timer,
        &QTimer::timeout,
        this,
        &ScreenCaptureManager::checkRunningProcesses
        );

    m_blockedProcesses =
        {
            "obs64.exe",
            "obs32.exe",

            "Streamlabs OBS.exe",

            "bdcam.exe",

            "sharex.exe",

            "CamtasiaStudio.exe",
            "camtasia.exe",

            "ScreenRec.exe",

            "Action.exe",

        };
}


void ScreenCaptureManager::startMonitoring()
{
    if (!m_timer->isActive())
    {
        m_timer->start(5000);
    }
}

void ScreenCaptureManager::stopMonitoring()
{
    m_timer->stop();
}

void ScreenCaptureManager::checkRunningProcesses()
{
#ifdef Q_OS_WIN

    HANDLE snapshot =
        CreateToolhelp32Snapshot(
            TH32CS_SNAPPROCESS,
            0
            );


    if(snapshot == INVALID_HANDLE_VALUE)
    {
        return;
    }


    PROCESSENTRY32 processEntry;

    processEntry.dwSize =
        sizeof(PROCESSENTRY32);


    if(Process32First(
            snapshot,
            &processEntry))
    {

        do
        {
            QString processName =
                QString::fromWCharArray(
                    processEntry.szExeFile
                    );


            QString lowerName =
                processName.toLower();


            for(auto it = m_blockedProcesses.cbegin();
                 it != m_blockedProcesses.cend();
                 ++it)
            {

                if(lowerName ==
                    it->toLower())
                {

                    if(!m_captureDetected)
                    {
                        m_captureDetected = true;


                        qDebug()
                            << "Screen capture detected:"
                            << processName;


                        emit screenCaptureDetected(
                            processName
                            );
                    }


                    CloseHandle(snapshot);

                    return;
                }
            }


        }
        while(Process32Next(
            snapshot,
            &processEntry));
    }


    CloseHandle(snapshot);


    m_captureDetected = false;


#endif
}