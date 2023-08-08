#include <gtk/gtk.h>
int main (int argc, char *argv[])
{
    GtkWidget *window;// GtkWidget is the storage type for widgets
    gtk_init (&argc, &argv);// This is called in all GTK applications. Arguments are parsed
     			        //from the command line and are returned to the application.
    window=gtk_window_new(GTK_WINDOW_TOPLEVEL); //creating a new window.
    gtk_widget_show(window);
    gtk_main (); //All GTK applications must have a gtk_main(). Control ends here
     	               //and waits for an event to occur (like a key press or mouse event).
   return 0;
}
