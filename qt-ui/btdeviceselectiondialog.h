#ifndef BTDEVICESELECTIONDIALOG_H
#define BTDEVICESELECTIONDIALOG_H

#include <QDialog>
#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtBluetooth/qbluetoothglobal.h>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>

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
    void remoteDeviceScanFinished();
    void hostModeStateChanged(QBluetoothLocalDevice::HostMode);
    void addRemoteDevice(const QBluetoothDeviceInfo &remoteDeviceInfo);

private:
    Ui::BtDeviceSelectionDialog *ui;
    QBluetoothLocalDevice *localDevice;
    QBluetoothDeviceDiscoveryAgent *remoteDeviceDiscoveryAgent;
};

#endif // BTDEVICESELECTIONDIALOG_H
