#ifndef BTDEVICESELECTIONDIALOG_H
#define BTDEVICESELECTIONDIALOG_H

#include <QDialog>

namespace Ui {
    class BtDeviceSelectionDialog;
}

class BtDeviceSelectionDialog : public QDialog {
    Q_OBJECT

public:
    explicit BtDeviceSelectionDialog(QWidget *parent = 0);
    ~BtDeviceSelectionDialog();

private:
    Ui::BtDeviceSelectionDialog *ui;
};

#endif // BTDEVICESELECTIONDIALOG_H
