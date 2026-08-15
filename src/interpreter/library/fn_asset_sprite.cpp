#include "libpre.h"
    #include "fn_asset.h"
#include "libpost.h"

#include "ogm/interpreter/Variable.hpp"
#include "ogm/common/error.hpp"
#include "ogm/common/util.hpp"
#include "ogm/interpreter/Executor.hpp"
#include "ogm/interpreter/execute.hpp"
#include "ogm/interpreter/display/Display.hpp"
#include "ogm/asset/Image.hpp"

#include <string>
#include "ogm/common/error.hpp"

#include <cctype>
#include <cstdlib>

using namespace ogm::interpreter;
using namespace ogm::interpreter::fn;

#define frame staticExecutor.m_frame

// PROBABLE BUG FIX (signature: annika marie schlögel)
void ogm::interpreter::fn::sprite_exists(VO out, V vs)
{
    asset_index_t index = vs.castCoerce<asset_index_t>();

    // LEGACY CODE
    //out = !!frame.m_assets.get_asset<AssetBackground*>(index);

    // NEW CODE
    out = !!frame.m_assets.get_asset<AssetSprite*>(index);
}
// BUG FIX END

void ogm::interpreter::fn::sprite_get_number(VO out, V vs)
{
    AssetSprite* s = frame.get_asset_from_variable<AssetSprite>(vs);
    ogm_assert(s);
    out = static_cast<real_t>(s->image_count());
}

void ogm::interpreter::fn::sprite_get_width(VO out, V vs)
{
    AssetSprite* s = frame.get_asset_from_variable<AssetSprite>(vs);
    ogm_assert(s);
    out = static_cast<real_t>(s->m_dimensions.x);
}

void ogm::interpreter::fn::sprite_get_height(VO out, V vs)
{
    AssetSprite* s = frame.get_asset_from_variable<AssetSprite>(vs);
    ogm_assert(s);
    out = static_cast<real_t>(s->m_dimensions.y);
}

void ogm::interpreter::fn::sprite_get_xoffset(VO out, V vs)
{
    AssetSprite* s = frame.get_asset_from_variable<AssetSprite>(vs);
    ogm_assert(s);
    out = static_cast<real_t>(s->m_offset.x);
}

void ogm::interpreter::fn::sprite_get_yoffset(VO out, V vs)
{
    AssetSprite* s = frame.get_asset_from_variable<AssetSprite>(vs);
    ogm_assert(s);
    out = static_cast<real_t>(s->m_offset.y);
}

void ogm::interpreter::fn::sprite_get_bbox_left(VO out, V vs)
{
    AssetSprite* s = frame.get_asset_from_variable<AssetSprite>(vs);
    ogm_assert(s);
    out = static_cast<real_t>(s->m_aabb.m_start.x);
}

void ogm::interpreter::fn::sprite_get_bbox_top(VO out, V vs)
{
    AssetSprite* s = frame.get_asset_from_variable<AssetSprite>(vs);
    ogm_assert(s);
    out = static_cast<real_t>(s->m_aabb.m_start.y);
}

void ogm::interpreter::fn::sprite_get_bbox_right(VO out, V vs)
{
    AssetSprite* s = frame.get_asset_from_variable<AssetSprite>(vs);
    ogm_assert(s);
    out = static_cast<real_t>(s->m_aabb.m_end.x);
}

void ogm::interpreter::fn::sprite_get_bbox_bottom(VO out, V vs)
{
    AssetSprite* s = frame.get_asset_from_variable<AssetSprite>(vs);
    ogm_assert(s);
    out = static_cast<real_t>(s->m_aabb.m_end.y);
}

void ogm::interpreter::fn::sprite_get_speed(VO out, V vs)
{
    AssetSprite* s = frame.get_asset_from_variable<AssetSprite>(vs);
    ogm_assert(s);
    out = static_cast<real_t>(s->m_speed);
}

void ogm::interpreter::fn::sprite_get_speed_type(VO out, V vs)
{
    AssetSprite* s = frame.get_asset_from_variable<AssetSprite>(vs);
    ogm_assert(s);
    out = static_cast<real_t>(
        s->m_speed_real_time
            ? constant::spritespeed_framespersecond
            : constant::spritespeed_framespergameframe
    );
}

void ogm::interpreter::fn::sprite_set_speed(VO out, V vs, V vspd, V vspdtype)
{
    AssetSprite* s = frame.get_asset_from_variable<AssetSprite>(vs);
    ogm_assert(s);

    s->m_speed = vspd.castCoerce<real_t>();
    s->m_speed_real_time = (vspdtype.castCoerce<real_t>() == constant::spritespeed_framespersecond);
}

// MODIFIED FEATURE (signature: annika marie schlögel), removed exception put debug print
void ogm::interpreter::fn::sprite_create_from_surface(VO out, V surface, V vx, V vy, V vw, V vh, V removebackground, V smooth, V xo, V yo)
{
    if (removebackground.cond() || smooth.cond())
    {
        std::cout
        << "\033[38;5;208m"
        << "WARNING: sprite_create_from_surface(removeback="
        << removebackground.cond()
        << ", smooth="
        << smooth.cond()
        << ") currently ignores these parameters."
        << "\033[0m\n";
    }

    asset_index_t asset_index;
    AssetSprite* sprite = frame.m_assets.add_asset<AssetSprite>("<dynamic sprite>", &asset_index);
    sprite->m_dimensions = {
      vw.castCoerce<coord_t>(),
      vh.castCoerce<coord_t>()
    };

    sprite->m_aabb = { { 0, 0 }, sprite->m_dimensions};
    sprite->m_offset = {
      xo.castCoerce<coord_t>(),
      yo.castCoerce<coord_t>()
    };
    sprite->m_subimage_count = 1;

    geometry::Vector<coord_t> src_coord = { vx.castCoerce<coord_t>(), vy.castCoerce<coord_t>()};

    surface_id_t si = surface.castCoerce<uint32_t>();
    TextureView* view = frame.m_display->m_textures.get_surface_texture_view(si);
    frame.m_display->m_textures.bind_asset_copy_texture(
        { asset_index, 0 },
        view,
        geometry::AABB<coord_t>{ src_coord, sprite->m_dimensions + src_coord }
    );
    
    out = static_cast<real_t>(asset_index);
}

void ogm::interpreter::fn::sprite_add(VO out, V fname, V vimgnum, V removeback, V smooth, V xorig, V yorig)
{
    if (smooth.cond())
    {
        std::cout
        << "\033[38;5;208m"
        << "WARNING: sprite_add(smooth="
        << smooth.cond()
        << ") currently ignores these parameters."
        << "\033[0m\n";
    }

    size_t imgnum = std::max(1, vimgnum.castCoerce<int32_t>());
    
    asset_index_t index;
    AssetSprite* sprite = frame.m_assets.add_asset<AssetSprite>(
        "<dynamic sprite>", &index
    );
    
    sprite->m_offset = {
        xorig.castCoerce<coord_t>(),
        yorig.castCoerce<coord_t>()
    };
    sprite->m_shape = AssetSprite::rectangle;
    sprite->m_subimage_count = imgnum;
    
    // load image (slight hack: store in the subimage vector, but it is actually
    // the whole image.)
    asset::Image& image = sprite->m_subimages.emplace_back(
        frame.m_fs.resolve_file_path(fname.castCoerce<std::string>())
    );
    image.realize_data();

    if (removeback.cond())
    {
        image.apply_color_key_from_bottom_left();
    }
    
    TexturePage* tpage = frame.m_display->m_textures.create_tpage_from_callback(
        [sprite]() -> asset::Image* {
            return &sprite->m_subimages.front();
        }
    );
    
    // split image
    sprite->m_dimensions = geometry::Vector<int32_t>{
        static_cast<int32_t>(image.m_dimensions.x / imgnum),
        image.m_dimensions.y
    };
    
    for (size_t i = 0; i < imgnum; ++i)
    {
        int32_t start = i * sprite->m_dimensions.x;
        coord_t invxrange = 1.0 / static_cast<coord_t>(image.m_dimensions.x);
        frame.m_display->m_textures.bind_asset_to_tpage_location(
            { index, i },
            tpage,
            {
                invxrange * start, 0,
                invxrange * (start + sprite->m_dimensions.x), 1
            }
        );
    }
    
    out = static_cast<real_t>(index);
}

void ogm::interpreter::fn::sprite_add_from_surface(VO out, V index, V surface, V vx, V vy, V vw, V vh, V removebackground, V smooth)
{
    if (removebackground.cond() || smooth.cond())
    {
        std::cout
        << "\033[38;5;208m"
        << "WARNING: sprite_add_from_surface(removeback="
        << removebackground.cond()
        << ", smooth="
        << smooth.cond()
        << ") currently ignores these parameters."
        << "\033[0m\n";
    }

    asset_index_t asset_index = index.castCoerce<asset_index_t>();
    AssetSprite* sprite = frame.m_assets.get_asset<AssetSprite*>(asset_index);
    geometry::Vector<coord_t> dim = {
        vw.castCoerce<coord_t>(),
        vh.castCoerce<coord_t>()
    };

    sprite->m_dimensions.x = std::max(sprite->m_dimensions.x, dim.x);
    sprite->m_dimensions.y = std::max(sprite->m_dimensions.y, dim.y);

    geometry::Vector<coord_t> src_coord = { vx.castCoerce<coord_t>(), vy.castCoerce<coord_t>()};

    surface_id_t si = surface.castCoerce<uint32_t>();
    TextureView* view = frame.m_display->m_textures.get_surface_texture_view(si);
    frame.m_display->m_textures.bind_asset_copy_texture(
        { asset_index, sprite->m_subimage_count++ },
        view,
        geometry::AABB<coord_t>{ src_coord, dim + src_coord }
    );
}
// MODIFIED FEATURE END

// NEW FEATURE (signature: annika marie schlögel)
void ogm::interpreter::fn::sprite_set_offset(VO out, V vs, V vx, V vy)
{
    AssetSprite* s = frame.get_asset_from_variable<AssetSprite>(vs);
    ogm_assert(s);

    s->m_offset.x = vx.castCoerce<coord_t>();
    s->m_offset.y = vy.castCoerce<coord_t>();
}
// NEW FEATURE END

// NEW FEATURE (signature: annika marie schlögel)
// helper for copying, used in sprite_duplicate and sprite_assign
static void copy_sprite_asset(AssetSprite* dst, const AssetSprite* src)
{
    ogm_assert(dst);
    ogm_assert(src);

    dst->m_offset = src->m_offset;
    dst->m_dimensions = src->m_dimensions;
    dst->m_aabb = src->m_aabb;
    dst->m_shape = src->m_shape;
    dst->m_subimage_count = src->m_subimage_count;
    dst->m_speed = src->m_speed;
    dst->m_speed_real_time = src->m_speed_real_time;

    dst->m_subimages = src->m_subimages;

    // prevent mem leakage from the allocations before deletion by clear(), as there is no destructor
    for (auto& raster : dst->m_raster)
    {
        delete[] raster.m_data;
        raster.m_data = nullptr;
    }

    dst->m_raster.clear();
    dst->m_raster.reserve(src->m_raster.size());

    for (const auto& src_raster : src->m_raster)
    {
        dst->m_raster.emplace_back();

        auto& dst_raster = dst->m_raster.back();

        dst_raster.m_width = src_raster.m_width;
        dst_raster.m_length = src_raster.m_length;

        if (src_raster.m_data && src_raster.m_length)
        {
            dst_raster.m_data = new bool[src_raster.m_length];

            std::copy(
                src_raster.m_data,
                src_raster.m_data + src_raster.m_length,
                dst_raster.m_data
            );
        }
        else
        {
            dst_raster.m_data = nullptr;
        }
    }
}

void ogm::interpreter::fn::sprite_duplicate(VO out, V vsprite)
{
    const asset_index_t src_index = vsprite.castCoerce<asset_index_t>();

    AssetSprite* src =
        frame.m_assets.get_asset<AssetSprite*>(src_index);

    if (!src)
    {
        out = k_no_asset;
        return;
    }

    asset_index_t dst_index;

    AssetSprite* dst =
        frame.m_assets.add_asset<AssetSprite>(
            "<dynamic sprite>",
            &dst_index
        );

    copy_sprite_asset(dst, src);

    for (size_t i = 0; i < src->image_count(); ++i)
    {
        TextureView* tv =
            frame.m_display->m_textures.get_texture(
                { src_index, i }
            );

        frame.m_display->m_textures.bind_asset_copy_texture(
            { dst_index, i },
            tv,
            {
                0,
                0,
                static_cast<uint32_t>(src->m_dimensions.x),
                static_cast<uint32_t>(src->m_dimensions.y)
            }
        );
    }

    out = static_cast<real_t>(dst_index);
}

void ogm::interpreter::fn::sprite_assign(VO out, V vdst, V vsrc)
{
    const asset_index_t dst_index =
        vdst.castCoerce<asset_index_t>();

    const asset_index_t src_index =
        vsrc.castCoerce<asset_index_t>();

    AssetSprite* dst =
        frame.m_assets.get_asset<AssetSprite*>(dst_index);

    AssetSprite* src =
        frame.m_assets.get_asset<AssetSprite*>(src_index);

    if (!dst || !src)
    {
        return;
    }

    for (size_t i = 0; i < dst->image_count(); ++i)
    {
        frame.m_display->m_textures.free_texture(
            { dst_index, i }
        );
    }

    copy_sprite_asset(dst, src);

    for (size_t i = 0; i < src->image_count(); ++i)
    {
        TextureView* tv =
            frame.m_display->m_textures.get_texture(
                { src_index, i }
            );

        frame.m_display->m_textures.bind_asset_copy_texture(
            { dst_index, i },
            tv,
            {
                0,
                0,
                static_cast<uint32_t>(src->m_dimensions.x),
                static_cast<uint32_t>(src->m_dimensions.y)
            }
        );
    }
}

void ogm::interpreter::fn::sprite_delete(VO out, V vsprite)
{
    const asset_index_t index =
        vsprite.castCoerce<asset_index_t>();

    AssetSprite* sprite =
        frame.m_assets.get_asset<AssetSprite*>(index);

    if (!sprite)
    {
        return;
    }

    for (size_t i = 0; i < sprite->image_count(); ++i)
    {
        frame.m_display->m_textures.free_texture(
            { index, i }
        );
    }

    frame.m_assets.free_asset<AssetSprite>(index);
}

void ogm::interpreter::fn::sprite_replace(
    VO out,
    V sprite,
    V filename,
    V imagenumb,
    V removeback,
    V smooth,
    V xorig,
    V yorig
)
{
    asset_index_t index = sprite.castCoerce<asset_index_t>();
    AssetSprite* spr = frame.m_assets.get_asset<AssetSprite*>(index);

    if (!spr)
    {
        return;
    }

    // Free old GPU textures.
    for (size_t i = 0; i < spr->image_count(); ++i)
    {
        frame.m_display->m_textures.free_texture({ index, i });
    }

    // Free old collision rasters.
    for (auto& raster : spr->m_raster)
    {
        delete[] raster.m_data;
        raster.m_data = nullptr;
    }

    spr->m_raster.clear();
    spr->m_subimages.clear();

    // ----- from here on this is essentially sprite_add -----

    if (smooth.cond())
    {
        std::cout
        << "\033[38;5;208m"
        << "WARNING: sprite_replace(smooth="
        << smooth.cond()
        << ") currently ignores this parameter."
        << "\033[0m\n";
    }

    size_t imgnum = std::max(1, imagenumb.castCoerce<int32_t>());

    spr->m_offset = {
        xorig.castCoerce<coord_t>(),
        yorig.castCoerce<coord_t>()
    };

    spr->m_shape = AssetSprite::rectangle;
    spr->m_subimage_count = imgnum;

    asset::Image& image = spr->m_subimages.emplace_back(
        frame.m_fs.resolve_file_path(filename.castCoerce<std::string>())
    );

    image.realize_data();

    if (removeback.cond())
    {
        image.apply_color_key_from_bottom_left();
    }

    TexturePage* tpage =
        frame.m_display->m_textures.create_tpage_from_callback(
            [spr]() -> asset::Image*
            {
                return &spr->m_subimages.front();
            }
        );

    spr->m_dimensions = {
        static_cast<int32_t>(image.m_dimensions.x / imgnum),
        image.m_dimensions.y
    };

    for (size_t i = 0; i < imgnum; ++i)
    {
        int32_t start = i * spr->m_dimensions.x;
        coord_t invxrange = 1.0 / static_cast<coord_t>(image.m_dimensions.x);

        frame.m_display->m_textures.bind_asset_to_tpage_location(
            { index, i },
            tpage,
            {
                invxrange * start,
                0,
                invxrange * (start + spr->m_dimensions.x),
                1
            }
        );
    }
}

void ogm::interpreter::fn::sprite_merge(
    VO out,
    V vsprite1,
    V vsprite2
)
{
    const asset_index_t dst_index =
        vsprite1.castCoerce<asset_index_t>();

    const asset_index_t src_index =
        vsprite2.castCoerce<asset_index_t>();

    AssetSprite* dst =
        frame.m_assets.get_asset<AssetSprite*>(dst_index);

    AssetSprite* src =
        frame.m_assets.get_asset<AssetSprite*>(src_index);

    if (!dst || !src)
    {
        return;
    }

    const size_t original_count = dst->image_count();

    for (size_t i = 0; i < src->image_count(); ++i)
    {
        TextureView* tv =
            frame.m_display->m_textures.get_texture(
                { src_index, i }
            );

        frame.m_display->m_textures.bind_asset_copy_texture(
            { dst_index, original_count + i },
            tv,
            {
                0,
                0,
                static_cast<uint32_t>(src->m_dimensions.x),
                static_cast<uint32_t>(src->m_dimensions.y)
            }
        );
    }

    dst->m_subimage_count =
        original_count + src->image_count();
}
// NEW FEATURE END
