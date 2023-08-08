#include <stdlib.h>
#include "app.h"
#include "keypad.h"

struct key_data {
    gint id;
    GtkWidget *entry;
};

static gint key_w = 72,
            key_h = 60,
            key_ws = 7,
            key_hs = 7,
            row0_of = 8,
            row1_of = 48,
            row2_of = 127;
static const gchar keys_en[26] = "qwertyuiopasdfghjklzxcvbnm";
static gchar single_char[2]    = {'a', '\0'};
static GtkWidget *keypad;
static GtkWidget *keypad_fixed;
static guint     timerHide = 0;
static guint     timerFocus = 0;

static void on_keypad_button_clicked(GtkWidget *button, struct key_data *user_data) {
    // gchar *string;
    single_char[0] = keys_en[user_data->id];
    // GtkEntryBuffer *buf = gtk_entry_get_buffer(GTK_ENTRY(user_data->entry));
    gint beg = -1, end = -1;
    gboolean hasSel = gtk_editable_get_selection_bounds(GTK_EDITABLE(user_data->entry), &beg, &end);
    if(hasSel) {
        g_print("Button index %i=%s sel=(%d, %d)\n", user_data->id, single_char, beg, end);
        gtk_editable_delete_selection(GTK_EDITABLE(user_data->entry));
    } else {
        g_print("Button index %i=%s no-sel\n", user_data->id, single_char);
    }
    gint pos = gtk_editable_get_position(GTK_EDITABLE(user_data->entry));
    g_print("Insert text at %d\n", pos);
    gtk_editable_insert_text(GTK_EDITABLE(user_data->entry), single_char, 1, &pos);
    // gtk_entry_buffer_insert_text(buf, pos, single_char, 1);
    gtk_editable_set_position(GTK_EDITABLE(user_data->entry), pos);
    // string = g_strdup_printf("%s%s", gtk_entry_get_text(GTK_ENTRY(user_data->entry)), single_char);
    // gtk_entry_set_text(GTK_ENTRY(user_data->entry), string);
    // g_free(string);
    keypad_stop_wait_focus();
}

gboolean keypad_timeout_hide(gpointer userData) {
    selfLogDbg("keypad: timeout hide");
    keypad_hide();
    return FALSE;
}

gboolean keypad_timeout_lost_focus(gpointer userData) {
    selfLogDbg("keypad: timeout lost focus");
    keypad_hide();
    return FALSE;
}

void keypad_start_hide_timer() {
    keypad_stop_hide_timer();
    selfLogDbg("keypad: start hide timer");
    timerHide = g_timeout_add(5000, *keypad_timeout_hide, NULL);
}

void keypad_stop_hide_timer() {
    if(timerHide) {
        selfLogDbg("keypad: remove hide timer");
        g_source_remove(timerHide);
        timerHide = 0;
    }
}

void keypad_start_wait_focus() {
    keypad_stop_wait_focus();
    selfLogDbg("keypad: start focus timer");
    timerFocus = g_timeout_add(500, *keypad_timeout_lost_focus, NULL);
}

void keypad_stop_wait_focus() {
    if(timerFocus) {
        selfLogDbg("keypad: remove focus timer");
        g_source_remove(timerFocus);
        timerFocus = 0;
    }
}

void keypad_init(GtkBuilder *builder, GtkWidget *entry) {
    gint i = 0, x = row0_of, y = key_hs;
    struct key_data *puser_data;
    GtkWidget *btn_key;
    GtkStyleContext *style_cntx;
    keypad = GTK_WIDGET(gtk_builder_get_object(builder, "keypad"));
    keypad_fixed = GTK_WIDGET(gtk_builder_get_object(builder, "keypad_fixed"));
    for(i = 0; i < 26; i++) {
        single_char[0] = keys_en[i];
        btn_key = gtk_button_new_with_label(single_char);
        puser_data = (struct key_data *)calloc(1, sizeof(struct key_data));
        puser_data->id = i;
        puser_data->entry = entry;
        gtk_widget_set_size_request(btn_key, key_w, key_h);
        style_cntx = gtk_widget_get_style_context(btn_key);
        gtk_style_context_add_class(style_cntx, "keypad_key");
        gtk_fixed_put(GTK_FIXED(keypad_fixed), btn_key, x, y);
        g_signal_connect(btn_key, "clicked", G_CALLBACK(on_keypad_button_clicked), puser_data);
        x += key_w + key_ws;
        if(i == 9 || i == 18) {
            y += key_h + key_hs;
            x = i == 9 ? row1_of : row2_of;
        }
    }
}

void keypad_show() {
    if(keypad_fixed) {
        gtk_widget_show_all(keypad_fixed);
    }
    if(keypad) {
        gtk_widget_set_visible(keypad, TRUE);
    }
}

void keypad_hide() {
    if(keypad_fixed) {
        gtk_widget_hide(keypad_fixed);
    }
    if(keypad) {
        gtk_widget_set_visible(keypad, FALSE);
    }
    keypad_stop_hide_timer();
    keypad_stop_wait_focus();
}

void keypad_on_search_focus(gboolean enter) {
    if(enter) {
        keypad_stop_wait_focus();
        if(!gtk_widget_is_visible(keypad)) {
            keypad_show();
        }
    } else {
        keypad_start_wait_focus();
    }
}