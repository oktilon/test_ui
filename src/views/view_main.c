#include <glib.h>
#include <stdlib.h>

#include "app.h"
#include "keypad.h"
#include "views/view_main.h"

static MdlDoorbell      *main_data;
static GtkApplication   *app;
static GtkWidget        *window;
static GtkWidget        *txtSearch = NULL;
static GtkWidget        *btnClose = NULL;
static GtkWidget        *main_flow_box = NULL;

static void on_back_button_clicked(GtkButton *btn, gpointer data) {
    gchar *txt;
    if(txtSearch) {
        txt = g_strdup_printf("%s%s", gtk_entry_get_text(GTK_ENTRY(txtSearch)), "<BACK");
        gtk_entry_set_text(GTK_ENTRY(txtSearch), txt);
        g_free(txt);
    }
}

static void on_close_button_clicked(GtkButton *btn, gpointer data) {
    g_application_quit(G_APPLICATION(app));
}

static void on_prev_button_click(GtkButton *_btn, gpointer _data) {
    selfLogDbg("prev btn click");
    keypad_show();
    GtkWidget *img;
    img = gtk_button_get_image(GTK_BUTTON(btnClose));
    gtk_image_set_from_resource(GTK_IMAGE(img), "/com/getdefigo/ui/src/resources/settingsIcon.png");
    selfLogDbg("Set close icon = settingIcon");
}

static void on_next_button_click(GtkButton *_btn, gpointer _data) {
    selfLogDbg("next btn click");
    keypad_hide();
}

static void on_clear_button_click(GtkButton *_btn, gpointer _data) {
    selfLogDbg("clear text");
    gtk_entry_set_text(GTK_ENTRY(txtSearch), "");
}


static gboolean on_search_focus_in(GtkEntry *_entry, GdkEventFocus _event, gpointer _user_data) {
    selfLogDbg("search focus in event");
    keypad_on_search_focus(TRUE);
    return FALSE;
}

static gboolean on_search_focus_out(GtkEntry *_entry, GdkEventFocus _event, gpointer user_data) {
    selfLogDbg("search focus out event");
    keypad_on_search_focus(FALSE);
    return FALSE;
}

static void on_unit_box_activated(GtkFlowBox *_flowbox, GtkFlowBoxChild* child, gpointer _user_data) {
    gint ix = gtk_flow_box_child_get_index(child);
    selfLogDbg("select unit ix=%d", ix);
}

static gboolean on_window_touch(GtkWidget *_w, GdkEventTouch ev) {
    // selfLogDbg("Touch at [%lf, %lf]", ev.x, ev.y);
    return FALSE;
}

void main_view_new_data(MdlDoorbell *data) {
    doorbell_clean(main_data);
    main_data = doorbell_clone(data);
}

AppState main_view_show(AppState old) {
    selfLogDbg("show main (%p)", window);
    if(!window) return old;
    gtk_widget_show(window);
    return UI_MainView;
}

void main_view_hide() {
    gtk_widget_hide(window);
}

gboolean main_view_init(GtkApplication *a) {
    // Variables
    GtkWidget *btnBack, *btnPrev, *btnNext, *btnClear;
    // Gtk builder
    GtkBuilder  *builder = gtk_builder_new_from_resource("/com/getdefigo/ui/src/ui/window.glade");
    if(!builder) {
        selfLogErr("View builder error: %m");
        return FALSE;
    }

    // Store app
    app = a;

    // Get window object
    window = GTK_WIDGET(gtk_builder_get_object(builder, "mainWindow"));
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
    // Get objects
    btnBack = GTK_WIDGET(gtk_builder_get_object(builder, "btnBack"));
    btnClose = GTK_WIDGET(gtk_builder_get_object(builder, "btnClose"));
    txtSearch = GTK_WIDGET(gtk_builder_get_object(builder, "searchEdit"));
    btnPrev = GTK_WIDGET(gtk_builder_get_object(builder, "btnPrev"));
    btnNext = GTK_WIDGET(gtk_builder_get_object(builder, "btnNext"));
    btnClear = GTK_WIDGET(gtk_builder_get_object(builder, "btnClear"));
    main_flow_box = GTK_WIDGET(gtk_builder_get_object(builder, "main_flow_box"));
    // Connect objects signals
    g_signal_connect(window, "touch-event", G_CALLBACK(on_window_touch), NULL);
    g_signal_connect(btnBack, "clicked", G_CALLBACK(on_back_button_clicked), NULL);
    g_signal_connect(btnClose, "clicked", G_CALLBACK(on_close_button_clicked), NULL);
    g_signal_connect(btnPrev, "clicked", G_CALLBACK(on_prev_button_click), NULL);
    g_signal_connect(btnNext, "clicked", G_CALLBACK(on_next_button_click), NULL);
    g_signal_connect(btnClear, "clicked", G_CALLBACK(on_clear_button_click), NULL);

    // g_signal_connect(txtSearch, "focus", G_CALLBACK(on_search_focus), NULL);
    g_signal_connect(txtSearch, "focus-in-event", G_CALLBACK(on_search_focus_in), NULL);
    g_signal_connect(txtSearch, "focus-out-event", G_CALLBACK(on_search_focus_out), NULL);

    g_signal_connect(main_flow_box, "child-activated", G_CALLBACK(on_unit_box_activated), NULL);

    // Init keypad
    keypad_init(builder, txtSearch);

    selfLogDbg("init main finished (%p)", window);

    return TRUE;
}