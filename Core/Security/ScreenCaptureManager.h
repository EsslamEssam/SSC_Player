#ifndef SCREENCAPTUREMANAGER_H
#define SCREENCAPTUREMANAGER_H

#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QThread>

class ScreenCaptureManager : public QObject
{
    Q_OBJECT

public:
    explicit ScreenCaptureManager(QObject *parent = nullptr);

    void startMonitoring();
    void stopMonitoring();

signals:
    void screenCaptureDetected(QString processName);

private slots:
    void checkRunningProcesses();

private:

    QTimer *m_timer;

    QThread *m_workerThread;

    QStringList m_blockedProcesses;

    bool m_captureDetected;
};

#endif // SCREENCAPTUREMANAGER_H