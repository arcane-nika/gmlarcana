#include "libpre.h"
    #include "fn_asset.h"
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

// NEW FEATURE (signature: annika marie schlögel)
// BACKGROUND ASSET IMPORTER/LOADER
void ogm::interpreter::fn::background_add(
    VO out,
    V fname,
    V removeback,
    V smooth)
{
    if (removeback.cond() || smooth.cond())
    {
        std::cout
        << "\033[38;5;208m"
        << "WARNING: background_add(removeback="
        << removeback.cond()
        << ", smooth="
        << smooth.cond()
        << ") currently ignores these parameters."
        << "\033[0m\n";
    }

    asset_index_t index;

    AssetBackground* bg = frame.m_assets.add_asset<AssetBackground>(
        "<dynamic background>",
        &index
    );

    bg->m_image = asset::Image(
        frame.m_fs.resolve_file_path(
            fname.castCoerce<std::string>()
        )
    );

    bg->m_image.realize_data();

    bg->m_dimensions = bg->m_image.m_dimensions;

    TexturePage* tpage =
    frame.m_display->m_textures.create_tpage_from_callback(
        [bg]() -> asset::Image*
        {
            return &bg->m_image;
        }
    );

    frame.m_display->m_textures.bind_asset_to_tpage_location(
        { index },
        tpage,
        {
            0.0, 0.0,
            1.0, 1.0
        }
    );

    out = static_cast<real_t>(index);
}
// NEW FEATURE END

void ogm::interpreter::fn::background_exists(VO out, V bg)
{
    asset_index_t index = bg.castCoerce<asset_index_t>();
    out = !!frame.m_assets.get_asset<AssetBackground*>(index);
}

void ogm::interpreter::fn::background_get_width(VO out, V vb)
{
    AssetBackground* bg = frame.m_assets.get_asset<AssetBackground*>(vb.castCoerce<asset_index_t>());
    if (bg)
    {
        out = static_cast<real_t>(bg->m_dimensions.x);
    }
    else
    {
        out = 0.0;
    }
}

void ogm::interpreter::fn::background_get_height(VO out, V vb)
{
    AssetBackground* bg = frame.m_assets.get_asset<AssetBackground*>(vb.castCoerce<asset_index_t>());
    if (bg)
    {
        out = static_cast<real_t>(bg->m_dimensions.y);
    }
    else
    {
        out = 0.0;
    }
}

void ogm::interpreter::fn::background_get_name(VO out, V vb)
{
    asset_index_t index = vb.castCoerce<asset_index_t>();
    if (frame.m_assets.get_asset<AssetBackground*>(index))
    {
        out = frame.m_assets.get_asset_name(index);
    }
    else
    {
        out = "<undefined>";
    }
}

void ogm::interpreter::fn::background_duplicate(VO out, V vb)
{
    AssetBackground* bg = frame.get_asset_from_variable<AssetBackground>(vb);
    out = bg->m_dimensions.x;
    asset_index_t asset_index;
    AssetBackground* new_bg = frame.m_assets.add_asset<AssetBackground>(
        ("?unnamed" + std::to_string(frame.m_assets.asset_count())).c_str(),
        &asset_index
    );
    *new_bg = *bg;
    out = static_cast<real_t>(asset_index);
    frame.m_display->m_textures.bind_asset_to_callback(
        asset_index,
        [bg]() { return &bg->m_image; }
    );
}

// NEW FEATURE (signature: annika marie schlögel)
// DESTRUCTS AN BG ASSET IN GPU
void ogm::interpreter::fn::background_delete(
    VO out,
    V background
)
{
    asset_index_t index = background.castCoerce<asset_index_t>();

    frame.m_display->m_textures.free_texture({ index });
    frame.m_assets.free_asset<AssetBackground>(index);
}
// NEW FEATURE END
