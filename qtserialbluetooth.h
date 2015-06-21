#ifndef QTSERIALBLUETOOTH_H
#define QTSERIALBLUETOOTH_H

#include <libdivecomputer/custom_serial.h>

#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QBluetoothSocket>

struct serial_t {
	/* Library context. */
	dc_context_t *context;
	/*
	 * The file descriptor corresponding to the serial port.
	 */
	QBluetoothSocket *socket;
	long timeout;
};

dc_status_t dc_serial_qt_open(dc_serial_t **out, dc_context_t *context, const char *devaddr);

#endif // QTSERIALBLUETOOTH_H
