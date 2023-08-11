#include <glib.h>

#include "app.h"
#include "dbus.h"
#include "view_manager.h"
#include "views/view_main.h"
#include "views/view_connecting.h"

static ViewManager      *manager;
static GtkApplication   *app;
static View             *activeView;
static AppState         state;

static View views[] = {
      CREATE_VIEW(connecting)
    , CREATE_VIEW(main)
    , LAST_VIEW()
};

gboolean view_manager_on_new_data(gpointer data) {
    MdlDoorbell *mdl = (MdlDoorbell *)data;
    selfLogInf("New data: id=%d, bid=%d, pubName=%s, addr=%s", mdl->id, mdl->buildingId, mdl->publicName, mdl->address);
    // g_signal_emit();
    return FALSE;
}

const gchar *view_manager_state_name(AppState state) {
    switch(state) {
        case UI_Initializing:   return "Initialization";
        case UI_Connecting:     return "Connecting";
        case UI_MainView:       return "MainView";
        default:                return "Unknown";
    }
    return "";
}

const gchar *view_manager_get_current_view_name() {
    if(activeView) return activeView->name;
    return "";
}

AppState view_manager_get_state() {
    return state;
}

const gchar *view_manager_get_state_name() {
    return view_manager_state_name(state);
}


ViewManager *view_manager_init(GtkApplication *_app) {
    selfLogDbg("View manager init");
    // Remember app
    app = _app;
    manager = (ViewManager *) g_malloc0(sizeof(ViewManager));
    if(!manager) {
        selfLogErr("Allocate memory error");
        return NULL;
    }

    // Functions
    manager->onNewData = view_manager_on_new_data;
    manager->getCurrentViewName = view_manager_get_current_view_name;
    manager->getState = view_manager_get_state;
    manager->getStateName = view_manager_get_state_name;

    view_manager_set_state(UI_Initializing);

    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(css, "/com/getdefigo/ui/src/ui/window.css");

    GdkScreen *screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Init all views
    for(View* it = views; it->name; it++) {
        selfLogDbg("Init view %s", it->name);
        it->valid = it->init(app);
    }

    return manager;
}

View *view_manager_get(const gchar *viewName) {
    View *ret = NULL;
    for(View *it = views; it; it++) {
        if(g_strcmp0(it->name, viewName) == 0) {
            ret = it;
            break;
        }
    }
    return ret;
}

int view_manager_show(const gchar *viewName) {
    selfLogDbg("Show view %s", viewName);
    if(!manager) {
        selfLogErr("Manager not initialized");
        return -ENODEV;
    }
    View *view = view_manager_get(viewName);
    if(!view) {
        selfLogDbg("Unknown view: %s", viewName);
        return -EINVAL;
    }
    if(!view->valid) {
        selfLogDbg("View %s not initialized", viewName);
        return -EINVAL;
    }
    selfLogDbg("Show view: %s", view->name);
    view_manager_set_state(view->show(state));
    return 0;
}

void view_manager_start(GatewayStatus status) {
    if(status == GW_Online) {
        view_manager_show("main");
    } else {
        view_manager_show("connecting");
    }
}

void view_manager_set_state(AppState newState) {
    if(state == newState) return;
    state = newState;
    dbus_emit_state();
}