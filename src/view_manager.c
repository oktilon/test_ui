#include <glib.h>

#include "app.h"
#include "view_manager.h"

static ViewManager *manager;

gboolean view_manager_is_active(gchar *name) {
    if(g_strcmp0(manager->currentViewName, name) == 0) {
        return TRUE;
    }
    return FALSE;
}

gboolean view_manager_on_new_data(gpointer data) {
    //
}

ViewManager *view_manager_init(GtkApplication *app) {
    selfLogDbg("Try to allocate memory...");
    manager = (ViewManager *) g_malloc0(sizeof(ViewManager));
    if(manager) {
        selfLogDbg("Memory is allocated (%p)", manager);
        manager->app = app;
        manager->currentViewName = "";
        manager->state = "init";
        // Fuuncs
        manager->isActive = view_manager_is_active;
        manager->onNewData = view_manager_on_new_data;
    } else {
        selfLogErr("Allocate memory error");
    }
    return manager;
}

int view_manager_show(const gchar *viewName) {
    selfLogDbg("Show view %s", viewName);
    if(!manager) {
        selfLogErr("Manager not initialized");
        return -ENODEV;
    }
    if(g_strcmp0(viewName, "main") == 0) {
        selfLogDbg("Initializing main view...");
        main_view_init(manager->app);
        selfLogDbg("Starting main view...");
        main_view_show();
        manager->currentViewName = viewName;
        manager->state = "main";
        selfLogDbg("Main view started");
        return 0;
    }
    return -EINVAL;
}
