#ifndef ACTIVATIONWINDOW_H
#define ACTIVATIONWINDOW_H

#include <QDialog>

#include "../LicenseManager.h"
#include "../../Device/DeviceManager.h"

namespace Ui {
class ActivationWindow;
}

class ActivationWindow : public QDialog
{
    Q_OBJECT

public:

    explicit ActivationWindow(QWidget *parent = nullptr);

    ~ActivationWindow();

signals:

    void licenseActivated();

private slots:

    void on_activateButton_clicked();

    void on_copyDeviceIdButton_clicked();

private:

    Ui::ActivationWindow *ui;

    LicenseManager m_licenseManager;

    DeviceManager m_deviceManager;
};

#endif // ACTIVATIONWINDOW_H