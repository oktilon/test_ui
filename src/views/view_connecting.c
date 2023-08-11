#include <glib.h>
#include <stdlib.h>

#include "app.h"
#include "keypad.h"
#include "views/view_connecting.h"

static GtkWidget *window;

AppState connecting_view_show(AppState old) {
    selfLogDbg("show connecting (%p)", window);
    if(!window) return old;
    gtk_widget_show(window);
    return UI_Connecting;
}

void connecting_view_hide() {
    if(window)
        gtk_widget_hide(window);
}

gboolean connecting_view_init(GtkApplication *app) {
    // Gtk builder
    GtkBuilder  *builder = gtk_builder_new_from_resource("/com/getdefigo/ui/src/ui/connecting.glade");
    if(!builder) {
        selfLogErr("View builder error: %m");
        return FALSE;
    }

    // Get window object
    window = GTK_WIDGET(gtk_builder_get_object(builder, "connectingWindow"));
    if(!window) {
        selfLogErr("Get view window error: %m");
        return FALSE;
    }

    // Set window to app
    gtk_window_set_application(GTK_WINDOW(window), app);
    // Set fullscreen
    gtk_window_fullscreen(GTK_WINDOW(window));
    // Connect signals (glade file)
    gtk_builder_connect_signals(builder, NULL);

    selfLogDbg("init connecting finished (%p)", window);

    return TRUE;
}