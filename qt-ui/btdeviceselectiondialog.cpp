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
    /* Check if Bluetooth is available on this device */
    if (!localDevice->isValid()) {
        QMessageBox::warning(this, tr("Warning"),
                     "Your local Bluetooth device cannot be accessed. Please check if you have installed qtconnectivity library.");
        return;
    }

    ui->setupUi(this);

    /* Quit button callbacks*/
    QShortcut *quit = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this);
    connect(quit, SIGNAL(activated()), this, SLOT(close()));
    connect(ui->quit, SIGNAL(clicked()), this, SLOT(close()));

    /* Set UI information about the local device */
    ui->deviceAddress->setText(localDevice->address().toString());
    ui->deviceName->setText(localDevice->name());

    connect(localDevice, SIGNAL(hostModeStateChanged(QBluetoothLocalDevice::HostMode)),
            this, SLOT(hostModeStateChanged(QBluetoothLocalDevice::HostMode)));

    /* Initialize the state of the local device and activate/deactive the scan button */
    hostModeStateChanged(localDevice->hostMode());

    /* Intialize the discovery agent */
    remoteDeviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent();

    connect(remoteDeviceDiscoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
            this, SLOT(addRemoteDevice(QBluetoothDeviceInfo)));
    connect(remoteDeviceDiscoveryAgent, SIGNAL(finished()), this, SLOT(remoteDeviceScanFinished()));

    /* Add context menu for devices to be able to pair device */
    ui->discoveredDevicesList->setContextMenuPolicy(Qt::CustomContextMenu);
}

BtDeviceSelectionDialog::~BtDeviceSelectionDialog()
{
    delete ui;
}

void BtDeviceSelectionDialog::on_changeDeviceState_clicked()
{
    if (localDevice->hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
        ui->dialogStatus->setText("Trying to turn on the local Bluetooth device...");
        localDevice->powerOn();
    } else {
        ui->dialogStatus->setText("Trying to turn off the local Bluetooth device...");
        localDevice->setHostMode(QBluetoothLocalDevice::HostPoweredOff);
    }
}

void BtDeviceSelectionDialog::on_save_clicked()
{
    qDebug() << "Saving the selected device";
    close();
}

void BtDeviceSelectionDialog::on_clear_clicked()
{
    ui->dialogStatus->setText("Remote devices list was cleaned.");
    ui->discoveredDevicesList->clear();
}

void BtDeviceSelectionDialog::on_scan_clicked()
{
    ui->dialogStatus->setText("Scanning for remote devices...");
    remoteDeviceDiscoveryAgent->start();
    ui->scan->setEnabled(false);
}

void BtDeviceSelectionDialog::remoteDeviceScanFinished()
{
    ui->dialogStatus->setText("Scanning finished.");
    ui->scan->setEnabled(true);
}

void BtDeviceSelectionDialog::hostModeStateChanged(QBluetoothLocalDevice::HostMode mode)
{
    bool on = !(mode == QBluetoothLocalDevice::HostPoweredOff);

    ui->dialogStatus->setText(QString("The local Bluetooth device was turned %1.").arg(on? "ON" : "OFF"));
    ui->deviceState->setChecked(on);
    ui->scan->setEnabled(on);
}

void BtDeviceSelectionDialog::addRemoteDevice(const QBluetoothDeviceInfo &remoteDeviceInfo)
{
    //TODO use a QTableView
    QString deviceLable = QString("%1  (%2)").arg(remoteDeviceInfo.name()).arg(remoteDeviceInfo.address().toString());
    QList<QListWidgetItem *> itemsWithSameSignature = ui->discoveredDevicesList->findItems(deviceLable, Qt::MatchExactly);

    /* Check if the remote device is already in the list */
    if (itemsWithSameSignature.empty()) {
        QListWidgetItem *item = new QListWidgetItem(deviceLable);
        QBluetoothLocalDevice::Pairing pairingStatus = localDevice->pairingStatus(remoteDeviceInfo.address());

        if (pairingStatus == QBluetoothLocalDevice::Paired) {
            item->setBackgroundColor(QColor(Qt::gray));
        } else if (pairingStatus == QBluetoothLocalDevice::AuthorizedPaired) {
            item->setBackgroundColor(QColor(Qt::blue));
        } else {
            item->setTextColor(QColor(Qt::black));
        }

        qDebug() << item->text();
        ui->discoveredDevicesList->addItem(item);
    }
}
