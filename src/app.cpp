#include <gtk/gtk.h>

GtkWidget *lblInfo = nullptr;
GtkWidget *lblInfo = nullptr;

static void on_back_button_clicked(GtkButton *btn, gpointer data) {
    // change button label when it's clicked
    if(lblInfo) {
        gtk_label_set_label(GTK_LABEL(lblInfo), (const gchar*)"<<< BACK <<<");
    }
}

static void on_close_button_clicked(GtkButton *btn, gpointer data) {
    // change button label when it's clicked
    if(lblInfo) {
        gtk_label_set_label(GTK_LABEL(lblInfo), (const gchar*)"xxx CLOSE xxx");
    }
    gtk_main_quit();
}

//
// Should provided the active view for a GTK application
//
// static void activate(GtkApplication* app, gpointer user_data)
// {
//     GtkWidget *window;
//     window = gtk_application_window_new(GTK_APPLICATION(app));

//     GtkWidget *btn = gtk_button_new_with_label("Click Me!");
//     gtk_container_add(GTK_CONTAINER(window), btn);

//     gtk_widget_show_all(window);
// } // end of function activate

//
// main is where all program execution starts
//
int main(int argc, char **argv)
{
    // GtkApplication *app;
    // int status;

    // app = gtk_application_new("com.defigo.ui", G_APPLICATION_FLAGS_NONE);
    // g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    // status = g_application_run(G_APPLICATION(app), argc, argv);
    // g_object_unref(app);
    gtk_init(&argc, &argv);


    GtkWidget *window, *btnBack, *btnClose;
    GtkBuilder *builder = gtk_builder_new_from_file("/home/denis/projects/defigo/services/ui/src/ui/window.glade");

    window = GTK_WIDGET(gtk_builder_get_object(builder, "mainWindow"));
    printf("window=%p\n", window);
    // g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_builder_connect_signals(builder, NULL);

    btnBack = GTK_WIDGET(gtk_builder_get_object(builder, "btnBack"));
    printf("btnBack=%p\n", btnBack);
    btnClose = GTK_WIDGET(gtk_builder_get_object(builder, "btnClose"));
    printf("btnClose=%p\n", btnClose);
    lblInfo = GTK_WIDGET(gtk_builder_get_object(builder, "lblInfo"));
    printf("lblInfo=%p\n", lblInfo);

    g_signal_connect(btnBack, "clicked", G_CALLBACK(on_back_button_clicked), NULL);
    g_signal_connect(btnClose, "clicked", G_CALLBACK(on_close_button_clicked), NULL);

    gtk_widget_show(window);
    gtk_main();

    printf("finish\n");
    return EXIT_SUCCESS;
} // end of function main