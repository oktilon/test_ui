#include <glib.h>
#include <stdlib.h>

#include "app.h"
#include "keypad.h"
#include "view_main.h"

MdlDoorbell *main_data;
static GtkApplication *app;
GtkWidget *txtSearch = NULL;
GtkWidget *window = NULL;
GtkWidget *keypad = NULL;
GtkWidget *keypad_fixed = NULL;
GtkWidget *btnClose = NULL;
GtkWidget *main_flow_box = NULL;

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

void main_view_show() {
    gtk_widget_show(window);
}

void main_view_init(GtkApplication *_app) {
    app = _app;

    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(css, "/com/getdefigo/ui/src/ui/window.css");

    GdkScreen *screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_USER);

    GtkWidget *btnBack, *btnPrev, *btnNext, *btnClear;
    GtkBuilder *builder = gtk_builder_new_from_resource("/com/getdefigo/ui/src/ui/window.glade");

    window = GTK_WIDGET(gtk_builder_get_object(builder, "mainWindow"));

    gtk_window_set_application(GTK_WINDOW(window), app);
    gtk_window_fullscreen(GTK_WINDOW(window));

    gtk_builder_connect_signals(builder, NULL);

    btnBack = GTK_WIDGET(gtk_builder_get_object(builder, "btnBack"));
    btnClose = GTK_WIDGET(gtk_builder_get_object(builder, "btnClose"));
    txtSearch = GTK_WIDGET(gtk_builder_get_object(builder, "searchEdit"));
    btnPrev = GTK_WIDGET(gtk_builder_get_object(builder, "btnPrev"));
    btnNext = GTK_WIDGET(gtk_builder_get_object(builder, "btnNext"));
    btnClear = GTK_WIDGET(gtk_builder_get_object(builder, "btnClear"));

    main_flow_box = GTK_WIDGET(gtk_builder_get_object(builder, "main_flow_box"));

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


    keypad_init(builder, txtSearch);
}