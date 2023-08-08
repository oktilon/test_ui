/**
 * @file dbus.c
 * @author Denys Stovbun (denis.stovbun@lanars.com)
 * @brief Service D-bus routines
 * @version 1.0
 * @date 2023-08-01
 *
 *
 *
 */
#include <gio/gio.h>

#include "app.h"
#include "dbus.h"

static GTask        *taskDbus;
static sd_bus       *bus;

typedef struct DbusTaskDataStruct {
    ViewManager *viewManager;
    GMainContext *mainContext;
} DbusTaskData;

void dbus_deinit() {
    if(bus)
        sd_bus_unref(bus);
}

void dbus_stop (GObject *sourceObject, GAsyncResult *res, gpointer userData) {
    selfLogDbg("dbus_stop, userData=%p", userData);
    // dbus_deinit();
}

int dbus_get_current_view_cb (sd_bus *b, const char *p, const char *i, const char *name, sd_bus_message *reply, void *userData, sd_bus_error *retError) {
    DbusTaskData *td = (ViewManager *)userData;
    return sd_bus_message_append(reply, "s", td->viewManager->currentViewName);
}

int dbus_get_state_cb (sd_bus *b, const char *p, const char *i, const char *name, sd_bus_message *reply, void *userData, sd_bus_error *retError) {
    DbusTaskData *td = (ViewManager *)userData;
    return sd_bus_message_append(reply, "s", td->viewManager->state);
}

int dbus_gateway_message_handler(sd_bus_message *m, void *userData, sd_bus_error *retError) {
    // Variable
    int r;
    // Convert data pointer
    DbusTaskData *td = (ViewManager *)userData;
    // Get member name
    const char *name = sd_bus_message_get_member(m);
    // If it's onUiData signal
    if(g_strcmp0(name, GW_SIGNAL_ON_DATA) == 0) {
        GSource *src = g_idle_source_new();
        if(!src) {
            sd_bus_error_set_const(retError, "Source error", "Source create error");
            return -1;
        }
        MdlDoorbell *mdl = (MdlDoorbell *)g_malloc0(sizeof(MdlDoorbell));
        if(!mdl) {
            sd_bus_error_set_const(retError, "Model error", "Model allocate error");
            return -1;
        }

        r = doorbell_parse_message(mdl, m);
        if(r < 0) {
            sd_bus_error_set_const(retError, "Model error", "Model parse error");
            return r;
        }
        g_source_set_callback(src, td->viewManager->onNewData, m, NULL);
        g_source_attach(src, td->mainContext);
    }
    return 0;
}

/**
 * @brief Service interface items table
 */
static const sd_bus_vtable vtableInterface[] = {
    SD_BUS_VTABLE_START(BUS_COMMON_FLAGS),
    SD_BUS_PROPERTY(UI_PROPERTY_CURRENT_VIEW, "s", dbus_get_current_view_cb, 0, BUS_COMMON_FLAGS),
    SD_BUS_PROPERTY(UI_PROPERTY_STATE, "s", dbus_get_state_cb, 0, BUS_COMMON_FLAGS | SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    /*SD_BUS_SIGNAL_WITH_NAMES(UI_SIGNAL_OPEN_DOOR
        , "sst", SD_BUS_PARAM("cardValue")
                 SD_BUS_PARAM("cardPin")
                 SD_BUS_PARAM("requestIdentifier")
        , BUS_COMMON_FLAGS),*/
    SD_BUS_VTABLE_END
};

int dbus_loop() {
    int r;
    /* Process requests */
    r = sd_bus_process(bus, NULL);
    if (r < 0) {
        selfLogErr("Failed to process bus: %s", strerror(-r));
        return -r;
    }
    if (r > 0) /* we processed a request, try to process another one, right-away */
        return EXIT_SUCCESS;

    /* Wait for the next request to process */
    r = sd_bus_wait(bus, 100000UL);
    if (r < 0) {
        selfLogErr("Failed to wait on bus: %s", strerror(-r));
        return -r;
    }
    return EXIT_SUCCESS;
}

void dbus_thread(GTask *t, gpointer src, gpointer data, GCancellable *c) {
    // Variables
    int stop = 0;
    // int r;
    DbusTaskData *taskData = (ViewManager *)data;

    while(!stop) {
        if(dbus_loop() != EXIT_SUCCESS) break;
        // g_main_context_iteration()
    }
    selfLogErr("Exit form bus thread");
}

/**
 * @brief Initialize D-bus UI-service
 *
 * @param manager pointer to ViewManager
 *
 * @return int error code
 */
int dbus_init(ViewManager *manager, int useSysBus) {
    // Variable
    int r;
    gboolean ok;

    if(!manager) return -EINVAL;

    DbusTaskData *taskData = (DbusTaskData*)g_malloc0(sizeof(DbusTaskData));
    taskData->viewManager = manager;
    taskData->mainContext = g_main_context_default();

    if(useSysBus) {
        r = sd_bus_open_system(&bus);
    } else {
        r = sd_bus_open(&bus);
    }
    if(r < 0) {
        selfLogErr("Failed to open %s bus(%d): %s", useSysBus ? "system" : "session", r, strerror(-r));
        return r;
    }

    r = sd_bus_add_object_vtable(bus,
                                NULL,
                                DBUS_THIS_PATH,
                                DBUS_THIS_INTERFACE,
                                vtableInterface,
                                taskData);
    if (r < 0) {
        // Interface registration error
        selfLogErr("Failed to issue interface(%d): %s", r, strerror(-r));
        return r;
    }

    gchar *match = g_strdup_printf();
    sd_bus_message_handler_t mh = NULL;
    r = sd_bus_add_match(bus, NULL, match, mh, taskData);
    if (r < 0) {
        // Subscription to gateway error
        selfLogErr("Failed to issue interface(%d): %s", r, strerror(-r));
        return r;
    }

    /* Take a well-known service name so that clients can find us */
    r = sd_bus_request_name(bus, DBUS_THIS_NAME, 0);
    if (r < 0) {
        selfLogErr("Failed to acquire service name(%d): %s", r, strerror(-r));
        return r;
    }

    taskDbus = g_task_new (NULL, NULL, dbus_stop, NULL);
    if(taskDbus) {
        g_task_set_task_data(taskDbus, taskData, g_free);
        g_task_run_in_thread(taskDbus, dbus_thread);
    } else {
        selfLogErr("Error create dbus task");
        return -1;
    }


    return 0;
}