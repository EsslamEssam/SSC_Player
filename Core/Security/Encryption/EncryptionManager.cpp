#include "EncryptionManager.h"

EncryptionManager::EncryptionManager(QObject *parent)
    : QObject(parent)
{
    initializeKeys();
}

void EncryptionManager::initializeKeys()
{
}

bool EncryptionManager::encryptFile(
    const QString &inputFile,
    const QString &outputFile
    )
{
    Q_UNUSED(inputFile)
    Q_UNUSED(outputFile)

    return false;
}

bool EncryptionManager::decryptFile(
    const QString &inputFile,
    QByteArray &outputData
    )
{
    Q_UNUSED(inputFile)
    Q_UNUSED(outputData)

    return false;
}