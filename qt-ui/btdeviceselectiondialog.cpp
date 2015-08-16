#include <QShortcut>
#include <QDebug>
#include <QMessageBox>
#include <QMenu>

#include "ui_btdeviceselectiondialog.h"
#include "btdeviceselectiondialog.h"

BtDeviceSelectionDialog::BtDeviceSelectionDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::BtDeviceSelectionDialog)
{
	ui->setupUi(this);

	// Quit button callbacks
	QShortcut *quit = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this);
	connect(quit, SIGNAL(activated()), this, SLOT(reject()));
	connect(ui->quit, SIGNAL(clicked()), this, SLOT(reject()));

	// Disable the save button because there is no device selected
	ui->save->setEnabled(false);

	// Add event for item selection
	connect(ui->discoveredDevicesList, SIGNAL(itemClicked(QListWidgetItem*)),
		this, SLOT(itemClicked(QListWidgetItem*)));

#if defined(Q_OS_WIN)
	ULONG       ulRetCode = SUCCESS;
	WSADATA     WSAData = { 0 };

	// Initialize WinSock and ask for version 2.2.
	ulRetCode = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (ulRetCode != SUCCESS) {
		QMessageBox::StandardButton warningBox;
		warningBox = QMessageBox::critical(this, "Bluetooth",
						   "Could not initialize the Winsock version 2.2", QMessageBox::Ok);
		return;
	}

	// Initialize the device discovery agent
	initializeDeviceDiscoveryAgent();

	// On Windows we cannot select a device or show information about the local device
	ui->localDeviceDetails->hide();
#else
	// Initialize the local Bluetooth device
	localDevice = new QBluetoothLocalDevice();

	// Populate the list with local bluetooth devices
	QList<QBluetoothHostInfo> localAvailableDevices = localDevice->allDevices();
	int availableDevicesSize = localAvailableDevices.size();

	if (availableDevicesSize > 1) {
		int defaultDeviceIndex = -1;

		for (int it = 0; it < availableDevicesSize; it++) {
			QBluetoothHostInfo localAvailableDevice = localAvailableDevices.at(it);
			ui->localSelectedDevice->addItem(localAvailableDevice.name(),
							 QVariant::fromValue(localAvailableDevice.address()));

			if (localDevice->address() == localAvailableDevice.address())
				defaultDeviceIndex = it;
		}

		// Positionate the current index to the default device and register to index changes events
		ui->localSelectedDevice->setCurrentIndex(defaultDeviceIndex);
		connect(ui->localSelectedDevice, SIGNAL(currentIndexChanged(int)),
			this, SLOT(localDeviceChanged(int)));
	} else {
		// If there is only one local Bluetooth adapter hide the combobox and the label
		ui->selectDeviceLabel->hide();
		ui->localSelectedDevice->hide();
	}

	// Update the UI information about the local device
	updateLocalDeviceInformation();

	// Initialize the device discovery agent
	if (localDevice->isValid())
		initializeDeviceDiscoveryAgent();
#endif
}

BtDeviceSelectionDialog::~BtDeviceSelectionDialog()
{
	delete ui;

#if defined(Q_OS_WIN)
	// Terminate the use of Winsock 2 DLL
	WSACleanup();
#else
	// Clean the local device
	delete localDevice;
#endif
	// Clean the device discovery agent
	if (remoteDeviceDiscoveryAgent->isActive()) {
		remoteDeviceDiscoveryAgent->stop();
#if defined(Q_OS_WIN)
		remoteDeviceDiscoveryAgent->wait();
#endif
	}

	delete remoteDeviceDiscoveryAgent;
}

void BtDeviceSelectionDialog::on_changeDeviceState_clicked()
{
#if defined(Q_OS_WIN)
	// TODO add implementation
#else
	if (localDevice->hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
		ui->dialogStatus->setText("Trying to turn on the local Bluetooth device...");
		localDevice->powerOn();
	} else {
		ui->dialogStatus->setText("Trying to turn off the local Bluetooth device...");
		localDevice->setHostMode(QBluetoothLocalDevice::HostPoweredOff);
	}
#endif
}

void BtDeviceSelectionDialog::on_save_clicked()
{
	// Get the selected device. There will be always a selected device if the save button is enabled.
	QListWidgetItem *currentItem = ui->discoveredDevicesList->currentItem();
	QBluetoothDeviceInfo remoteDeviceInfo = currentItem->data(Qt::UserRole).value<QBluetoothDeviceInfo>();

	// Save the selected device
	selectedRemoteDeviceInfo = QSharedPointer<QBluetoothDeviceInfo>(new QBluetoothDeviceInfo(remoteDeviceInfo));

	if (remoteDeviceDiscoveryAgent->isActive()) {
		// Stop the SDP agent if the clear button is pressed and enable the Scan button
		remoteDeviceDiscoveryAgent->stop();
#if defined(Q_OS_WIN)
		remoteDeviceDiscoveryAgent->wait();
#endif
		ui->scan->setEnabled(true);
	}

	// Close the device selection dialog and set the result code to Accepted
	accept();
}

void BtDeviceSelectionDialog::on_clear_clicked()
{
	ui->dialogStatus->setText("Remote devices list was cleaned.");
	ui->discoveredDevicesList->clear();
	ui->save->setEnabled(false);

	if (remoteDeviceDiscoveryAgent->isActive()) {
		// Stop the SDP agent if the clear button is pressed and enable the Scan button
		remoteDeviceDiscoveryAgent->stop();
#if defined(Q_OS_WIN)
		remoteDeviceDiscoveryAgent->wait();
#endif
		ui->scan->setEnabled(true);
	}
}

void BtDeviceSelectionDialog::on_scan_clicked()
{
	ui->dialogStatus->setText("Scanning for remote devices...");
	ui->discoveredDevicesList->clear();
	remoteDeviceDiscoveryAgent->start();
	ui->scan->setEnabled(false);
}

void BtDeviceSelectionDialog::remoteDeviceScanFinished()
{
	if (remoteDeviceDiscoveryAgent->error() == QBluetoothDeviceDiscoveryAgent::NoError) {
		ui->dialogStatus->setText("Scanning finished successfully.");
	} else {
		deviceDiscoveryError(remoteDeviceDiscoveryAgent->error());
	}

	ui->scan->setEnabled(true);
}

void BtDeviceSelectionDialog::hostModeStateChanged(QBluetoothLocalDevice::HostMode mode)
{
#if defined(Q_OS_WIN)
	// TODO add implementation
#else
	bool on = !(mode == QBluetoothLocalDevice::HostPoweredOff);

	ui->dialogStatus->setText(QString("The local Bluetooth device was turned %1.")
				  .arg(on? "ON" : "OFF"));
	ui->deviceState->setChecked(on);
	ui->scan->setEnabled(on);
#endif
}

void BtDeviceSelectionDialog::addRemoteDevice(const QBluetoothDeviceInfo &remoteDeviceInfo)
{
#if defined(Q_OS_WIN)
	// TODO add the remote device
#else
	QColor pairingColor = QColor(Qt::red);
	QString pairingStatusLabel = QString("UNPAIRED");
	QBluetoothLocalDevice::Pairing pairingStatus = localDevice->pairingStatus(remoteDeviceInfo.address());

	if (pairingStatus == QBluetoothLocalDevice::Paired) {
		pairingStatusLabel = QString("PAIRED");
		pairingColor = QColor(Qt::gray);
	} else if (pairingStatus == QBluetoothLocalDevice::AuthorizedPaired) {
		pairingStatusLabel = QString("AUTHORIZED_PAIRED");
		pairingColor = QColor(Qt::blue);
	}

	QString deviceLabel = QString("%1 (%2)   [State: %3]").arg(remoteDeviceInfo.name(),
								   remoteDeviceInfo.address().toString(),
								   pairingStatusLabel);
	QListWidgetItem *item = new QListWidgetItem(deviceLabel);

	item->setData(Qt::UserRole, QVariant::fromValue(remoteDeviceInfo));
	item->setBackgroundColor(pairingColor);

	ui->discoveredDevicesList->addItem(item);
#endif
}

void BtDeviceSelectionDialog::itemClicked(QListWidgetItem *item)
{
#if defined(Q_OS_WIN)
	// TODO enable the save button and log the information about the selected item
#else
	QBluetoothDeviceInfo remoteDeviceInfo = item->data(Qt::UserRole).value<QBluetoothDeviceInfo>();
	QBluetoothLocalDevice::Pairing pairingStatus = localDevice->pairingStatus(remoteDeviceInfo.address());

	if (pairingStatus == QBluetoothLocalDevice::Unpaired) {
		ui->dialogStatus->setText(QString("The device %1 must be paired in order to be used. Please use the context menu for pairing options.")
					  .arg(remoteDeviceInfo.address().toString()));
		ui->save->setEnabled(false);
	} else {
		ui->dialogStatus->setText(QString("The device %1 can be used for connection. You can press the Save button.")
					  .arg(remoteDeviceInfo.address().toString()));
		ui->save->setEnabled(true);
	}
#endif
}

void BtDeviceSelectionDialog::localDeviceChanged(int index)
{
#if defined(Q_OS_WIN)
	// TODO add implementation
#else
	QBluetoothAddress localDeviceSelectedAddress = ui->localSelectedDevice->itemData(index, Qt::UserRole).value<QBluetoothAddress>();

	// Delete the old localDevice
	if (localDevice)
		delete localDevice;

	// Create a new local device using the selected address
	localDevice = new QBluetoothLocalDevice(localDeviceSelectedAddress);

	ui->dialogStatus->setText(QString("The local device was changed."));

	// Clear the discovered devices list
	on_clear_clicked();

	// Update the UI information about the local device
	updateLocalDeviceInformation();

	// Initialize the device discovery agent
	if (localDevice->isValid())
		initializeDeviceDiscoveryAgent();
#endif
}

void BtDeviceSelectionDialog::displayPairingMenu(const QPoint &pos)
{
#if defined(Q_OS_WIN)
	// TODO add implementation
#else
	QMenu menu(this);
	QAction *pairAction = menu.addAction("Pair");
	QAction *removePairAction = menu.addAction("Remove Pairing");
	QAction *chosenAction = menu.exec(ui->discoveredDevicesList->viewport()->mapToGlobal(pos));
	QListWidgetItem *currentItem = ui->discoveredDevicesList->currentItem();
	QBluetoothDeviceInfo currentRemoteDeviceInfo = currentItem->data(Qt::UserRole).value<QBluetoothDeviceInfo>();
	QBluetoothLocalDevice::Pairing pairingStatus = localDevice->pairingStatus(currentRemoteDeviceInfo.address());

	//TODO: disable the actions
	if (pairingStatus == QBluetoothLocalDevice::Unpaired) {
		pairAction->setEnabled(true);
		removePairAction->setEnabled(false);
	} else {
		pairAction->setEnabled(false);
		removePairAction->setEnabled(true);
	}

	if (chosenAction == pairAction) {
		ui->dialogStatus->setText(QString("Trying to pair device %1")
					  .arg(currentRemoteDeviceInfo.address().toString()));
		localDevice->requestPairing(currentRemoteDeviceInfo.address(), QBluetoothLocalDevice::Paired);
	} else if (chosenAction == removePairAction) {
		ui->dialogStatus->setText(QString("Trying to unpair device %1")
					  .arg(currentRemoteDeviceInfo.address().toString()));
		localDevice->requestPairing(currentRemoteDeviceInfo.address(), QBluetoothLocalDevice::Unpaired);
	}
#endif
}

void BtDeviceSelectionDialog::pairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing)
{
	// Determine the color, the new pairing status and the log message. By default we assume that the devices are UNPAIRED.
	QString remoteDeviceStringAddress = address.toString();
	QColor pairingColor = QColor(Qt::red);
	QString pairingStatusLabel = QString("UNPAIRED");
	QString dialogStatusMessage = QString("Device %1 was unpaired.").arg(remoteDeviceStringAddress);
	bool enableSaveButton = false;

	if (pairing == QBluetoothLocalDevice::Paired) {
		pairingStatusLabel = QString("PAIRED");
		pairingColor = QColor(Qt::gray);
		enableSaveButton = true;
		dialogStatusMessage = QString("Device %1 was paired.").arg(remoteDeviceStringAddress);
	} else if (pairing == QBluetoothLocalDevice::AuthorizedPaired) {
		pairingStatusLabel = QString("AUTHORIZED_PAIRED");
		pairingColor = QColor(Qt::blue);
		enableSaveButton = true;
		dialogStatusMessage = QString("Device %1 was authorized paired.").arg(remoteDeviceStringAddress);
	}

	// Find the items which represent the BTH device and update their state
	QList<QListWidgetItem *> items = ui->discoveredDevicesList->findItems(remoteDeviceStringAddress, Qt::MatchContains);

	for (int i = 0; i < items.count(); ++i) {
		QListWidgetItem *item = items.at(i);
		QString updatedDeviceLabel = item->text().replace(QRegularExpression("PAIRED|AUTHORIZED_PAIRED|UNPAIRED"),
								  pairingStatusLabel);

		item->setText(updatedDeviceLabel);
		item->setBackgroundColor(pairingColor);
	}

	// Check if the updated device is the selected one from the list and inform the user that it can/cannot start the download mode
	QListWidgetItem *currentItem = ui->discoveredDevicesList->currentItem();

	if (currentItem != NULL && currentItem->text().contains(remoteDeviceStringAddress, Qt::CaseInsensitive)) {
		if (pairing == QBluetoothLocalDevice::Unpaired) {
			dialogStatusMessage = QString("The device %1 must be paired in order to be used. Please use the context menu for pairing options.")
						     .arg(remoteDeviceStringAddress);
		} else {
			dialogStatusMessage = QString("The device %1 can now be used for connection. You can press the Save button.")
						     .arg(remoteDeviceStringAddress);
		}
	}

	// Update the save button and the dialog status message
	ui->save->setEnabled(enableSaveButton);
	ui->dialogStatus->setText(dialogStatusMessage);
}

void BtDeviceSelectionDialog::error(QBluetoothLocalDevice::Error error)
{
	ui->dialogStatus->setText(QString("Local device error: %1.")
				  .arg((error == QBluetoothLocalDevice::PairingError)? "Pairing error. If the remote device requires a custom PIN code, "
										       "please try to pair the devices using your operating system. "
										     : "Unknown error"));
}

void BtDeviceSelectionDialog::deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error)
{
	QString errorDescription;

	switch (error) {
	case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
		errorDescription = QString("The Bluetooth adaptor is powered off, power it on before doing discovery.");
		break;
	case QBluetoothDeviceDiscoveryAgent::InputOutputError:
		errorDescription = QString("Writing or reading from the device resulted in an error.");
		break;
	default:
		errorDescription = QString("An unknown error has occurred.");
		break;
	}

	ui->dialogStatus->setText(QString("Device discovery error: %1.").arg(errorDescription));
}

QString BtDeviceSelectionDialog::getSelectedDeviceAddress()
{
	if (selectedRemoteDeviceInfo) {
		return selectedRemoteDeviceInfo.data()->address().toString();
	}

	return QString();
}

QString BtDeviceSelectionDialog::getSelectedDeviceName()
{
	if (selectedRemoteDeviceInfo) {
		return selectedRemoteDeviceInfo.data()->name();
	}

	return QString();
}

void BtDeviceSelectionDialog::updateLocalDeviceInformation()
{
#if defined(Q_OS_WIN)
	// TODO add implementation
#else
	// Check if the selected Bluetooth device can be accessed
	if (!localDevice->isValid()) {
		QString na = QString("Not available");

		// Update the UI information
		ui->deviceAddress->setText(na);
		ui->deviceName->setText(na);

		// Announce the user that there is a problem with the selected local Bluetooth adapter
		ui->dialogStatus->setText(QString("The local Bluetooth adapter cannot be accessed."));

		// Disable the buttons
		ui->save->setEnabled(false);
		ui->scan->setEnabled(false);
		ui->clear->setEnabled(false);
		ui->changeDeviceState->setEnabled(false);

		return;
	}

	// Set UI information about the local device
	ui->deviceAddress->setText(localDevice->address().toString());
	ui->deviceName->setText(localDevice->name());

	connect(localDevice, SIGNAL(hostModeStateChanged(QBluetoothLocalDevice::HostMode)),
		this, SLOT(hostModeStateChanged(QBluetoothLocalDevice::HostMode)));

	// Initialize the state of the local device and activate/deactive the scan button
	hostModeStateChanged(localDevice->hostMode());

	// Add context menu for devices to be able to pair them
	ui->discoveredDevicesList->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->discoveredDevicesList, SIGNAL(customContextMenuRequested(QPoint)),
		this, SLOT(displayPairingMenu(QPoint)));
	connect(localDevice, SIGNAL(pairingFinished(QBluetoothAddress, QBluetoothLocalDevice::Pairing)),
		this, SLOT(pairingFinished(QBluetoothAddress, QBluetoothLocalDevice::Pairing)));

	connect(localDevice, SIGNAL(error(QBluetoothLocalDevice::Error)),
		this, SLOT(error(QBluetoothLocalDevice::Error)));
#endif
}

void BtDeviceSelectionDialog::initializeDeviceDiscoveryAgent()
{
#if defined(Q_OS_WIN)
	// TODO initialize the discovery agent
#else
	// Intialize the discovery agent
	remoteDeviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(localDevice->address());

	// Test if the discovery agent was successfully created
	if (remoteDeviceDiscoveryAgent->error() == QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError) {
		ui->dialogStatus->setText(QString("The device discovery agent was not created because the %1 address does not "
						  "match the physical adapter address of any local Bluetooth device.")
					  .arg(localDevice->address().toString()));
		ui->scan->setEnabled(false);
		ui->clear->setEnabled(false);
		return;
	}

	connect(remoteDeviceDiscoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
		this, SLOT(addRemoteDevice(QBluetoothDeviceInfo)));
	connect(remoteDeviceDiscoveryAgent, SIGNAL(finished()),
		this, SLOT(remoteDeviceScanFinished()));
	connect(remoteDeviceDiscoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),
		this, SLOT(deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error)));
#endif
}

#if defined(Q_OS_WIN)
static QString getLastErrorAsString()
{
	LPVOID lpMsgBuf = NULL;
	DWORD lastError = GetLastError();

	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			  NULL,
			  lastError,
			  0,
			  (LPTSTR) &lpMsgBuf,
			  0,NULL)) {
		return QString((char *)lpMsgBuf);
	} else {
		qDebug() << "Failed to format the message for the last error! Error number : " << lastError;
	}

	return QString("Unknown error");
}

WinBluetoothDeviceDiscoveryAgent::WinBluetoothDeviceDiscoveryAgent(QObject *parent) : QThread(parent)
{
}

WinBluetoothDeviceDiscoveryAgent::~WinBluetoothDeviceDiscoveryAgent()
{
}

bool WinBluetoothDeviceDiscoveryAgent::isActive() const
{
	return running;
}

QString WinBluetoothDeviceDiscoveryAgent::errorToString() const
{
	return lastErrorToString;
}

QBluetoothDeviceDiscoveryAgent::Error WinBluetoothDeviceDiscoveryAgent::error() const
{
	return lastError;
}

void WinBluetoothDeviceDiscoveryAgent::run()
{
	// Initialize query for device and start the lookup service
	WSAQUERYSET queryset;
	HANDLE hLookup;
	int result = SUCCESS;

	running = true;
	lastError = QBluetoothDeviceDiscoveryAgent::NoError;

	memset(&queryset, 0, sizeof(WSAQUERYSET));
	queryset.dwSize = sizeof(WSAQUERYSET);
	queryset.dwNameSpace = NS_BTH;

	// The LUP_CONTAINERS flag is used to signal that we are doing a device inquiry
	// while LUP_FLUSHCACHE flag is used to flush the device cache for all inquiries
	// and to do a fresh lookup instead.
	result = WSALookupServiceBegin(&queryset, LUP_CONTAINERS | LUP_FLUSHCACHE, &hLookup);

	if (result != SUCCESS) {
		// Get the last error and emit a signal
		lastErrorToString = getLastErrorAsString();
		lastError = QBluetoothDeviceDiscoveryAgent::PoweredOffError;
		emit error(lastError);

		// Announce that the inquiry finished and restore the stopped flag
		running = false;
		stopped = false;

		return;
	}

	// Declare the necessary variables to collect the information
	BYTE buffer[4096];
	DWORD bufferLength = sizeof(buffer);
	WSAQUERYSET *pResults = (WSAQUERYSET*)&buffer;

	memset(buffer, 0, sizeof(buffer));

	pResults->dwSize = sizeof(WSAQUERYSET);
	pResults->dwNameSpace = NS_BTH;
	pResults->lpBlob = NULL;

	//Start looking for devices
	while (result == SUCCESS && !stopped){
		// LUP_RETURN_NAME and LUP_RETURN_ADDR flags are used to return the name and the address of the discovered device
		result = WSALookupServiceNext(hLookup, LUP_RETURN_NAME | LUP_RETURN_ADDR, &bufferLength, pResults);

		if (result == SUCCESS) {
			// Found a device
			char addressAsString[BTH_ADDR_STR_LEN];
			DWORD addressSize = sizeof (addressAsString);

			// Collect the address of the device from the WSAQUERYSET
			SOCKADDR_BTH *socketBthAddress = (SOCKADDR_BTH *) pResults->lpcsaBuffer->RemoteAddr.lpSockaddr;

			// Convert the BTH_ADDR to string
			if (WSAAddressToStringA((LPSOCKADDR) socketBthAddress,
						sizeof (*socketBthAddress),
						NULL,
						addressAsString,
						&addressSize
						) != 0) {
				// Get the last error and emit a signal
				lastErrorToString = getLastErrorAsString();
				lastError = QBluetoothDeviceDiscoveryAgent::UnknownError;
				emit(lastError);

				break;
			}

			// Save the address and the name of the discovered device
			QString deviceName = QString(pResults->lpszServiceInstanceName);
			QString deviceAddress = QString(addressAsString);

			// Remove the round parentheses
			deviceAddress.remove(')');
			deviceAddress.remove('(');

			// Create an object with information about the discovered device
			QBluetoothDeviceInfo deviceInfo = QBluetoothDeviceInfo(QBluetoothAddress(deviceAddress), deviceName, 0);

			// Raise a signal with information about the found remote device
			emit deviceDiscovered(deviceInfo);
		} else {
			// Get the last error and emit a signal
			lastErrorToString = getLastErrorAsString();
			lastError = QBluetoothDeviceDiscoveryAgent::UnknownError;
			emit(lastError);
		}
	}

	// Announce that the inquiry finished and restore the stopped flag
	running = false;
	stopped = false;

	// Restore the error status
	lastError = QBluetoothDeviceDiscoveryAgent::NoError;

	// End the lookup service
	WSALookupServiceEnd(hLookup);
}

void WinBluetoothDeviceDiscoveryAgent::stop()
{
	// Stop the inqury
	stopped = true;
}
#endif
