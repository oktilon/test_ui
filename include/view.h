#ifndef VIEW_H
#define VIEW_H

#include <glib.h>
#include <gtk/gtk.h>
#include "app.h"

typedef struct ViewStruct {
    gchar       *name;
    gboolean    valid;
    // Handlers
    gboolean (*init)(GtkApplication *app);
    AppState (*show)(AppState);
    void (*hide)();
} View;

typedef struct ViewsHistoryStruct {
    View                        *view;
    struct ViewsHistoryStruct   *prev;
} ViewsHistory;

gboolean view_init_dummy(GtkApplication *app);
AppState view_show_dummy(AppState);
void view_hide_dummy();


#endif // VIEW_H