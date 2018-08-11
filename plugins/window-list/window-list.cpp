#include <set>

#include "nonstd/make_unique.hpp"
#include "core.hpp"
#include "view.hpp"
#include "plugin.hpp"
#include "output.hpp"
#include "signal-definitions.hpp"
#include "workspace-manager.hpp"
#include "debug.hpp"

#include "wayfire-window-list-protocol.h"

using zwf_window_resource_list = wl_list*;
using zwf_client_callback = std::function<void(wl_resource*)>*;

void zwf_window_manager_v1_destroy(wl_client *client, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct zwf_window_manager_v1_interface zwf_window_manager_implementation = {
    zwf_window_manager_v1_destroy
};

static void handle_zwf_window_manager_destroy(wl_resource *resource)
{
    wl_list_remove(wl_resource_get_link(resource));
}

void bind_zwf_window_manager(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    auto callback = (zwf_client_callback) data;
    auto resource = wl_resource_create(client, &zwf_window_manager_v1_interface, 1, id);
    wl_resource_set_implementation(resource, &zwf_window_manager_implementation, NULL,
                                   handle_zwf_window_manager_destroy);

    (*callback) (resource);
}

static const std::string cdata_name = "zwf-window-list";
static void zwf_window_v1_focus(struct wl_client *client, struct wl_resource *resource)
{
    auto view = (wayfire_view_t*) wl_resource_get_user_data(resource);
    view->get_output()->focus_view(view->self());
}

static void zwf_window_v1_toggle_minimized(wl_client *client, wl_resource *resource)
{
    /* TODO */
}

static void zwf_window_v1_destroy(wl_client *client, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

static const struct zwf_window_v1_interface zwf_window_implementation = {
    zwf_window_v1_focus,
    zwf_window_v1_toggle_minimized,
    zwf_window_v1_destroy
};

static void handle_zwf_window_v1_destroy(wl_resource *resource)
{
    // if it hasn't been already destroyed server-side, its user data will be
    // the view and not NULL
    log_info("destroy resource %p has %p", resource, wl_resource_get_user_data(resource));
    if (wl_resource_get_user_data(resource))
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

static void window_v1_send_title(wl_resource *resource, wayfire_view view)
{
    zwf_window_v1_send_title(resource, view->get_title().c_str());
}

static void window_v1_send_app_id(wl_resource *resource, wayfire_view view)
{
    zwf_window_v1_send_app_id(resource, view->get_app_id().c_str());
}

static void window_v1_send_output(wl_resource *resource, wayfire_view view, bool leave)
{
    if (leave)
    {
        zwf_window_v1_send_leave_output(resource, view->get_output()->handle->name);
    } else
    {
        zwf_window_v1_send_enter_output(resource, view->get_output()->handle->name);
    }
}

static void create_zwf_window_v1(wl_resource *client, wayfire_view view)
{
    if (!view->custom_data.count(cdata_name))
        view->custom_data[cdata_name] = nonstd::make_unique<zwf_custom_view_data>();

    auto data = get_data_for_view(view);
    auto resource = wl_resource_create(wl_resource_get_client(client), &zwf_window_v1_interface, 1u, 0);
    wl_list_insert(&data->list, wl_resource_get_link(resource));

    wl_resource_set_implementation(resource, &zwf_window_implementation, view.get(),
                                   handle_zwf_window_v1_destroy);

    log_info("create resource %p", resource);

    zwf_window_manager_v1_send_window_created(client, resource);

    window_v1_send_title(resource, view);
    window_v1_send_app_id(resource, view);
    window_v1_send_output(resource, view, false);
}

static wl_list client_list;
static wl_global *global;
static int loaded_count = 0;

class wayfire_window_list : public wayfire_plugin_t
{
    void setup_global()
    {
        wl_list_init(&client_list);
        global = wl_global_create(core->display, &zwf_window_manager_v1_interface,
                                  1, &initialize_client, bind_zwf_window_manager);

        if (!global)
            log_error("Failed to create wayfire_shell interface");
    }

    void create_view(wayfire_view view)
    {
        wl_resource *resource;
        wl_resource_for_each(resource, &client_list)
            create_zwf_window_v1(resource, view);
    }

    std::function<void(wl_resource*)> initialize_client =
    [=] (wl_resource *resource)
    {
        output->workspace->for_each_view([=] (wayfire_view view) {
            if (view->is_mapped() && view->role == WF_VIEW_ROLE_TOPLEVEL)
                create_zwf_window_v1(resource, view);
        }, WF_ALL_LAYERS);

        wl_list_insert(&client_list, wl_resource_get_link(resource));
    };

    void view_for_each_resource(wayfire_view view, std::function<void(wl_resource*)> action)
    {
        if (!view->custom_data.count(cdata_name))
            return;

        log_info("destroy view window-list");
        auto data = get_data_for_view(view);

        wl_resource *resource;
        std::vector<wl_resource*> accumulated;
        wl_resource_for_each(resource, &data->list)
            accumulated.push_back(resource);

        for (auto& resource : accumulated)
            action(resource);
    }

    void destroy_view(wayfire_view view)
    {
        view_for_each_resource(view, [] (wl_resource *resource)
        {
            zwf_window_v1_send_destroyed(resource);
            // remove so that no further events are sent
            wl_list_remove(wl_resource_get_link(resource));
            // we removed the resource from the list, so we must make it inert
            wl_resource_set_user_data(resource, NULL);

            log_info("reset destroy resource %p has %p", resource, wl_resource_get_user_data(resource));
        });
    }

    void update_view_title(wayfire_view view)
    {
        view_for_each_resource(view, [&view] (wl_resource *resource)
        {
            window_v1_send_title(resource, view);
        });
    }

    void update_view_app_id(wayfire_view view)
    {
        view_for_each_resource(view, [&view] (wl_resource *resource)
        {
            window_v1_send_app_id(resource, view);
        });
    }

    void view_detached(wayfire_view view)
    {
        view_for_each_resource(view, [&view] (wl_resource *resource)
        {
            window_v1_send_output(resource, view, true);
        });
    }

    void view_attached(wayfire_view view)
    {
        view_for_each_resource(view, [&view] (wl_resource *resource)
        {
            window_v1_send_output(resource, view, false);
        });
    }

    signal_callback_t view_map, view_unmap, view_attach, view_detach, view_title, view_app_id;
    void init(wayfire_config *)
    {
        /* first output */
        if (loaded_count++ == 0)
            setup_global();

        if (!global)
            return;

        view_map = [=] (signal_data *data)
        {
            auto view = get_signaled_view(data);
            if (view->role == WF_VIEW_ROLE_TOPLEVEL)
                create_view(view);
        };

        view_unmap = [=] (signal_data *data) { destroy_view(get_signaled_view(data)); };
        view_attach = [=] (signal_data *data) { view_attached(get_signaled_view(data)); };
        view_detach = [=] (signal_data *data) { view_detached(get_signaled_view(data)); };
        view_title  = [=] (signal_data *data) { update_view_title(get_signaled_view(data)); };
        view_app_id = [=] (signal_data *data) { update_view_app_id(get_signaled_view(data)); };

        output->connect_signal("map-view", &view_map);
        output->connect_signal("unmap-view", &view_unmap);
        output->connect_signal("attach-view", &view_attach);
        output->connect_signal("detach-view", &view_detach);
        output->connect_signal("view-title-changed", &view_title);
        output->connect_signal("view-app-id-changed", &view_app_id);
    }

    void fini()
    {
        output->disconnect_signal("map-view", &view_map);
        output->disconnect_signal("unmap-view", &view_unmap);
        output->disconnect_signal("attach-view", &view_attach);
        output->disconnect_signal("detach-view", &view_detach);
        output->disconnect_signal("view-title-changed", &view_title);
        output->disconnect_signal("view-app-id-changed", &view_app_id);

        /* we are not the last output, the plugin is still loaded */
        if (--loaded_count)
            return;

        output->workspace->for_each_view([=] (wayfire_view view){
            destroy_view(view);
        }, WF_ALL_LAYERS);

        wl_global_destroy(global);
    }
};

extern "C"
{
    wayfire_plugin_t *newInstance()
    {
        return new wayfire_window_list;
    }
}
