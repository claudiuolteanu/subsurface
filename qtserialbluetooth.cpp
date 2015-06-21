#include "qtserialbluetooth.h"

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

	// TODO: wait for connection completion
	device->socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
	device->socket->connectToService(QBluetoothAddress(devaddr), QBluetoothUuid::SerialPort);


	if (device->socket->socketDescriptor() == -1) {
		free (device);
		return DC_STATUS_IO;
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
	if (device == NULL || device->socket)
		return DC_STATUS_INVALIDARGS;

	device->socket->read((char *)data, size);

	return 0;
}

static int qt_serial_write(serial_t *device, const void* data, unsigned int size)
{
	if (device == NULL || device->socket)
		return DC_STATUS_INVALIDARGS;

	device->socket->write((char *)data, size);

	return 0;
}

static int qt_serial_flush(serial_t *device, int queue)
{
	if (device == NULL || device->socket)
		return DC_STATUS_INVALIDARGS;

	//TODO: add implementation

	return DC_STATUS_SUCCESS;
}

static int qt_serial_get_received(serial_t *device)
{
	if (device == NULL || device->socket)
		return DC_STATUS_INVALIDARGS;

	return device->socket->bytesAvailable();
}

static int qt_serial_get_transmitted(serial_t *device)
{
	if (device == NULL || device->socket)
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
