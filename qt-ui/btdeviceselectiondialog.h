#ifndef BTDEVICESELECTIONDIALOG_H
#define BTDEVICESELECTIONDIALOG_H

#include <QDialog>
#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtBluetooth/qbluetoothglobal.h>

namespace Ui {
    class BtDeviceSelectionDialog;
}

class BtDeviceSelectionDialog : public QDialog {
    Q_OBJECT

public:
    explicit BtDeviceSelectionDialog(QWidget *parent = 0);
    ~BtDeviceSelectionDialog();

private slots:
    void on_changeDeviceState_clicked();
    void on_save_clicked();
    void on_clear_clicked();
    void on_scan_clicked();
    void hostModeStateChanged(QBluetoothLocalDevice::HostMode);

private:
    Ui::BtDeviceSelectionDialog *ui;
    QBluetoothLocalDevice *localDevice;
};

#endif // BTDEVICESELECTIONDIALOG_H
