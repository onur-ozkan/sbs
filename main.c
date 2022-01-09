#include <Imlib2.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xinerama.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

Display *display;
int screen;

int set_root_atoms(Pixmap pixmap) {
    Atom atom_root, atom_eroot, type;
    int format;
    unsigned char *data_root, *data_eroot;
    unsigned long length, after;

    atom_root = XInternAtom(display, "_XROOTPMAP_ID", True);
    atom_eroot = XInternAtom(display, "ESETROOT_PMAP_ID", True);

    // doing this to clean up after old background
    if (atom_root != None && atom_eroot != None) {
        XGetWindowProperty(display, RootWindow(display, screen), atom_root, 0L,
                           1L, False, AnyPropertyType, &type, &format, &length,
                           &after, &data_root);

        if (type == XA_PIXMAP) {
            XGetWindowProperty(display, RootWindow(display, screen), atom_eroot,
                               0L, 1L, False, AnyPropertyType, &type, &format,
                               &length, &after, &data_eroot);

            if (data_root && data_eroot && type == XA_PIXMAP &&
                *((Pixmap *)data_root) == *((Pixmap *)data_eroot))
                XKillClient(display, *((Pixmap *)data_root));
        }
    }

    atom_root = XInternAtom(display, "_XROOTPMAP_ID", False);
    atom_eroot = XInternAtom(display, "ESETROOT_PMAP_ID", False);

    if (atom_root == None || atom_eroot == None)
        return 0;

    // setting new background atoms
    XChangeProperty(display, RootWindow(display, screen), atom_root, XA_PIXMAP,
                    32, PropModeReplace, (unsigned char *)&pixmap, 1);
    XChangeProperty(display, RootWindow(display, screen), atom_eroot, XA_PIXMAP,
                    32, PropModeReplace, (unsigned char *)&pixmap, 1);

    return 1;
}

int load_image(const char *arg, int alpha, Imlib_Image rootimg,
               XineramaScreenInfo *outputs, int noutputs) {
    int img_w, img_h, o;
    Imlib_Image buffer = imlib_load_image(arg);

    if (!buffer)
        return 0;

    imlib_context_set_image(buffer);
    img_w = imlib_image_get_width();
    img_h = imlib_image_get_height();

    if (alpha < 255) {
        imlib_image_set_has_alpha(1);
        Imlib_Color_Modifier modifier = imlib_create_color_modifier();
        imlib_context_set_color_modifier(modifier);

        DATA8 red[256], green[256], blue[256], alph[256];
        imlib_get_color_modifier_tables(red, green, blue, alph);
        for (o = 0; o < 256; o++)
            alph[o] = (DATA8)alpha;
        imlib_set_color_modifier_tables(red, green, blue, alph);

        imlib_apply_color_modifier();
        imlib_free_color_modifier();
    }

    imlib_context_set_image(rootimg);

    for (int i = 0; i < noutputs; i++) {
        XineramaScreenInfo o = outputs[i];
        printf("output %d: size(%d, %d) pos(%d, %d)\n", i, o.width, o.height,
               o.x_org, o.y_org);
        imlib_context_set_cliprect(o.x_org, o.y_org, o.width, o.height);

        imlib_blend_image_onto_image(buffer, 0, 0, 0, img_w, img_h, o.x_org,
                                     o.y_org, o.width, o.height);
    }

    imlib_context_set_image(buffer);
    imlib_free_image();

    imlib_context_set_image(rootimg);

    return 1;
}

int main(int argc, char **argv) {
    Visual *vis;
    Colormap cm;
    Imlib_Image image;
    int width, height, depth, alpha;
    Pixmap pixmap;
    Imlib_Color_Modifier modifier = NULL;
    unsigned long screen_mask = ~0;
    int opt_root = false;

    display = XOpenDisplay(NULL);

    if (!display) {
        fprintf(stderr, "Cannot open X display!\n");
        exit(123);
    }

    int noutputs = 0;
    XineramaScreenInfo *outputs = NULL;

    XineramaScreenInfo fake = {
        .x_org = 0,
        .y_org = 0,
        .width = 0,
        .height = 0,
    };

    if (opt_root) {
        noutputs = 1;
        outputs = &fake;
    } else {
        outputs = XineramaQueryScreens(display, &noutputs);
    }

    for (/* global */ screen = 0; screen < ScreenCount(display); screen++) {
        if ((screen_mask & (1 << screen)) == 0)
            continue;

        Imlib_Context *context = imlib_context_new();
        imlib_context_push(context);

        imlib_context_set_display(display);
        vis = DefaultVisual(display, screen);
        cm = DefaultColormap(display, screen);
        width = DisplayWidth(display, screen);
        height = DisplayHeight(display, screen);
        depth = DefaultDepth(display, screen);

        if (opt_root) {
            outputs[0].width = width;
            outputs[0].height = height;
        }

        pixmap = XCreatePixmap(display, RootWindow(display, screen), width,
                               height, depth);

        imlib_context_set_visual(vis);
        imlib_context_set_colormap(cm);
        imlib_context_set_drawable(pixmap);
        imlib_context_set_color_range(imlib_create_color_range());

        image = imlib_create_image(width, height);
        imlib_context_set_image(image);

        imlib_context_set_color(0, 0, 0, 255);
        imlib_image_fill_rectangle(0, 0, width, height);

        imlib_context_set_dither(1);
        imlib_context_set_blend(1);

        alpha = 255;

        if (modifier != NULL) {
            imlib_apply_color_modifier();
            imlib_free_color_modifier();
        }
        modifier = imlib_create_color_modifier();
        imlib_context_set_color_modifier(modifier);

        if (argc < 2) {
            fprintf(stderr, "Missing image\n");
            continue;
        }

        if (load_image(argv[1], alpha, image, outputs, noutputs) == 0) {
            fprintf(stderr, "Bad image (%s)\n", argv[1]);
            continue;
        }

        if (modifier != NULL) {
            imlib_context_set_color_modifier(modifier);
            imlib_apply_color_modifier();
            imlib_free_color_modifier();
            modifier = NULL;
        }

        imlib_render_image_on_drawable(0, 0);
        imlib_free_image();
        imlib_free_color_range();

        if (set_root_atoms(pixmap) == 0)
            fprintf(stderr, "Couldn't create atoms...\n");

        XKillClient(display, AllTemporary);
        XSetCloseDownMode(display, RetainTemporary);

        XSetWindowBackgroundPixmap(display, RootWindow(display, screen),
                                   pixmap);
        XClearWindow(display, RootWindow(display, screen));

        XFlush(display);
        XSync(display, False);

        imlib_context_pop();
        imlib_context_free(context);
    }

    if (outputs != NULL) {
        if (!opt_root) {
            XFree(outputs);
        }

        outputs = NULL;
        noutputs = 0;
    }

    return 0;
}

