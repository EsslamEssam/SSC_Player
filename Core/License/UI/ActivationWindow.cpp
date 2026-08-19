#include "ActivationWindow.h"
#include "ui_ActivationWindow.h"

#include <QClipboard>
#include <QColor>
#include <QGuiApplication>
#include <QList>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QRectF>
#include <QVBoxLayout>


namespace
{

QPixmap makeActivationMark()
{
    QPixmap mark(52, 52);
    mark.fill(Qt::transparent);

    QPainter painter(&mark);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#1d5d89"));
    painter.drawRoundedRect(QRectF(1, 1, 50, 50), 14, 14);

    painter.setBrush(QColor("#f3f9ff"));
    painter.drawRoundedRect(QRectF(15, 25, 22, 16), 5, 5);

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor("#f3f9ff"), 3, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(QRectF(19, 12, 14, 20), 0, 180 * 16);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#1d5d89"));
    painter.drawEllipse(QRectF(24, 30, 4, 4));
    painter.drawRoundedRect(QRectF(25, 33, 2, 5), 1, 1);

    return mark;
}

}


ActivationWindow::ActivationWindow(
    QWidget *parent
    )
    : QDialog(parent)
    , ui(new Ui::ActivationWindow)
{
    ui->setupUi(this);

    setMinimumSize(580, 470);
    resize(640, 500);

    ui->activationCard->setAttribute(Qt::WA_StyledBackground, true);
    ui->brandIcon->setPixmap(makeActivationMark());

    // Keep the activation flow unchanged while bringing it into the same
    // dark, focused visual system as the main player window.
    const QList<QWidget*> styledWidgets =
        {
            ui->brandIcon,
            ui->label,
            ui->subtitleLabel,
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
        layout->setContentsMargins(28, 28, 28, 28);
        layout->setSpacing(0);
    }

    ui->verticalLayout->setSpacing(10);
    ui->deviceRowLayout->setSpacing(10);
    ui->actionLayout->setSpacing(14);

    setStyleSheet(QString::fromUtf8(R"QSS(
QDialog#ActivationWindow {
    background-color: #0b1220;
    color: #edf3fb;
}

QFrame#activationCard {
    background-color: #111b2d;
    border: 1px solid #263b59;
    border-radius: 18px;
}

QLabel {
    color: #b7c8dc;
    font-family: "Segoe UI";
}

QLabel#label {
    color: #f6f8fc;
    font-size: 23px;
    font-weight: 700;
}

QLabel#subtitleLabel {
    color: #8fa6bf;
    font-size: 12px;
    font-weight: 500;
}

QFrame#headerLine {
    background-color: #263b59;
    border: none;
}

QLabel#label_2,
QLabel#deviceIdTitleLabel {
    color: #a9bad0;
    font-size: 12px;
    font-weight: 600;
    padding: 0;
}

QLineEdit {
    color: #edf3fb;
    background-color: #121f33;
    border: 1px solid #2a3d59;
    border-radius: 10px;
    padding: 0 14px;
    min-height: 44px;
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
    padding: 0 14px;
    min-height: 44px;
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

QPushButton:focus {
    border: 1px solid #8bd4ff;
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
    min-width: 128px;
}

QPushButton#activateButton {
    color: #071321;
    background-color: #62c2ff;
    border: 1px solid #8bd4ff;
    font-weight: 700;
    min-width: 140px;
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
    padding: 0 2px;
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
