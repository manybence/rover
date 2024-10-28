MainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL); // GTK_WINDOW_TOPLEVEL = Has a titlebar and border, managed by the window manager.

// Get screen size
int ScreenWidth;
int ScreenHeight;
GdkScreen *screen = gtk_window_get_screen(GTK_WINDOW(MainWindow));
ScreenWidth = gdk_screen_get_width(screen); // in pixels
ScreenHeight = gdk_screen_get_height(screen);

// Create a Fixed Container
GtkWidget *fixed;
fixed = gtk_fixed_new();
gtk_widget_set_size_request(fixed, ScreenWidth, ScreenHeight);
gtk_container_add(GTK_CONTAINER(MainWindow), fixed);
gtk_widget_show(fixed);

// Add an image
GtkWidget *image;
image = gtk_image_new_from_file("/home/pi/projects/my_project/images/my_image.png");
gtk_fixed_put(GTK_FIXED(fixed), image, 10, 20); // x, y from top left

gtk_widget_show_all(MainWindow);
