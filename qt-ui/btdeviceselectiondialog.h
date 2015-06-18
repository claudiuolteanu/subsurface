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

private slots:
    void on_changeDeviceState_clicked();
    void on_save_clicked();
    void on_clear_clicked();
    void on_scan_clicked();

private:
    Ui::BtDeviceSelectionDialog *ui;
};

#endif // BTDEVICESELECTIONDIALOG_H
