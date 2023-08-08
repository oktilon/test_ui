#ifndef DBUS_H
#define DBUS_H
#include <systemd/sd-bus.h>

#include "view_manager.h"

int dbus_init (ViewManager *manager, int useSysBus);
void dbus_stop (GObject *sourceObject, GAsyncResult *res, gpointer userData);
void dbus_start (GTask *task, gpointer sourceObject, gpointer taskData, GCancellable *cancellable);

#endif // DBUS_H