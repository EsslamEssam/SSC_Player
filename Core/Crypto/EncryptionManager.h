#ifndef ENCRYPTIONMANAGER_H
#define ENCRYPTIONMANAGER_H


#include <QString>


class EncryptionManager
{

public:

    static bool encryptFile(
        const QString& inputFile,
        const QString& outputFile,
        const QString& videoId
        );


    static bool decryptFile(
        const QString& inputFile,
        const QString& outputFile
        );

    static bool encryptVideoFile(
        const QString& inputFile,
        const QString& outputFile,
        const QString& videoId
        );


};


#endif