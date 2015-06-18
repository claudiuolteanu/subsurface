#include "ui_btdeviceselectiondialog.h"
#include "btdeviceselectiondialog.h"

BtDeviceSelectionDialog::BtDeviceSelectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BtDeviceSelectionDialog)
{
    ui->setupUi(this);
}

BtDeviceSelectionDialog::~BtDeviceSelectionDialog()
{
    delete ui;
}
