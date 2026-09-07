
#pragma once

namespace ogm
{

namespace asset
{

// NEW FEATURE (signature: annika marie schlögel)
// Shader lang enum to specify which shader is used (for the ANGLE shader converter)
enum class ShaderLanguage
{
    Unknown,
    GLSLES,
    GLSL,
    HLSL9
};

inline ShaderLanguage shader_language_from_string(std::string_view value)
{
    if (value == "GLSLES")
        return ShaderLanguage::GLSLES;

    if (value == "GLSL")
        return ShaderLanguage::GLSL;

    if (value == "HLSL9")
        return ShaderLanguage::HLSL9;

    return ShaderLanguage::Unknown;
}
// NEW FEATURE END

}

}