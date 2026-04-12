#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

enum {
    COLUMN_ICON,
    COLUMN_NAME,
    COLUMN_SIZE,
    COLUMN_SIZE_BYTES,
    NUM_COLUMNS
};

typedef struct {
    GtkTreeStore *store;
    GtkWidget *tree_view;
    GFile *current_root;
    GtkWidget *window;
    GtkWidget *statusbar;
    guint context_id;
    int file_count;
} AppData;

static void fill_tree_recursive(AppData *app, GtkTreeIter *parent, GFile *dir) {
    GFileEnumerator *enumerator = g_file_enumerate_children(dir, 
        "standard::name,standard::type,standard::size,standard::icon",
        G_FILE_QUERY_INFO_NONE, NULL, NULL);

    if (!enumerator) return;

    GFileInfo *info;
    while ((info = g_file_enumerator_next_file(enumerator, NULL, NULL)) != NULL) {
        const char *name = g_file_info_get_name(info);
        GFileType type = g_file_info_get_file_type(info);
        goffset size = g_file_info_get_size(info);
        
        app->file_count++;

        GIcon *gicon = g_file_info_get_icon(info);
        GdkPixbuf *pixbuf = NULL;
        if (gicon) {
            GtkIconTheme *theme = gtk_icon_theme_get_for_screen(gtk_widget_get_screen(app->tree_view));
            GtkIconInfo *icon_info = gtk_icon_theme_lookup_by_gicon(theme, gicon, 16, GTK_ICON_LOOKUP_USE_BUILTIN);
            if (icon_info) {
                pixbuf = gtk_icon_info_load_icon(icon_info, NULL);
                g_object_unref(icon_info);
            }
        }

        char *size_str = (type == G_FILE_TYPE_DIRECTORY) ? g_strdup("-") : g_format_size((guint64)size);

        GtkTreeIter iter;
        gtk_tree_store_append(app->store, &iter, parent);
        gtk_tree_store_set(app->store, &iter, 
                           COLUMN_ICON, pixbuf, COLUMN_NAME, name, 
                           COLUMN_SIZE, size_str, COLUMN_SIZE_BYTES, (guint64)size, -1);

        if (pixbuf) g_object_unref(pixbuf);
        g_free(size_str);

        if (type == G_FILE_TYPE_DIRECTORY) {
            GFile *child = g_file_get_child(dir, name);
            fill_tree_recursive(app, &iter, child);
            g_object_unref(child);
        }
        g_object_unref(info);
    }
    g_file_enumerator_close(enumerator, NULL, NULL);
    g_object_unref(enumerator);
}

static void refresh_view(AppData *app) {
    app->file_count = 0;
    gtk_tree_store_clear(app->store);
    char *path = g_file_get_path(app->current_root);
    gtk_window_set_title(GTK_WINDOW(app->window), path);
    g_free(path);

    fill_tree_recursive(app, NULL, app->current_root);

    gtk_statusbar_pop(GTK_STATUSBAR(app->statusbar), app->context_id);
    char *status_msg = g_strdup_printf("Объектов: %d (Esc для выхода)", app->file_count);
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->context_id, status_msg);
    g_free(status_msg);
}

static gboolean on_key_press(GtkWidget *widget G_GNUC_UNUSED, GdkEventKey *event, gpointer data G_GNUC_UNUSED) {
    if (event->keyval == GDK_KEY_Escape) {
        gtk_main_quit();
        return TRUE;
    }
    return FALSE;
}

static void on_row_activated(GtkTreeView *tv, GtkTreePath *path, GtkTreeViewColumn *col G_GNUC_UNUSED, gpointer data) {
    AppData *app = (AppData *)data;
    GtkTreeIter iter;
    if (gtk_tree_model_get_iter(gtk_tree_view_get_model(tv), &iter, path)) {
        char *name;
        gtk_tree_model_get(gtk_tree_view_get_model(tv), &iter, COLUMN_NAME, &name, -1);
        GFile *new_dir = g_file_get_child(app->current_root, name);
        GFileInfo *info = g_file_query_info(new_dir, "standard::type", 0, NULL, NULL);
        if (info && g_file_info_get_file_type(info) == G_FILE_TYPE_DIRECTORY) {
            g_object_unref(app->current_root);
            app->current_root = new_dir;
            refresh_view(app);
        } else {
            g_object_unref(new_dir);
        }
        if (info) g_object_unref(info);
        g_free(name);
    }
}

static void on_up_clicked(GtkButton *btn G_GNUC_UNUSED, gpointer data) {
    AppData *app = (AppData *)data;
    GFile *parent = g_file_get_parent(app->current_root);
    if (parent) {
        g_object_unref(app->current_root);
        app->current_root = parent;
        refresh_view(app);
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    AppData app;
    app.current_root = g_file_new_for_path(".");
    app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(app.window), 500, 600);
    g_signal_connect(app.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(app.window, "key-press-event", G_CALLBACK(on_key_press), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *btn_up = gtk_button_new_with_label("⬆ На уровень выше");
    g_signal_connect(btn_up, "clicked", G_CALLBACK(on_up_clicked), &app);
    gtk_box_pack_start(GTK_BOX(vbox), btn_up, FALSE, FALSE, 5);

    app.store = gtk_tree_store_new(NUM_COLUMNS, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT64);
    app.tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(app.store));
    g_object_unref(app.store);

    GtkTreeViewColumn *c1 = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(c1, "Имя");
    GtkCellRenderer *r1 = gtk_cell_renderer_pixbuf_new();
    GtkCellRenderer *r2 = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(c1, r1, FALSE);
    gtk_tree_view_column_pack_start(c1, r2, TRUE);
    gtk_tree_view_column_add_attribute(c1, r1, "pixbuf", COLUMN_ICON);
    gtk_tree_view_column_add_attribute(c1, r2, "text", COLUMN_NAME);
    gtk_tree_view_column_set_sort_column_id(c1, COLUMN_NAME);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app.tree_view), c1);

    GtkCellRenderer *r3 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *c2 = gtk_tree_view_column_new_with_attributes("Размер", r3, "text", COLUMN_SIZE, NULL);
    gtk_tree_view_column_set_sort_column_id(c2, COLUMN_SIZE_BYTES);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app.tree_view), c2);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), app.tree_view);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    app.statusbar = gtk_statusbar_new();
    app.context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(app.statusbar), "status");
    gtk_box_pack_start(GTK_BOX(vbox), app.statusbar, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(app.window), vbox);
    g_signal_connect(app.tree_view, "row-activated", G_CALLBACK(on_row_activated), &app);

    refresh_view(&app);
    gtk_widget_show_all(app.window);
    gtk_main();

    g_object_unref(app.current_root);
    return 0;
}
