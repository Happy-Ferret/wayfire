#include <memory>
#include <getopt.h>
#include "config.hpp"
#include "dock.hpp"
#include "window.hpp"

class wayfire_dock
{
    std::string image;

    wayfire_output *output;
    wayfire_window *window = nullptr;
    cairo_t *cr;

    public:
    wayfire_dock(wayfire_output *output)
    {
        this->image = image;
        this->output = output;
        output->resized_callback = [=] (wayfire_output *output, int w, int h)
        { resize(w, h); };

        output->destroyed_callback = [=] (wayfire_output* output)
        { delete this; };

        zwf_output_v1_inhibit_output(output->zwf);
    }

    void resize(int w, int h)
    {
        if (window)
            delete window;

        window = output->create_window(w, h, [=] () { init_dock(w, h); });
    }

    void init_dock(int width, int height)
    {
        window->zwf = zwf_output_v1_get_wm_surface(output->zwf, window->surface, ZWF_OUTPUT_V1_WM_ROLE_BACKGROUND);

        zwf_wm_surface_v1_configure(window->zwf, 0, 0);

        using namespace std::placeholders;
        window->pointer_enter = std::bind(std::mem_fn(&wayfire_dock::on_enter),
                                          this, _1, _2, _3, _4);

        cr = cairo_create(window->cairo_surface);

        window->damage_commit();
        zwf_output_v1_inhibit_output_done(output->zwf);
    }

    void on_enter(wl_pointer *ptr, uint32_t serial, int x, int y)
    {
        output->display->show_default_cursor(serial);
    }

    ~wayfire_dock()
    {
        if (window)
        {
            cairo_destroy(cr);
            delete window;
        }
    }
};

std::map<uint32_t, std::unique_ptr<wayfire_dock>> outputs;

int main(int argc, char *argv[])
{
    std::string home_dir = secure_getenv("HOME");
    std::string config_file = home_dir + "/.config/wayfire.ini";

    struct option opts[] = {
        { "config",   required_argument, NULL, 'c' },
        { 0,          0,                 NULL,  0  }
    };

    int c, i;
    while((c = getopt_long(argc, argv, "c:l:", opts, &i)) != -1)
    {
        switch(c)
        {
            case 'c':
                config_file = optarg;
                break;
            default:
                std::cerr << "failed to parse option " << optarg << std::endl;
        }
    }

 //   wayfire_config *config = new wayfire_config(config_file);
//    auto section = config->get_section("shell");

    auto display = new wayfire_display([=] (wayfire_output *output)
    {
        new wayfire_dock(output);
    });

    while(true)
    {
        if (wl_display_dispatch(display->display) < 0)
            break;
    }

    delete display;
    return 0;
}
