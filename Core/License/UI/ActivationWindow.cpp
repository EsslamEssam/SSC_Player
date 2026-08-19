#include "activationwindow.h"
#include "ui_activationwindow.h"


ActivationWindow::ActivationWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ActivationWindow)
{
    ui->setupUi(this);
}


ActivationWindow::~ActivationWindow()
{
    delete ui;
}



void ActivationWindow::on_activateButton_clicked()
{
    QString key = ui->licenseKeyEdit->text();


    if(m_licenseManager.activateLicense(key))
    {
        ui->statusLabel->setText(
            "License Activated Successfully"
            );

        emit licenseActivated();

        accept();
    }
    else
    {
        ui->statusLabel->setText(
            "Invalid License Key"
            );
    }
}