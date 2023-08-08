#ifndef VIEW_MAIN_H
#define VIEW_MAIN_H

#include <gtk/gtk.h>
#include "models/mdl_doorbell.h"

void main_view_init(GtkApplication *app);
void main_view_show();
void main_view_new_data(MdlDoorbell *data);

#endif // VIEW_MAIN_H