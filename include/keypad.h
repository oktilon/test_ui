#ifndef KEYPAD_H
#define KEYPAD_H
#include <gtk/gtk.h>
#include <glib.h>

void keypad_init(GtkBuilder *builder, GtkWidget *entry);
void keypad_show();
void keypad_hide();
void keypad_on_search_focus(gboolean enter);

gboolean keypad_timeout_hide(gpointer userData);
gboolean keypad_timeout_lost_focus(gpointer userData);
void keypad_start_hide_timer();
void keypad_stop_hide_timer();
void keypad_start_wait_focus();
void keypad_stop_wait_focus();

#endif // KEYPAD_H