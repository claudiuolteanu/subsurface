#include <QShortcut>
#include <QDebug>

#include "ui_btdeviceselectiondialog.h"
#include "btdeviceselectiondialog.h"

BtDeviceSelectionDialog::BtDeviceSelectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BtDeviceSelectionDialog)
{
    ui->setupUi(this);

    /* quit button callbacks*/
    QShortcut *quit = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this);
    connect(quit, SIGNAL(activated()), this, SLOT(close()));
    connect(ui->quit, SIGNAL(clicked()), this, SLOT(close()));

    // Add context menu for devices to be able to pair device
    ui->discoveredDevicesList->setContextMenuPolicy(Qt::CustomContextMenu);
}

BtDeviceSelectionDialog::~BtDeviceSelectionDialog()
{
    delete ui;
}

void BtDeviceSelectionDialog::on_changeDeviceState_clicked()
{
    qDebug() << "Change device state ";
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
