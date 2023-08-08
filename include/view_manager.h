#ifndef VIEW_MANAGER_H
#define VIEW_MANAGER_H

#include <gtk/gtk.h>
#include <gio/gio.h>

#include "view_main.h"

typedef struct ViewManagerStruct {
    GtkApplication *app;
    const gchar *currentViewName;
    const gchar *state;
    // Method handlers
    gboolean (*isActive)(gchar *name);
    gboolean (*onNewData)(gpointer data);
} ViewManager;

ViewManager *view_manager_init(GtkApplication *app);
int view_manager_show(const gchar *viewName);

#endif // VIEW_MANAGER_H