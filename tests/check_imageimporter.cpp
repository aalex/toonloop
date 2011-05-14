#include <iostream>
#include <cstdio> // tmpnam
#include <clutter-gtk/clutter-gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <boost/filesystem.hpp>
#include "image_importer.h"
#include "config.h"

/* XPM */
//static char * colors_xpm[] = {
const static char *colors_xpm[] = {
"4 3 12 1",
" 	c #FF0000",
".	c #00FF00",
"+	c #0000FF",
"@	c #FFFFFF",
"#	c #00FFFF",
"$	c #FF00FF",
"%	c #FFFF00",
"&	c #000000",
"*	c #333333",
"=	c #666666",
"-	c #999999",
";	c #CCCCCC",
" .+@",
"#$%&",
"*=-;"};

bool create_4x3_image(const std::string &file_name)
{
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data(colors_xpm);

    std::string img_type = "jpeg";
    GError *error = NULL;
    gboolean success = gdk_pixbuf_save(pixbuf, file_name.c_str(), img_type.c_str(), &error, NULL);
    g_object_unref(pixbuf);
    if (error)
    {
        g_error("Error saving image %s: %s", file_name.c_str(), error->message);
        g_error_free(error);
        return false;
    }
    if (! success)
    {
        std::cerr << "Failed to save " << file_name << std::endl;
        return false;
    }
    return true;
}

std::string make_temporary_name()
{
    // FIXME: tmpnam is not recommended
    //int fd = 0;
    //fd = std::mkstemp("/tmp/imageXXXXXX");
    return std::string(std::tmpnam(0));
}

bool check_image_resize()
{
    std::string input = make_temporary_name() + ".jpg";
    std::string output = make_temporary_name() + ".jpg";

    bool success;
    success = create_4x3_image(input);
    if (! success)
        return false;
    ImageImporter img(input, output, 640, 480, true);
    success = img.resize();
    //std::cout << "Input: " << input << std::endl;
    //if (success)
    //    std::cout << " Output: " << output << std::endl;
#ifdef HAVE_BOOST_FILESYSTEM
    namespace fs = boost::filesystem;
    fs::remove(input);
    fs::remove(output);
#endif
    return success;
}

int main(int argc, char *argv[])
{
    gtk_clutter_init(&argc, &argv);
    if (check_image_resize())
    {
        return 0;
    } else {
        return 1;
    }
}

