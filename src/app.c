#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <glib.h>
#include <sqlite3.h>
#include <stdio.h>
#include <sys/time.h>
#include <getopt.h>
#include "as-resources.h"
#include "app.h"
#include "dbus.h"
#include "view_manager.h"

static GtkApplication   *app;
gchar buffer[BUFFER_SIZE];
sqlite3 *db         = NULL;              // Sqlite3 DB pointer (log to file db)
int gLogLevel       = LOG_LEVEL_INFO;    // Logging level
int useSysBus       = 1;

const char *logLevelHeaders[] = {
    "\033[1;31mERR\033[0m",  // LOG_LEVEL_ERROR
    "\033[1;37mINF\033[0m",  // LOG_LEVEL_INFO
    "\033[1;33mDBG\033[0m"   // LOG_LEVEL_DEBUG
};

const char *logLevelColor[] = {
    "\033[0;31m",  // LOG_LEVEL_ERROR
    "\033[0;37m",  // LOG_LEVEL_INFO
    "\033[0;33m"   // LOG_LEVEL_DEBUG
};

// Long command line options
const struct option longOptions[] = {
    {"verbose",         no_argument,  0,  'v'},
    {"wayland-debug",   no_argument,  0,  'w'},
    {"quiet",           no_argument,  0,  'q'},
    {"user",            no_argument,  0,  'u'}
};

/**
 * @brief Logging main body
 *
 * @param file current file
 * @param line current line
 * @param func current function
 * @param lvl debug (verbosity) level
 * @param fmt log format
 * @param argp log other arguments
 */
void selfLogFunction(const char *file, int line, const char *func, int lvl, const char* fmt, ...) {
    // Variables
    int r;
    size_t sz;
    sqlite3_stmt *db_st = NULL;
    char buf[60] = {0};
    char msec[30] = {0};
    struct timeval tv;
    struct tm t = {0};

    // Create timestamp with ms
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &t);
    r = snprintf(msec, 30, "%ld", tv.tv_usec / 1000);
    for(; r < 3; r++) strcat(msec, "0");

    sz = strftime(buf, sizeof(buf), "%F %T", &t); // %F => %Y-%m-%d,  %T => %H:%M:%S
    snprintf(buf + sz, 60, ".%s", msec);
    char *msg = NULL;

    // Connect to log db
    if(!db) {
        r = sqlite3_open("/var/opt/defigo/gateway.db", &db);
        if(r) {
            sqlite3_close(db);
            db = NULL;
        } else {
            // Create log table if not exists
            r = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS log ( \
                  id INTEGER \
                , lvl INTEGER \
                , dt TEXT \
                , file TEXT \
                , func TEXT \
                , line INTEGER \
                , msg TEXT \
                , PRIMARY KEY(id AUTOINCREMENT) \
                );", NULL, NULL, NULL);
            if(r != SQLITE_OK) {
                sqlite3_close(db);
                db = NULL;
            }
        }
    }

    // Format log message
    va_list arglist;
    va_start(arglist, fmt);
    r = vasprintf(&msg, fmt, arglist);
    va_end(arglist);

    // Write log to DB
    if(db) {
        r = sqlite3_prepare(db, "INSERT INTO log(lvl, dt, file, func, line, msg) VALUES (?,?,?,?,?,?);", -1, &db_st, NULL);
        if(r == SQLITE_OK) {

            r = sqlite3_bind_int( db_st, 1, lvl);
            r = sqlite3_bind_text(db_st, 2, buf, -1, NULL);
            r = sqlite3_bind_text(db_st, 3, file, -1, NULL);
            r = sqlite3_bind_text(db_st, 4, func, -1, NULL);
            r = sqlite3_bind_int( db_st, 5, line);
            r = sqlite3_bind_text(db_st, 6, msg, -1, NULL);

            r = sqlite3_step(db_st);
        }
        (void)sqlite3_finalize(db_st);
        db_st = NULL;
    }

    // Output log to stdout
    if(lvl <= gLogLevel) {
        printf("%s: [%s] %s %s%s\033[0m\n", buf, logLevelHeaders[lvl], func, logLevelColor[lvl], msg);
        fflush(stdout);
    }

    // Free formatted log buffer
    free(msg);
}

void on_app_activate(GtkApplication *_app) {
    GatewayStatus gwStatus = GW_Initializing;

    ViewManager* manager = view_manager_init(app);
    view_manager_show("connecting");

    selfLogInf("Initializing dbus");
    int r = dbus_init(manager, useSysBus);
    if(r == 0) {
        gwStatus = dbus_get_gw_status();
        view_manager_start(gwStatus);
    } else {
        selfLogErr("DBus init error");
    }
}

int main(int argc, char **argv) {
    // Variables
    int status, i;

    // ???? Unused`
    memset(buffer, 0, BUFFER_SIZE);

    // Parse cmdline options
    while ((i = getopt_long(argc, argv, "vwqs", longOptions, NULL)) != -1) {
        switch(i) {
            case 'v': // verbose
                gLogLevel = LOG_LEVEL_DEBUG;
                break;

            case 'q': // quiet
                gLogLevel = LOG_LEVEL_ERROR;
                break;

            case 'u': // session bus (user bus)
                useSysBus = 0;
                break;

            case 'w': // wayland-debug
                setenv("WAYLAND_DEBUG", "1", 1);
                break;

            default:
                break;
        }
    }

    app = gtk_application_new("com.getdefigo.ui", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), NULL);

    selfLogInf("Start UI...");
    status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);
    // Stop dbus task?

    selfLogErr("Stop with code %d", status);
    return status;
}