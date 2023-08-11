#ifndef VIEW_MAIN_H
#define VIEW_MAIN_H

#include <glib.h>
#include <gtk/gtk.h>
#include "view.h"
#include "models/mdl_doorbell.h"

gboolean main_view_init(GtkApplication *app);
AppState main_view_show(AppState);
void main_view_hide();
void main_view_new_data(MdlDoorbell *data);

#endif // VIEW_MAIN_H