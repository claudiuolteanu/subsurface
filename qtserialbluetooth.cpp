#include <errno.h>

#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QBluetoothSocket>
#include <QEventLoop>

#include <libdivecomputer/custom_serial.h>

extern "C" {
typedef struct serial_t {
	/* Library context. */
	dc_context_t *context;
	/*
	 * RFCOMM socket used for Bluetooth Serial communication.
	 */
	QBluetoothSocket *socket;
	long timeout;
} serial_t;

static int qt_serial_open(serial_t **out, dc_context_t *context, const char* devaddr)
{
	if (out == NULL)
		return DC_STATUS_INVALIDARGS;

	// Allocate memory.
	serial_t *device = (serial_t *) malloc (sizeof (serial_t));
	if (device == NULL) {
		return DC_STATUS_NOMEMORY;
	}

	// Library context.
	device->context = context;

	// Default to blocking reads.
	device->timeout = -1;

	// Create a RFCOMM socket
	device->socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);

	// Wait until the connection succeeds or until an error occurs
	QEventLoop loop;
	loop.connect(device->socket, SIGNAL(connected()), SLOT(quit()));
	loop.connect(device->socket, SIGNAL(error(QBluetoothSocket::SocketError)), SLOT(quit()));
	// Try to connect to the Serial Port Profile service
	device->socket->connectToService(QBluetoothAddress(devaddr), QBluetoothUuid::SerialPort);
	loop.exec();

	if (device->socket->socketDescriptor() == -1) {
		free (device);

		// Get the latest error and try to match it with one from libdivecomputer
		QBluetoothSocket::SocketError err = device->socket->error();
		switch(err) {
		case QBluetoothSocket::HostNotFoundError:
		case QBluetoothSocket::ServiceNotFoundError:
			return DC_STATUS_NODEVICE;
		case QBluetoothSocket::UnsupportedProtocolError:
			return DC_STATUS_PROTOCOL;
		case QBluetoothSocket::OperationError:
			return DC_STATUS_UNSUPPORTED;
		case QBluetoothSocket::NetworkError:
			return DC_STATUS_IO;
		default:
			return QBluetoothSocket::UnknownSocketError;
		}
	}

	*out = device;

	return DC_STATUS_SUCCESS;
}

static int qt_serial_close(serial_t *device)
{
	if (device == NULL || device->socket == NULL)
		return DC_STATUS_SUCCESS;

	device->socket->close();

	delete device->socket;
	free(device);

	return DC_STATUS_SUCCESS;
}

static int qt_serial_read(serial_t *device, void* data, unsigned int size)
{
	if (device == NULL || device->socket == NULL)
		return DC_STATUS_INVALIDARGS;

	unsigned int nbytes = 0, rc;

	while(nbytes < size)
	{
		device->socket->waitForReadyRead(device->timeout);

		rc = device->socket->read((char *) data + nbytes, size - nbytes);

		if (rc < 0) {
			if (errno == EINTR)
			    continue; // Retry.

			return -1; // Something really bad happened :-(
		} else if (rc == 0) {
			// Wait until the device is available for read operations
			QEventLoop loop;
			loop.connect(device->socket, SIGNAL(readyRead()), SLOT(quit()));
			loop.exec();
		}

		nbytes += rc;
	}

	return nbytes;
}

static int qt_serial_write(serial_t *device, const void* data, unsigned int size)
{
	if (device == NULL || device->socket == NULL)
		return DC_STATUS_INVALIDARGS;

	unsigned int nbytes = 0, rc;

	while(nbytes < size)
	{
		device->socket->waitForBytesWritten(device->timeout);

		rc = device->socket->write((char *) data + nbytes, size - nbytes);

		if (rc < 0) {
			if (errno == EINTR || errno == EAGAIN)
			    continue; // Retry.

			return -1; // Something really bad happened :-(
		} else if (rc == 0) {
			break;
		}

		nbytes += rc;
	}

	return nbytes;
}

static int qt_serial_flush(serial_t *device, int queue)
{
	if (device == NULL || device->socket == NULL)
		return DC_STATUS_INVALIDARGS;

	//TODO: add implementation

	return DC_STATUS_SUCCESS;
}

static int qt_serial_get_received(serial_t *device)
{
	if (device == NULL || device->socket == NULL)
		return DC_STATUS_INVALIDARGS;

	return device->socket->bytesAvailable();
}

static int qt_serial_get_transmitted(serial_t *device)
{
	if (device == NULL || device->socket == NULL)
		return DC_STATUS_INVALIDARGS;

	return device->socket->bytesToWrite();
}


const dc_serial_operations_t qt_serial_ops = {
	.open = qt_serial_open,
	.close = qt_serial_close,
	.read = qt_serial_read,
	.write = qt_serial_write,
	.flush = qt_serial_flush,
	.get_received = qt_serial_get_received,
	.get_transmitted = qt_serial_get_transmitted
};

extern void dc_serial_init (dc_serial_t *serial, void *data, const dc_serial_operations_t *ops);

dc_status_t dc_serial_qt_open(dc_serial_t **out, dc_context_t *context, const char *devaddr)
{
	if (out == NULL)
		return DC_STATUS_INVALIDARGS;

	// Allocate memory.
	dc_serial_t *serial_device = (dc_serial_t *) malloc (sizeof (dc_serial_t));

	if (serial_device == NULL) {
		return DC_STATUS_NOMEMORY;
	}

	serial_t *port;

	// Open the serial device.
	int rc = qt_serial_open (&port, context, devaddr);
	if (rc == -1) {
		free (serial_device);
		return DC_STATUS_IO;
	}

	// Initialize data and function pointers
	dc_serial_init(serial_device, (void*)port, &qt_serial_ops);

	// Set the type of the device
	serial_device->type = DC_TRANSPORT_BLUETOOTH;

	*out = serial_device;

	return DC_STATUS_SUCCESS;
}
}
