#ifndef VIEW_CONNECTING_H
#define VIEW_CONNECTING_H

#include <gtk/gtk.h>
#include "app.h"

gboolean connecting_view_init(GtkApplication *app);
AppState connecting_view_show(AppState);
void connecting_view_hide();

#endif // VIEW_CONNECTING_H