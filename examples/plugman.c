#include <stdlib.h>
#include <gtk/gtk.h>

static void
activate_toggle (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       user_data)
{
  GVariant *state;

  state = g_action_get_state (G_ACTION (action));
  g_action_change_state (G_ACTION (action), g_variant_new_boolean (!g_variant_get_boolean (state)));
  g_variant_unref (state);
}

static void
change_fullscreen_state (GSimpleAction *action,
                         GVariant      *state,
                         gpointer       user_data)
{
  if (g_variant_get_boolean (state))
    gtk_window_fullscreen (user_data);
  else
    gtk_window_unfullscreen (user_data);

  g_simple_action_set_state (action, state);
}

static GtkClipboard *
get_clipboard (GtkWidget *widget)
{
  return gtk_widget_get_clipboard (widget, gdk_atom_intern_static_string ("CLIPBOARD"));
}

static void
window_copy (GSimpleAction *action,
             GVariant      *parameter,
             gpointer       user_data)
{
  GtkWindow *window = GTK_WINDOW (user_data);
  GtkTextView *text = g_object_get_data ((GObject*)window, "plugman-text");

  gtk_text_buffer_copy_clipboard (gtk_text_view_get_buffer (text),
                                  get_clipboard ((GtkWidget*) text));
}

static void
window_paste (GSimpleAction *action,
              GVariant      *parameter,
              gpointer       user_data)
{
  GtkWindow *window = GTK_WINDOW (user_data);
  GtkTextView *text = g_object_get_data ((GObject*)window, "plugman-text");
  
  gtk_text_buffer_paste_clipboard (gtk_text_view_get_buffer (text),
                                   get_clipboard ((GtkWidget*) text),
                                   NULL,
                                   TRUE);

}

static GActionEntry win_entries[] = {
  { "copy", window_copy, NULL, NULL, NULL },
  { "paste", window_paste, NULL, NULL, NULL },
  { "fullscreen", activate_toggle, NULL, "false", change_fullscreen_state }
};

static void
new_window (GApplication *app,
            GFile        *file)
{
  GtkWidget *window, *grid, *scrolled, *view;

  window = gtk_application_window_new (GTK_APPLICATION (app));
  gtk_window_set_default_size ((GtkWindow*)window, 640, 480);
  g_action_map_add_action_entries (G_ACTION_MAP (window), win_entries, G_N_ELEMENTS (win_entries), window);
  gtk_window_set_title (GTK_WINDOW (window), "Plugman");

  grid = gtk_grid_new ();
  gtk_container_add (GTK_CONTAINER (window), grid);

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_hexpand (scrolled, TRUE);
  gtk_widget_set_vexpand (scrolled, TRUE);
  view = gtk_text_view_new ();

  g_object_set_data ((GObject*)window, "plugman-text", view);

  gtk_container_add (GTK_CONTAINER (scrolled), view);

  gtk_grid_attach (GTK_GRID (grid), scrolled, 0, 0, 1, 1);

  if (file != NULL)
    {
      gchar *contents;
      gsize length;

      if (g_file_load_contents (file, NULL, &contents, &length, NULL, NULL))
        {
          GtkTextBuffer *buffer;

          buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
          gtk_text_buffer_set_text (buffer, contents, length);
          g_free (contents);
        }
    }

  gtk_widget_show_all (GTK_WIDGET (window));
}

static void
plug_man_activate (GApplication *application)
{
  new_window (application, NULL);
}

static void
plug_man_open (GApplication  *application,
                GFile        **files,
                gint           n_files,
                const gchar   *hint)
{
  gint i;

  for (i = 0; i < n_files; i++)
    new_window (application, files[i]);
}

typedef GtkApplication PlugMan;
typedef GtkApplicationClass PlugManClass;

G_DEFINE_TYPE (PlugMan, plug_man, GTK_TYPE_APPLICATION)

static void
plug_man_finalize (GObject *object)
{
  G_OBJECT_CLASS (plug_man_parent_class)->finalize (object);
}

static void
show_about (GSimpleAction *action,
            GVariant      *parameter,
            gpointer       user_data)
{
  gtk_show_about_dialog (NULL,
                         "program-name", "Plugman",
                         "title", "About Plugman",
                         "comments", "A cheap Bloatpad clone.",
                         NULL);
}


static void
quit_app (GSimpleAction *action,
          GVariant      *parameter,
          gpointer       user_data)
{
  GList *list, *next;
  GtkWindow *win;

  g_print ("Going down...\n");

  list = gtk_application_get_windows (GTK_APPLICATION (g_application_get_default ()));
  while (list)
    {
      win = list->data;
      next = list->next;

      gtk_widget_destroy (GTK_WIDGET (win));

      list = next;
    }
}

static gboolean is_red_plugin_enabled;

static gboolean
red_plugin_enabled (void)
{
  return is_red_plugin_enabled;
}

static GMenuModel *
find_plugin_menu (void)
{
  GMenuModel *menubar;
  GMenuModel *menu;
  gint i, j;
  const gchar *label, *id;

  menubar = g_application_get_menubar (g_application_get_default ());
  for (i = 0; i < g_menu_model_get_n_items (menubar); i++)
    {
       if (g_menu_model_get_item_attribute (menubar, i, "label", "s", &label) &&
           g_strcmp0 (label, "_Edit") == 0)
         {
            menu = g_menu_model_get_item_link (menubar, i, "submenu");
            for (j = 0; j < g_menu_model_get_n_items (menu); j++)
              {
                if (g_menu_model_get_item_attribute (menu, j, "id", "s", &id) &&
                    g_strcmp0 (id, "plugins") == 0)
                  {
                    return g_menu_model_get_item_link (menu, j, "section");
                  }
              }
         }
    }

  return NULL;
}

static void
red_action (GAction  *action,
            GVariant *parameter,
            gpointer  data)
{
  g_print ("Here is where we turn the text red\n");
}

static void
enable_red_plugin (void)
{
  GMenu *plugin_menu;
  GAction *action;

  g_print ("Enabling 'Red' plugin\n");

  action = (GAction *)g_simple_action_new ("red-action", NULL);
  g_signal_connect (action, "activate", G_CALLBACK (red_action), NULL);
  g_action_map_add_action (G_ACTION_MAP (g_application_get_default ()), action);
  g_print ("Actions of 'Red' plugin added\n");

  plugin_menu = find_plugin_menu ();
  if (plugin_menu)
    {
      GMenu *section;
      GMenuItem *item;

      section = g_menu_new ();
      g_menu_insert (section, 0, "Turn text red", "app.red-action");
      item = g_menu_item_new_section (NULL, (GMenuModel*)section);
      g_menu_item_set_attribute (item, "id", "s", "red");
      g_menu_append_item (plugin_menu, item);
      g_object_unref (item);
      g_object_unref (section);
      g_print ("Menus of 'Red' plugin added\n");
    }
  else
    g_warning ("Plugin menu not found\n");

  is_red_plugin_enabled = TRUE;
}

static void
disable_red_plugin (void)
{
  GMenuModel *plugin_menu;

  g_print ("Disabling 'Red' plugin\n");

  g_action_map_remove_action (G_ACTION_MAP (g_application_get_default ()), "app.red-action");

  plugin_menu = find_plugin_menu ();
  if (plugin_menu)
    {
      const gchar *id;
      gint i;

      for (i = 0; i < g_menu_model_get_n_items (plugin_menu); i++)
        {
           if (g_menu_model_get_item_attribute (plugin_menu, i, "id", "s", &id) &&
               g_strcmp0 (id, "red") == 0)
             {
               g_menu_remove (plugin_menu, i);
               g_print ("Menus of 'Red' plugin removed\n");
             }
        }
    }
  else
    g_warning ("Plugin menu not found\n");

  is_red_plugin_enabled = FALSE;
}

static void
enable_or_disable_red_plugin (void)
{
  if (red_plugin_enabled ())
    disable_red_plugin ();
  else
    enable_red_plugin ();
}


static void
configure_plugins (GSimpleAction *action,
                   GVariant      *parameter,
                   gpointer       user_data)
{
  GtkBuilder *builder;
  GtkWidget *dialog;
  GtkWidget *red;
  GError *error = NULL;

  builder = gtk_builder_new ();
  gtk_builder_add_from_string (builder,
                               "<interface>"
                               "  <object class='GtkDialog' id='plugin-dialog'>"
                               "    <property name='border-width'>12</property>"
                               "    <property name='title'>Plugins</property>"
                               "    <child internal-child='vbox'>"
                               "      <object class='GtkBox' id='content-area'>"
                               "        <property name='visible'>True</property>"
                               "        <child>"
                               "          <object class='GtkCheckButton' id='red-plugin'>"
                               "            <property name='label'>Red Plugin - turn your text red</property>"
                               "            <property name='visible'>True</property>"
                               "          </object>"
                               "        </child>"
                               "      </object>"
                               "    </child>"
                               "    <child internal-child='action_area'>"
                               "      <object class='GtkButtonBox' id='action-area'>"
                               "        <property name='visible'>True</property>"
                               "        <child>"
                               "          <object class='GtkButton' id='close-button'>"
                               "            <property name='label'>Close</property>"
                               "            <property name='visible'>True</property>"
                               "          </object>"
                               "        </child>"
                               "      </object>"
                               "    </child>"
                               "    <action-widgets>"
                               "      <action-widget response='-5'>close-button</action-widget>"
                               "    </action-widgets>"
                               "  </object>"
                               "</interface>", -1, &error);
  if (error)
    {
      g_warning ("%s", error->message);
      g_error_free (error);
      return;
    }

  dialog = (GtkWidget *)gtk_builder_get_object (builder, "plugin-dialog");
  red = (GtkWidget *)gtk_builder_get_object (builder, "red-plugin");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (red), red_plugin_enabled ());
  g_signal_connect (red, "toggled", G_CALLBACK (enable_or_disable_red_plugin), NULL);

  g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);

  gtk_window_present (GTK_WINDOW (dialog));
}

static GActionEntry app_entries[] = {
  { "about", show_about, NULL, NULL, NULL },
  { "quit", quit_app, NULL, NULL, NULL },
  { "plugins", configure_plugins, NULL, NULL, NULL },
};

static void
plug_man_startup (GApplication *application)
{
  GtkBuilder *builder;

  G_APPLICATION_CLASS (plug_man_parent_class)
    ->startup (application);

  g_action_map_add_action_entries (G_ACTION_MAP (application), app_entries, G_N_ELEMENTS (app_entries), application);

  builder = gtk_builder_new ();
  gtk_builder_add_from_string (builder,
                               "<interface>"
                               "  <menu id='app-menu'>"
                               "    <section>"
                               "      <item label='_About Plugman' action='app.about'/>"
                               "    </section>"
                               "    <section>"
                               "      <item label='_Quit' action='app.quit' accel='<Primary>q'/>"
                               "    </section>"
                               "  </menu>"
                               "  <menu id='menubar'>"
                               "    <submenu label='_Edit'>"
                               "      <section>"
                               "        <item label='_Copy' action='win.copy'/>"
                               "        <item label='_Paste' action='win.paste'/>"
                               "      </section>"
                               "      <section id='plugins'>"
                               "      </section>"
                               "      <section>"
                               "        <item label='Plugins' action='app.plugins'/>"
                               "      </section>"
                               "    </submenu>"
                               "    <submenu label='_View'>"
                               "      <section>"
                               "        <item label='_Fullscreen' action='win.fullscreen'/>"
                               "      </section>"
                               "    </submenu>"
                               "  </menu>"
                               "</interface>", -1, NULL);
  g_application_set_app_menu (application, G_MENU_MODEL (gtk_builder_get_object (builder, "app-menu")));
  g_application_set_menubar (application, G_MENU_MODEL (gtk_builder_get_object (builder, "menubar")));
  g_object_unref (builder);
}

static void
plug_man_init (PlugMan *app)
{
}

static void
plug_man_class_init (PlugManClass *class)
{
  GApplicationClass *application_class = G_APPLICATION_CLASS (class);
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  application_class->startup = plug_man_startup;
  application_class->activate = plug_man_activate;
  application_class->open = plug_man_open;

  object_class->finalize = plug_man_finalize;

}

PlugMan *
plug_man_new (void)
{
  g_type_init ();

  return g_object_new (plug_man_get_type (),
                       "application-id", "org.gtk.Test.plugman",
                       "flags", G_APPLICATION_HANDLES_OPEN,
                       NULL);
}

int
main (int argc, char **argv)
{
  PlugMan *plug_man;
  int status;

  plug_man = plug_man_new ();
  gtk_application_add_accelerator (GTK_APPLICATION (plug_man),
                                   "F11", "win.fullscreen", NULL);
  status = g_application_run (G_APPLICATION (plug_man), argc, argv);
  g_object_unref (plug_man);

  return status;
}