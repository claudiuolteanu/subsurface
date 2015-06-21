#ifndef QTSERIALBLUETOOTH_H
#define QTSERIALBLUETOOTH_H

#include "libdivecomputer/custom_serial.h"

dc_status_t dc_serial_qt_open(dc_serial_t **out, dc_context_t *context, const char *devaddress);

#endif // QTSERIALBLUETOOTH_H
