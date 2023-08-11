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
#include <semaphore.h>

#include "app.h"
#include "models/mdl_doorbell.h"
#include "dbus.h"

typedef struct DbusTaskDataStruct {
    ViewManager *viewManager;
    GMainContext *mainContext;
    gboolean useSysBus;
} DbusTaskData;

static sem_t        mutex;
static GTask        *taskDbus;
static GQuark       qDbus;
static ViewManager  *viewManager;
// Thread variables
static sd_bus       *bus;

/**
 * @brief Gets dbus error if it's possible
 *
 * @param e Dbus error
 * @param error errno
 * @return const char*
 */
const char *dbus_get_error(const sd_bus_error *e, int error) {
    if (e) {
        if (sd_bus_error_has_name(e, SD_BUS_ERROR_ACCESS_DENIED))
            return "Access denied";

        if (e->message)
            return e->message;
    }

    return strerror(abs(error));
}

void dbus_deinit() {
    if(bus)
        sd_bus_unref(bus);
}

void dbus_stop (GObject *sourceObject, GAsyncResult *res, gpointer userData) {
    selfLogDbg("dbus_stop, userData=%p", userData);
    // dbus_deinit();
}

int dbus_get_current_view_cb (sd_bus *b, const char *p, const char *i, const char *name, sd_bus_message *reply, void *userData, sd_bus_error *retError) {
    DbusTaskData *td = (DbusTaskData *)userData;
    return sd_bus_message_append(reply, "s", td->viewManager->getCurrentViewName());
}

int dbus_get_state_cb (sd_bus *b, const char *p, const char *i, const char *name, sd_bus_message *reply, void *userData, sd_bus_error *retError) {
    DbusTaskData *td = (DbusTaskData *)userData;
    return sd_bus_message_append(reply, "y", (uint8_t)td->viewManager->getState());
}

int dbus_get_state_name_cb (sd_bus *b, const char *p, const char *i, const char *name, sd_bus_message *reply, void *userData, sd_bus_error *retError) {
    DbusTaskData *td = (DbusTaskData *)userData;
    return sd_bus_message_append(reply, "s", td->viewManager->getStateName());
}

int dbus_on_ui_data_handler(sd_bus_message *m, void *userData, sd_bus_error *retError) {
    // Variable
    int r;
    // Convert data pointer
    DbusTaskData *td = (DbusTaskData *)userData;
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
    g_source_set_callback(src, td->viewManager->onNewData, mdl, NULL);
    g_source_attach(src, td->mainContext);
    return 0;
}

/**
 * @brief Service interface items table
 */
static const sd_bus_vtable vtableInterface[] = {
    SD_BUS_VTABLE_START(BUS_COMMON_FLAGS),
    SD_BUS_PROPERTY(UI_PROPERTY_CURRENT_VIEW, "s", dbus_get_current_view_cb, 0, BUS_COMMON_FLAGS),
    SD_BUS_PROPERTY(UI_PROPERTY_STATE, "y", dbus_get_state_cb, 0, BUS_COMMON_FLAGS | SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_PROPERTY(UI_PROPERTY_STATE_NAME, "s", dbus_get_state_name_cb, 0, BUS_COMMON_FLAGS),
    /*SD_BUS_SIGNAL_WITH_NAMES(UI_SIGNAL_OPEN_DOOR
        , "sst", SD_BUS_PARAM("cardValue")
                 SD_BUS_PARAM("cardPin")
                 SD_BUS_PARAM("requestIdentifier")
        , BUS_COMMON_FLAGS),*/
    SD_BUS_VTABLE_END
};

int dbus_loop() {
    sd_bus_message *m = NULL;
    int r;
    /* Process requests */
    r = sd_bus_process(bus, &m);
    if (r < 0) {
        selfLogErr("Failed to process bus: %s", strerror(-r));
        if(m) {
            uint8_t type = _SD_BUS_MESSAGE_TYPE_INVALID;
            char sType;
            switch(type) {
                case SD_BUS_MESSAGE_METHOD_CALL: sType = 'C'; break;
                case SD_BUS_MESSAGE_METHOD_RETURN: sType = 'C'; break;
                case SD_BUS_MESSAGE_METHOD_ERROR: sType = 'C'; break;
                case SD_BUS_MESSAGE_SIGNAL: sType = 'C'; break;
                default: sType = '?'; break;
            }
            sd_bus_message_get_type(m, &type);
            selfLogDbg("%s => %s %c [%s]",
                sd_bus_message_get_sender(m),
                sd_bus_message_get_destination(m),
                sd_bus_message_get_member(m),
                sType
            );
        }
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
    int r;
    const char* uniqueName;
    DbusTaskData *taskData = (DbusTaskData *)data;

    selfLogDbg("Opening bus...");
    if(taskData->useSysBus) {
        r = sd_bus_open_system(&bus);
    } else {
        r = sd_bus_open(&bus);
    }
    if(r < 0) {
        selfLogErr("Failed to open %s bus(%d): %s", taskData->useSysBus ? "system" : "session", r, strerror(-r));
        g_task_return_new_error(t, qDbus, r, "Failed to open bus(%d): %s", r, strerror(-r));
        sem_post(&mutex);
        return;
    }

    selfLogDbg("Adding vtable...");
    r = sd_bus_add_object_vtable(bus,
                                NULL,
                                DBUS_THIS_PATH,
                                DBUS_THIS_INTERFACE,
                                vtableInterface,
                                taskData);
    if (r < 0) {
        // Interface registration error
        selfLogErr("Failed to issue interface(%d): %s", r, strerror(-r));
        g_task_return_new_error(t, qDbus, r, "Failed to issue interface(%d): %s", r, strerror(-r));
        sem_post(&mutex);
        return;
    }

    selfLogDbg("Request dbus name...");
    // Take a well-known service name so that clients can find us
    r = sd_bus_request_name(bus, DBUS_THIS_NAME, 0);
    if (r < 0) {
        selfLogErr("Failed to acquire service name(%d): %s", r, strerror(-r));
        g_task_return_new_error(t, qDbus, r, "Failed to acquire service name(%d): %s", r, strerror(-r));
        sem_post(&mutex);
        return;
    }

    r = sd_bus_get_unique_name(bus, &uniqueName);
    if (r < 0) {
        selfLogDbg("Unique name error(%d): %s", r, strerror(-r));
    } else {
        selfLogDbg("Unique name %s", uniqueName);
    }


    selfLogDbg("Adding signal match...");
    // Subscribe GW : onUiData
    r = sd_bus_match_signal(bus, NULL, NULL, DBUS_GW_PATH, DBUS_GW_IFACE_UI, GW_SIG_UI_DATA, dbus_on_ui_data_handler, taskData);
    if (r < 0) {
        // GW:onUiData subscription error
        selfLogErr("Failed to issue interface(%d): %s", r, strerror(-r));
        g_task_return_new_error(t, qDbus, r, "Failed to issue interface(%d): %s", r, strerror(-r));
        sem_post(&mutex);
        return;
    }

    selfLogDbg("Release mutex");
    sem_post(&mutex);

    // Dbus main loop
    selfLogDbg("Dbus task: starting loop...");
    while(TRUE) {
        dbus_loop();
    }
}

/**
 * @brief Initialize D-bus UI-service
 *
 * @param manager pointer to ViewManager
 *
 * @return int error code
 */
int dbus_init(ViewManager *manager, int useSysBus) {
    // Check required argument
    if(!manager) return EINVAL;

    // Remember viewManager
    viewManager = manager;

    sem_init(&mutex, 0, 1);

    // Allocate memory for task data
    selfLogDbg("Allocating TaskData...");
    DbusTaskData *taskData = (DbusTaskData*)g_malloc0(sizeof(DbusTaskData));
    selfLogDbg("Allocated TaskData %p", taskData);
    if(!taskData) return ENOMEM;

    // Fill task data
    taskData->viewManager = manager;
    taskData->mainContext = g_main_context_default();
    taskData->useSysBus = useSysBus;

    // Create quark for errors in dbus loop
    qDbus = g_quark_from_string("DBus");

    // Create d-bus loop task
    selfLogDbg("Locking mutex...");
    sem_wait(&mutex);
    selfLogDbg("Creating dbus task...");
    taskDbus = g_task_new (NULL, NULL, dbus_stop, taskData);
    if(taskDbus) {
        // Add data to d-bus loop task
        selfLogDbg("Setting dbus task data...");
        g_task_set_task_data(taskDbus, taskData, g_free);
        // Start d-bus loop task
        selfLogDbg("Running dbus task...");
        g_task_run_in_thread(taskDbus, dbus_thread);
    } else {
        selfLogErr("Error create dbus task");
        return ENOENT;
    }

    selfLogDbg("Waiting for mutex...");
    sem_wait(&mutex);

    selfLogDbg("Dbus init finished!");
    return 0;
}

void dbus_emit_state () {
    if(!viewManager) return;
    int r = sd_bus_emit_properties_changed(bus, DBUS_THIS_PATH, DBUS_THIS_INTERFACE, UI_PROPERTY_STATE, NULL);
    if (r < 0) {
        selfLogErr("Failed(%d) to emit properties changed signal for `state`: %s", r, strerror(-r));
    } else {
        selfLogDbg("Emitted changed state: %s", viewManager->getStateName());
    }
}

GatewayStatus dbus_get_gw_status () {
    // Variables
    GatewayStatus ret = GW_Initializing;
    int r;
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message *ans;

    // Get GW:status property
    r = sd_bus_get_property(bus
        , DBUS_GW_NAME
        , DBUS_GW_PATH
        , DBUS_GW_IFACE_CONN
        , "status"
        , &err
        , &ans
        , "y");
    if(r < 0) {
        selfLogErr("Get GW status error(%d): %s", r, dbus_get_error(&err, r));
    } else {
        // Open modem list
        uint8_t u = 0;
        r = sd_bus_message_read_basic(ans, 'y', &u);
        if(r < 0) {
            selfLogErr("Read GW status error(%d): %s", r, strerror(-r));
        } else {
            ret = (GatewayStatus)u;
        }
    }
    return ret;
}