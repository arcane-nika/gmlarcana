#include "libpre.h"
    #include "fn_surface.h"
    #include "ogm/fn_ogmeta.h"
#include "libpost.h"
#include "stb_image_write.h"

#include "ogm/interpreter/Variable.hpp"
#include "ogm/common/error.hpp"
#include "ogm/common/util.hpp"
#include "ogm/interpreter/Executor.hpp"
#include "ogm/interpreter/execute.hpp"
#include "ogm/interpreter/display/Display.hpp"
#include "ogm/geometry/Vector.hpp"
#include "ogm/common/error.hpp"

#include <string>

#include <cctype>
#include <cstdlib>

#include <vector>
#include <cstring>

#ifdef GFX_AVAILABLE
#include "common/sdl.hpp"
#endif

using namespace ogm::interpreter;
using namespace ogm::interpreter::fn;

#define frame staticExecutor.m_frame
#define display frame.m_display

namespace
{
    // NEW FEATURE (signature: annika marie schlögel)
    // helpers for image saving from drawing target (surface_save)
    void flip_rgba_rows(
        uint8_t* pixels,
        uint32_t width,
        uint32_t height
    )
    {
        const size_t stride = width * 4;

        std::vector<uint8_t> temp(stride);

        for (uint32_t y = 0; y < height / 2; ++y)
        {
            uint8_t* top = pixels + y * stride;
            uint8_t* bottom = pixels + (height - 1 - y) * stride;

            std::memcpy(temp.data(), top, stride);
            std::memcpy(top, bottom, stride);
            std::memcpy(bottom, temp.data(), stride);
        }
    }

    bool save_texturepage_png(
        TexturePage* page,
        uint32_t x,
        uint32_t y,
        uint32_t width,
        uint32_t height,
        const std::string& filename
    )
    {
        if (!page)
        {
            return false;
        }

        if (!page->cache())
        {
            return false;
        }

        GLint previous_fbo;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous_fbo);

        glBindFramebuffer(GL_FRAMEBUFFER, page->m_gl_framebuffer);

        std::vector<uint8_t> pixels(width * height * 4);

        glReadPixels(
            x,
            y,
            width,
            height,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            pixels.data()
        );

        glBindFramebuffer(GL_FRAMEBUFFER, previous_fbo);

        flip_rgba_rows(
            pixels.data(),
            width,
            height
        );

        return stbi_write_png(
            filename.c_str(),
            width,
            height,
            4,
            pixels.data(),
            width * 4
        ) != 0;
    }
    // NEW FEATURE END

    struct Target {
        TexturePage* m_tpage;
        bool m_view_projection_enabled;
    };
    std::vector<Target> g_target_stack;

    // application surface
    surface_id_t g_application_surface = -1;
    bool g_application_surface_enabled = true;
    bool g_application_surface_draw_enabled = true;
}

void ogm::interpreter::fn::surface_create(VO out, V w, V h)
{
    out = static_cast<real_t>(display->m_textures.create_surface(ogm::geometry::Vector<uint32_t>(w.castCoerce<int32_t>(), h.castCoerce<int32_t>())));
}

void ogm::interpreter::fn::surface_get_width(VO out, V vid)
{
    surface_id_t id = vid.castCoerce<surface_id_t>();
    TexturePage* tpage = display->m_textures.get_surface(id);
    if (tpage)
    {
        out = static_cast<real_t>(tpage->m_dimensions.x);
    }
    else
    {
        out = 0;
    }
}

void ogm::interpreter::fn::surface_get_height(VO out, V vid)
{
    surface_id_t id = vid.castCoerce<surface_id_t>();
    TexturePage* tpage = display->m_textures.get_surface(id);
    if (tpage)
    {
        out = static_cast<real_t>(tpage->m_dimensions.y);
    }
    else
    {
        out = 0;
    }
}

void ogm::interpreter::fn::surface_getpixel_ext(VO out, V vid, V vx, V vy)
{
    geometry::Vector<uint32_t> v {
        vx.castCoerce<uint32_t>(),
        vy.castCoerce<uint32_t>()
    };

    surface_id_t id = vid.castCoerce<surface_id_t>();
    TexturePage* tpage = display->m_textures.get_surface(id);

    if (!tpage)
    {
        out = 0;
        return;
    }

    uint32_t c = display->m_textures.get_texture_pixel(tpage, v);
    // rgba -> abgr
    uint32_t d =
          ((c & 0xff) << 24)
        | ((c & 0xff00) << 8)
        | ((c & 0xff0000) >> 8)
        | ((c & 0xff000000) >> 24);

    out = d;
}

void ogm::interpreter::fn::surface_getpixel(VO out, V vid, V vx, V vy)
{
    surface_getpixel_ext(out, vid, vx, vy);
    out &= 0xffffff;
    out = out.castCoerce<real_t>();
}


void ogm::interpreter::fn::surface_set_target(VO out, V vid)
{
    surface_id_t id = vid.castCoerce<surface_id_t>();
    TexturePage* tpage = display->m_textures.get_surface(id);
    if (!tpage)
    {
        throw MiscError("cannot set target to null surface; did you mean to use surface_reset_target()?");
        return;
    }
    Target t{ tpage, false };
    g_target_stack.push_back(t);
    display->set_target(tpage);
    display->enable_view_projection(false);
}

void ogm::interpreter::fn::surface_reset_target(VO out)
{
    g_target_stack.pop_back();
    if (g_target_stack.size() == 0)
    {
        display->set_target(nullptr);
        display->enable_view_projection(true);
    }
    else
    {
        display->set_target(g_target_stack.back().m_tpage);
        display->enable_view_projection(g_target_stack.back().m_view_projection_enabled);
    }
}

void ogm::interpreter::fn::ogm_target_view_projection_enable(VO out)
{
    g_target_stack.back().m_view_projection_enabled = true;
    display->enable_view_projection(true);
}

void ogm::interpreter::fn::ogm_surface_reset_target_all(VO out)
{
    g_target_stack.clear();
    display->set_target(nullptr);
    display->enable_view_projection(true);
}

void ogm::interpreter::fn::surface_free(VO out, V id)
{
    display->m_textures.free_surface(
        id.castCoerce<surface_id_t>()
    );
}

void ogm::interpreter::fn::surface_resize(VO out, V id, V w, V h)
{
    display->m_textures.resize_surface(
        id.castCoerce<surface_id_t>(),
        {
            static_cast<uint32_t>(w.castCoerce<size_t>()),
            static_cast<uint32_t>(h.castCoerce<size_t>())
        }
    );
}

void ogm::interpreter::fn::surface_exists(VO out, V vid)
{
    surface_id_t id = vid.castCoerce<surface_id_t>();
    out = !!display->m_textures.get_surface(id);
}

void ogm::interpreter::fn::surface_get_texture(VO out, V vid)
{
    surface_id_t id = vid.castCoerce<surface_id_t>();
    out = display->m_textures.get_surface_texture_view(id);
}

// NEW FEATURE (signature: annika marie schlögel)
// image saving by drawing area (surface_save)
void ogm::interpreter::fn::surface_save(
    VO out,
    V vid,
    V vfilename
)
{
    surface_id_t id = vid.castCoerce<surface_id_t>();

    TexturePage* tp = display->m_textures.get_surface(id);

    if (!tp)
    {
        throw MiscError("surface does not exist.");
    }

    std::string filename = frame.m_fs.resolve_file_path(vfilename.castCoerce<std::string>(), true);

    if (!save_texturepage_png(
        tp,
        0,
        0,
        tp->m_dimensions.x,
        tp->m_dimensions.y,
        filename
    ))
    {
        throw MiscError("failed to save surface.");
    }
}

void ogm::interpreter::fn::surface_save_part(
    VO out,
    V vid,
    V vfilename,
    V vx,
    V vy,
    V vw,
    V vh
)
{
    surface_id_t id = vid.castCoerce<surface_id_t>();

    TexturePage* tp = display->m_textures.get_surface(id);

    if (!tp)
    {
        throw MiscError("surface does not exist.");
    }

    uint32_t x = vx.castCoerce<uint32_t>();
    uint32_t y = vy.castCoerce<uint32_t>();
    uint32_t w = vw.castCoerce<uint32_t>();
    uint32_t h = vh.castCoerce<uint32_t>();

    std::string filename = frame.m_fs.resolve_file_path(vfilename.castCoerce<std::string>(), true);

    if (!save_texturepage_png(
        tp,
        x,
        y,
        w,
        h,
        filename
    ))
    {
        throw MiscError("failed to save surface.");
    }
}
// NEW FEATURE END

void ogm::interpreter::fn::ogm_create_application_surface(VO out)
{
    g_application_surface = display->m_textures.create_surface(
        {
            std::max<uint32_t>(1, static_cast<uint32_t>(display->get_window_dimensions().x)),
            std::max<uint32_t>(1, static_cast<uint32_t>(display->get_window_dimensions().y))
        }
    );
    TexturePage* surface = display->m_textures.get_surface(g_application_surface);
    if (surface)
    {
        surface->m_no_delete = true;
    }
}

void ogm::interpreter::fn::ogm_application_surface_is_draw_enabled(VO out)
{
    out = g_application_surface_draw_enabled;
}

void ogm::interpreter::fn::application_surface_enable(VO out, V a)
{
    g_application_surface_enabled = a.cond();
}

void ogm::interpreter::fn::application_surface_draw_enable(VO out, V a)
{
    g_application_surface_draw_enabled = a.cond();
}

void ogm::interpreter::fn::application_surface_is_enabled(VO out)
{
    out = g_application_surface_enabled;
}


void ogm::interpreter::fn::application_get_position(VO out)
{
    out.array_ensure();

    TexturePage* surface = display->m_textures.get_surface(g_application_surface);

    if (!surface)
    {
        out = 0.0f;
        return;
    }

    real_t maxw = display->get_window_dimensions().x;
    real_t maxh = display->get_window_dimensions().y;
    real_t w = surface->m_dimensions.x;
    real_t h = surface->m_dimensions.y;

    real_t scale_w = maxw / w;
    real_t scale_h = maxh / h;
    if (scale_w < scale_h)
    // scale to maximum width
    {
        real_t hmargin = (maxh - scale_w*h)/2;
        out.array_get(OGM_2DARRAY_DEFAULT_ROW 0) = 0.0f;
        out.array_get(OGM_2DARRAY_DEFAULT_ROW 1) = static_cast<real_t>(static_cast<uint32_t>(hmargin));
        out.array_get(OGM_2DARRAY_DEFAULT_ROW 2) = static_cast<real_t>(static_cast<uint32_t>(maxw));
        out.array_get(OGM_2DARRAY_DEFAULT_ROW 3) = static_cast<real_t>(static_cast<uint32_t>(maxh - hmargin));
    }
    else
    // scale to maximum height
    {
        real_t wmargin = (maxw - scale_h*w)/2;
        out.array_get(OGM_2DARRAY_DEFAULT_ROW 0) = wmargin;
        out.array_get(OGM_2DARRAY_DEFAULT_ROW 1) = 0.0f;
        out.array_get(OGM_2DARRAY_DEFAULT_ROW 2) = static_cast<real_t>(static_cast<uint32_t>(maxw - wmargin));
        out.array_get(OGM_2DARRAY_DEFAULT_ROW 3) = static_cast<real_t>(static_cast<uint32_t>(maxh));
    }
}

void ogm::interpreter::fn::getv::application_surface(VO out)
{
    if (g_application_surface == -1)
    {
        fn::ogm_create_application_surface(out);
    }
    out = static_cast<real_t>(g_application_surface);
}
