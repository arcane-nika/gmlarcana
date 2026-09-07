#pragma once

#include "Asset.hpp"
#include "ShaderLanguage.hpp"

namespace ogm
{
namespace asset
{

// MODIFIED FEATURE (signature: annika marie schlögel)
// included shader language as a parameter for ANGLE shader converter to read
class AssetShader : public Asset
{
public:
    ShaderLanguage m_language = ShaderLanguage::Unknown;

    std::string m_vertex_source;
    std::string m_pixel_source;
};
// MODIFIED FEATURE END

}
}
