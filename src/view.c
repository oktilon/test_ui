#include "app.h"
#include "view.h"

gboolean view_init_dummy(GtkApplication *app) { return FALSE; }
AppState view_show_dummy(AppState) { return UI_Initializing; }
void view_hide_dummy() {}
