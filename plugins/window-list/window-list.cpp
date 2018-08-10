#include <set>

#include "nonstd/make_unique.hpp"
#include "core.hpp"
#include "view.hpp"
#include "plugin.hpp"
#include "output.hpp"
#include "signal-definitions.hpp"
#include "debug.hpp"

#include "wayfire-task-list-protocol.h"

using zwf_client_list = wl_list*;
using zwf_window_resource_list = wl_list*;

void zwf_task_manager_v1_destroy(wl_client *client, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct zwf_task_manager_v1_interface zwf_task_manager_implementation = {
    zwf_task_manager_v1_destroy
};

static void handle_zwf_task_manager_destroy(wl_resource *resource)
{
    wl_list_remove(wl_resource_get_link(resource));
}

void bind_zwf_task_manager(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    auto list = (zwf_client_list*) data;
    auto resource = wl_resource_create(client, &zwf_task_manager_v1_interface, 1, id);
    wl_list_insert(*list, wl_resource_get_link(resource));

    wl_resource_set_implementation(resource, &zwf_task_manager_implementation, NULL,
                                   handle_zwf_task_manager_destroy);
}

static const std::string cdata_name = "zwf-window-list";
static void zwf_window_v1_focus(struct wl_client *client, struct wl_resource *resource)
{
    auto view = (wayfire_view_t*) wl_resource_get_user_data(resource);
    view->get_output()->focus_view(view->self());
}

static void zwf_window_v1_minimize(wl_client *client, wl_resource *resource)
{
    /* TODO */
}

static void zwf_window_v1_destroy(wl_client *client, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct zwf_window_v1_interface zwf_window_implementation = {
    zwf_window_v1_focus,
    zwf_window_v1_minimize,
    zwf_window_v1_destroy
};

static void handle_zwf_window_v1_destroy(wl_resource *resource)
{
    wl_list_remove(wl_resource_get_link(resource));
}

struct zwf_custom_view_data : public wf_custom_view_data
{
    wl_list list;
    zwf_custom_view_data()
    {
        wl_list_init(&list);
    }

    ~zwf_custom_view_data()
    { }
};

zwf_custom_view_data *get_data_for_view(wayfire_view view)
{
    auto data = dynamic_cast<zwf_custom_view_data*> (view->custom_data[cdata_name].get());
    assert(data);

    return data;
}

static void zwf_window_v1_send_data(wl_resource *resource, wayfire_view view)
{
    zwf_window_v1_send_title(resource, view->get_title().c_str());
    zwf_window_v1_send_app_id(resource, view->get_app_id().c_str());
    zwf_window_v1_send_enter_output(resource, view->get_output()->handle->name);
}

static void create_zwf_window_v1(wl_resource *client, wayfire_view view)
{
    if (!view->custom_data.count(cdata_name))
        view->custom_data[cdata_name] = nonstd::make_unique<zwf_custom_view_data>();

    auto data = get_data_for_view(view);
    auto resource = wl_resource_create(wl_resource_get_client(client), &zwf_window_v1_interface, 1u, 0);
    wl_list_insert(&data->list, wl_resource_get_link(resource));

    wl_resource_set_implementation(resource, &zwf_window_implementation, view.get(),
                                   handle_zwf_task_manager_destroy);

    zwf_task_manager_v1_send_window_created(client, resource);
    zwf_window_v1_send_data(resource, view);
}

class wayfire_window_list : public wayfire_plugin_t
{
    wl_list client_list;
    void setup_global()
    {
        wl_list_init(&client_list);
        if (wl_global_create(core->display, &zwf_task_manager_v1_interface,
                             1, &client_list, bind_zwf_task_manager) == NULL)
        {
            log_error("Failed to create wayfire_shell interface");
        }
    }

    void create_view(wayfire_view view)
    {
        wl_resource *resource;
        wl_resource_for_each(resource, &client_list)
            create_zwf_window_v1(resource, view);
    }

    void destroy_view(wayfire_view view)
    {
        if (!view->custom_data.count(cdata_name))
            return;

        log_info("destroy view window-list");
        auto data = get_data_for_view(view);
        wl_resource *resource;
        std::vector<wl_resource*> to_destroy;
        wl_resource_for_each(resource, &data->list)
        {
            log_info("got resoruce");
            to_destroy.push_back(resource);
        }

        for (auto resource : to_destroy)
        {
            zwf_window_v1_send_destroyed(resource);
            wl_resource_destroy(resource);
        }
    }

    signal_callback_t view_map, view_unmap;
    void init(wayfire_config *)
    {
        /* first output */
        if (core->get_num_outputs() == 0)
            setup_global();

        view_map = [=] (signal_data *data)
        {
            create_view(get_signaled_view(data));
        };

        view_unmap = [=] (signal_data *data)
        {
            destroy_view(get_signaled_view(data));
        };

        /* TODO: react to attach/detach */
        output->connect_signal("map-view", &view_map);
        output->connect_signal("unmap-view", &view_unmap);
    }

    void fini()
    {
    }
};

extern "C"
{
    wayfire_plugin_t *newInstance()
    {
        return new wayfire_window_list;
    }
}
