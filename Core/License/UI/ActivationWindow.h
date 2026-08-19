#ifndef ACTIVATIONWINDOW_H
#define ACTIVATIONWINDOW_H

#include <QDialog>

#include "../LicenseManager.h"


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


private:

    Ui::ActivationWindow *ui;

    LicenseManager m_licenseManager;

};


#endif // ACTIVATIONWINDOW_H