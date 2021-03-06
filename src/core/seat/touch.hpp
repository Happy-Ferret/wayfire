#ifndef TOUCH_HPP
#define TOUCH_HPP

#include <map>

extern "C"
{
#include <wlr/types/wlr_cursor.h>
}

struct wf_gesture_recognizer
{
    struct finger
    {
        int id;
        int sx, sy;
        int ix, iy;
    };

    std::map<int, finger> current;

    void update_touch(int32_t time, int id, int sx, int sy);

    void register_touch(int time, int id, int sx, int sy);
    void unregister_touch(int32_t time, int32_t id);

private:

    bool in_gesture = false, gesture_emitted = false;
    int start_sum_dist;

    void start_new_gesture(int reason_id, int time);
    void continue_gesture(int id, int sx, int sy);
    void stop_gesture();
    void reset_gesture();
};

struct wf_touch
{
    wl_listener down, up, motion;
    wf_gesture_recognizer gesture_recognizer;
    wlr_cursor *cursor;

    wf_touch(wlr_cursor *cursor);
    void add_device(wlr_input_device *device);
};

#endif /* end of include guard: TOUCH_HPP */
