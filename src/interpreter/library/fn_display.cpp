#include "libpre.h"
    #include "fn_display.h"
#include "libpost.h"

#include "ogm/interpreter/Variable.hpp"
#include "ogm/common/error.hpp"
#include "ogm/common/util.hpp"
#include "ogm/interpreter/Executor.hpp"
#include "ogm/interpreter/execute.hpp"
#include "ogm/interpreter/display/Display.hpp"

#include <string>
#include "ogm/common/error.hpp"

#include <cctype>
#include <cstdlib>

using namespace ogm::interpreter;
using namespace ogm::interpreter::fn;

#define frame staticExecutor.m_frame

void ogm::interpreter::fn::window_set_position(VO out, V x, V y)
{
    frame.m_display->set_window_position(x.castCoerce<real_t>(), y.castCoerce<real_t>());
}

void ogm::interpreter::fn::window_set_size(VO out, V x, V y)
{
    frame.m_display->set_window_size(x.castCoerce<real_t>(), y.castCoerce<real_t>());
}

// NEW FEATURE (signature: annika marie schlögel)
// function to set window size and position at once
void ogm::interpreter::fn::window_set_rectangle(VO out, V x, V y, V w, V h)
{
    frame.m_display->set_window_position(
        x.castCoerce<real_t>(),
        y.castCoerce<real_t>()
    );

    frame.m_display->set_window_size(
        w.castCoerce<real_t>(),
        h.castCoerce<real_t>()
    );
}
// NEW FEATURE END

void ogm::interpreter::fn::display_get_width(VO out)
{
    out = static_cast<real_t>(frame.m_display->get_display_dimensions().x);
}

void ogm::interpreter::fn::display_get_height(VO out)
{
    out = static_cast<real_t>(frame.m_display->get_display_dimensions().y);
}

void ogm::interpreter::fn::window_get_width(VO out)
{
    out = static_cast<real_t>(frame.m_display->get_window_dimensions().x);
}

void ogm::interpreter::fn::window_get_height(VO out)
{
    out = static_cast<real_t>(frame.m_display->get_window_dimensions().y);
}

void ogm::interpreter::fn::window_get_colour(VO out)
{
    out = frame.m_display->get_clear_colour();
}

void ogm::interpreter::fn::window_set_colour(VO out, V c)
{
    frame.m_display->set_clear_colour(c.castCoerce<uint32_t>());
}

// MODIFIED FEATURE (signature: annika marie schlögel)
// wrapper was non-functional, now it calls the newly implemented function
void ogm::interpreter::fn::window_get_fullscreen(VO out)
{
    out = frame.m_display->get_fullscreen();
}
// MODIFIED FEATURE END

// NEW FEATURE (signature: annika marie schlögel)
void ogm::interpreter::fn::window_set_fullscreen(VO out, V fullscreen)
{
    frame.m_display->set_fullscreen(fullscreen.cond());
}
// NEW FEATURE END

// NEW FEATURE (signature: annika marie schlögel)
void ogm::interpreter::fn::window_has_focus(VO out)
{
    out = frame.m_display->window_has_focus();
}
// NEW FEATURE END

// NEW FEATURE (signature: annika marie schlögel)
void ogm::interpreter::fn::window_handle(VO out)
{
    out = static_cast<real_t>(frame.m_display->get_native_window_handle());
}
// NEW FEATURE END

void ogm::interpreter::fn::display_reset(VO out, V aa, V vsync)
{
    frame.m_display->set_vsync(vsync.cond());
    
    // TODO: anti-aliasing
}

// NEW FEATURE (signature: annika marie schlögel)
void ogm::interpreter::fn::device_mouse_x(VO out, V device)
{
    // TODO: include touch support
    // literally ignore any other device (because GMLarcana does not support touch yet)
    if (device.castCoerce<int32_t>() != 0)
    {
        out = 0;
        return;
    }

    getv::mouse_x(out);
}

void ogm::interpreter::fn::device_mouse_y(VO out, V device)
{
    // TODO: include touch support
    // literally ignore any other device (because GMLarcana does not support touch yet)
    if (device.castCoerce<int32_t>() != 0)
    {
        out = 0;
        return;
    }

    getv::mouse_y(out);
}

void ogm::interpreter::fn::device_mouse_x_to_gui(VO out, V device)
{
    // TODO: add proper gui coordinate system support
    // just a compat wrapper now as the gui currently has no separate mouse coordinate system
    if (device.castCoerce<int32_t>() != 0)
    {
        out = 0;
        return;
    }

    getv::mouse_x(out);
}

void ogm::interpreter::fn::device_mouse_y_to_gui(VO out, V device)
{
    // TODO: add proper gui coordinate system support
    // just a compat wrapper now as the gui currently has no separate mouse coordinate system
    if (device.castCoerce<int32_t>() != 0)
    {
        out = 0;
        return;
    }

    getv::mouse_y(out);
}
// NEW FEATURE END

void ogm::interpreter::fn::getv::mouse_x(VO out)
{
    out = static_cast<real_t>(frame.m_display->get_mouse_coord_invm().x);
}

void ogm::interpreter::fn::getv::mouse_y(VO out)
{
    out = static_cast<real_t>(frame.m_display->get_mouse_coord_invm().y);
}

void ogm::interpreter::fn::window_mouse_get_x(VO out)
{
    out = static_cast<real_t>(frame.m_display->get_mouse_coord().x);
}

void ogm::interpreter::fn::window_mouse_get_y(VO out)
{
    out = static_cast<real_t>(frame.m_display->get_mouse_coord().y);
}

// NEW FEATURE (signature: annika marie schlögel)
void ogm::interpreter::fn::window_mouse_set(VO out, V x, V y)
{
    frame.m_display->set_mouse_position(
        x.castCoerce<coord_t>(),
        y.castCoerce<coord_t>()
    );
}
// NEW FEATURE END

// NEW FEATURE (signature: annika marie schlögel)
void ogm::interpreter::fn::display_set_gui_maximise(VO out)
{
    // GUI coordinates currently always match window coordinates.
}

void ogm::interpreter::fn::display_set_gui_size(
    VO out,
    V w,
    V h
)
{
    // GUI scaling not implemented yet.
}

void ogm::interpreter::fn::display_get_gui_width(VO out)
{
    out = frame.m_display->get_window_dimensions().x;
}

void ogm::interpreter::fn::display_get_gui_height(VO out)
{
    out = frame.m_display->get_window_dimensions().y;
}
// NEW FEATURE END
