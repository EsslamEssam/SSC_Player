#include "ActivationWindow.h"
#include "ui_ActivationWindow.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QList>
#include <QVBoxLayout>


ActivationWindow::ActivationWindow(
    QWidget *parent
    )
    : QDialog(parent)
    , ui(new Ui::ActivationWindow)
{
    ui->setupUi(this);

    setMinimumSize(540, 450);
    resize(540, 450);

    // Keep the activation flow unchanged while bringing it into the same
    // dark, focused visual system as the main player window.
    const QList<QWidget*> styledWidgets =
        {
            ui->label,
            ui->label_2,
            ui->licenseKeyEdit,
            ui->deviceIdTitleLabel,
            ui->deviceIdEdit,
            ui->copyDeviceIdButton,
            ui->activateButton,
            ui->statusLabel
        };

    for (QWidget *widget : styledWidgets)
    {
        if (widget)
        {
            widget->setStyleSheet(QString());
        }
    }

    if (auto *layout = qobject_cast<QVBoxLayout*>(this->layout()))
    {
        layout->setContentsMargins(36, 30, 36, 30);
        layout->setSpacing(16);
    }

    ui->verticalLayout->setSpacing(12);

    setStyleSheet(QString::fromUtf8(R"QSS(
QDialog#ActivationWindow {
    background-color: #0b1220;
    color: #edf3fb;
}

QLabel {
    color: #b7c8dc;
    font-family: "Segoe UI";
}

QLabel#label {
    color: #f6f8fc;
    font-size: 24px;
    font-weight: 700;
    padding-bottom: 6px;
}

QLabel#label_2,
QLabel#deviceIdTitleLabel {
    color: #a9bad0;
    font-size: 12px;
    font-weight: 600;
}

QLineEdit {
    color: #edf3fb;
    background-color: #121f33;
    border: 1px solid #2a3d59;
    border-radius: 10px;
    padding: 9px 12px;
    min-height: 36px;
    font-family: "Segoe UI";
    font-size: 13px;
    selection-background-color: #2b77ae;
    selection-color: #ffffff;
}

QLineEdit:focus {
    border: 1px solid #5bb8ff;
}

QLineEdit:read-only {
    color: #8fa6bf;
    background-color: #0f1a2b;
}

QPushButton {
    color: #dce9f8;
    background-color: #1b2b43;
    border: 1px solid #304865;
    border-radius: 10px;
    padding: 9px 14px;
    min-height: 40px;
    font-family: "Segoe UI";
    font-size: 13px;
    font-weight: 600;
}

QPushButton:hover {
    background-color: #24415f;
    border: 1px solid #5bb8ff;
}

QPushButton:pressed {
    background-color: #17304c;
}

QPushButton:disabled {
    color: #71839a;
    background-color: #172131;
    border: 1px solid #243247;
}

QPushButton#copyDeviceIdButton {
    color: #a9d9ff;
    background-color: transparent;
    border: 1px solid #355675;
}

QPushButton#activateButton {
    color: #071321;
    background-color: #62c2ff;
    border: 1px solid #8bd4ff;
    font-weight: 700;
}

QPushButton#activateButton:hover {
    background-color: #86d2ff;
}

QPushButton#activateButton:pressed {
    background-color: #42a9e8;
}

QLabel#statusLabel {
    color: #a9bad0;
    background-color: transparent;
    font-size: 12px;
    font-weight: 600;
}
)QSS"));


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
