#include <QShortcut>
#include <QDebug>
#include <QMessageBox>

#include "ui_btdeviceselectiondialog.h"
#include "btdeviceselectiondialog.h"

BtDeviceSelectionDialog::BtDeviceSelectionDialog(QWidget *parent) :
    QDialog(parent),
    localDevice(new QBluetoothLocalDevice),
    ui(new Ui::BtDeviceSelectionDialog)
{
    // Check if Bluetooth is available on this device
    if (!localDevice->isValid()) {
        QMessageBox::warning(this, tr("Warning"),
                     "Your local Bluetooth device cannot be accessed. Please check if you have installed qtconnectivity library.");
        return;
    }

    ui->setupUi(this);

    /* quit button callbacks*/
    QShortcut *quit = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this);
    connect(quit, SIGNAL(activated()), this, SLOT(close()));
    connect(ui->quit, SIGNAL(clicked()), this, SLOT(close()));

    // Add context menu for devices to be able to pair device
    ui->discoveredDevicesList->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(localDevice, SIGNAL(hostModeStateChanged(QBluetoothLocalDevice::HostMode)),
            this, SLOT(hostModeStateChanged(QBluetoothLocalDevice::HostMode)));

    // Initialize the state of the local device and activate/deactive the scan button
    hostModeStateChanged(localDevice->hostMode());
}

BtDeviceSelectionDialog::~BtDeviceSelectionDialog()
{
    delete ui;
}

void BtDeviceSelectionDialog::on_changeDeviceState_clicked()
{
    qDebug() << "Change device state "<< localDevice->hostMode();
    if (localDevice->hostMode() == QBluetoothLocalDevice::HostPoweredOff)
        localDevice->powerOn();
    else
        localDevice->setHostMode(QBluetoothLocalDevice::HostPoweredOff);
}

void BtDeviceSelectionDialog::on_save_clicked()
{
    qDebug() << "Saving the selected device";
    close();
}

void BtDeviceSelectionDialog::on_clear_clicked()
{
    qDebug() << "Cleaning the list";
    ui->discoveredDevicesList->clear();
    ui->scan->setEnabled(true);
}

void BtDeviceSelectionDialog::on_scan_clicked()
{
    qDebug() << "Starting the scan";
    ui->discoveredDevicesList->addItem("Device 1");
    ui->discoveredDevicesList->addItem("Device 2");
    ui->scan->setEnabled(false);
}


void BtDeviceSelectionDialog::hostModeStateChanged(QBluetoothLocalDevice::HostMode mode)
{
    bool on = !(mode == QBluetoothLocalDevice::HostPoweredOff);
    qDebug() << "Device mode " << on;
    ui->deviceState->setChecked(on);
    ui->scan->setEnabled(on);
}
