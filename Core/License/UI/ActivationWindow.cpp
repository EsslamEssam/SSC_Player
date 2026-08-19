#include "ActivationWindow.h"
#include "ui_ActivationWindow.h"

#include <QClipboard>
#include <QGuiApplication>


ActivationWindow::ActivationWindow(
    QWidget *parent
    )
    : QDialog(parent)
    , ui(new Ui::ActivationWindow)
{
    ui->setupUi(this);


    // --------------------------------------------------------
    // Show Device ID
    // --------------------------------------------------------

    ui->deviceIdEdit->setText(
        m_deviceManager.getDeviceId()
        );


    // --------------------------------------------------------
    // License activation result
    // --------------------------------------------------------

    connect(
        &m_licenseManager,
        &LicenseManager::activationFinished,
        this,
        [this](
            bool success,
            const QString &message
            )
        {
            // Show the actual server result
            ui->statusLabel->setText(
                message
                );


            // Allow activation button again
            ui->activateButton->setEnabled(
                true
                );


            // Only close the window if activation
            // was actually successful.
            if (success)
            {
                emit licenseActivated();

                accept();
            }
        }
        );
}


ActivationWindow::~ActivationWindow()
{
    delete ui;
}


// ============================================================
// ACTIVATE BUTTON
// ============================================================

void ActivationWindow::on_activateButton_clicked()
{
    const QString key =
        ui->licenseKeyEdit
            ->text()
            .trimmed();


    // --------------------------------------------------------
    // Empty key
    // --------------------------------------------------------

    if (key.isEmpty())
    {
        ui->statusLabel->setText(
            "Please enter a license key."
            );

        return;
    }


    // --------------------------------------------------------
    // Disable button while waiting for server
    // --------------------------------------------------------

    ui->activateButton->setEnabled(
        false
        );


    ui->statusLabel->setText(
        "Activating license..."
        );


    // --------------------------------------------------------
    // Send activation request
    // --------------------------------------------------------

    m_licenseManager.activateLicense(
        key
        );
}


// ============================================================
// COPY DEVICE ID
// ============================================================

void ActivationWindow::on_copyDeviceIdButton_clicked()
{
    QGuiApplication::clipboard()->setText(
        ui->deviceIdEdit->text()
        );


    ui->statusLabel->setText(
        "Device ID copied to clipboard."
        );
}