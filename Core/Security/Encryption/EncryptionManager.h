#ifndef ENCRYPTIONMANAGER_H
#define ENCRYPTIONMANAGER_H

#include <QObject>
#include <QString>
#include <QByteArray>

class EncryptionManager : public QObject
{
    Q_OBJECT

public:

    explicit EncryptionManager(QObject *parent = nullptr);

    bool encryptFile(
        const QString &inputFile,
        const QString &outputFile
        );

    bool decryptFile(
        const QString &inputFile,
        QByteArray &outputData
        );

private:

    QByteArray m_key;

    QByteArray m_iv;

    void initializeKeys();

};

#endif // ENCRYPTIONMANAGER_H