#ifndef VIEW_MANAGER_H
#define VIEW_MANAGER_H

#include <glib.h>
#include <gtk/gtk.h>
#include <gio/gio.h>

#include "app.h"
#include "view.h"
#include "view_history.h"

#define VIEW_CAT(a, b)    a##b
#define CREATE_VIEW(n)                      \
    {                                       \
        .name = #n,                         \
        .valid = FALSE,                     \
        .init = VIEW_CAT(n, _view_init),    \
        .show = VIEW_CAT(n, _view_show),    \
        .hide = VIEW_CAT(n, _view_hide)     \
    }
#define LAST_VIEW()                 \
    {                               \
        .name = NULL,               \
        .valid = FALSE,             \
        .init = view_init_dummy,    \
        .show = view_show_dummy,    \
        .hide = view_hide_dummy     \
    }


typedef struct ViewManagerStruct {
    const gchar *(*getCurrentViewName)();
    AppState (*getState)();
    const gchar *(*getStateName)();
    gboolean (*onNewData)(gpointer data);
} ViewManager;

ViewManager *view_manager_init(GtkApplication *app);
int view_manager_show(const gchar *viewName);
void view_manager_start(GatewayStatus status);
void view_manager_set_state(AppState newState);

#endif // VIEW_MANAGER_H