#pragma once

#include "ogm/asset/ShaderLanguage.hpp"

#include <string>

namespace ogm::interpreter
{
enum class ShaderStage
{
    Vertex,
    Fragment
};

struct ShaderTranslationResult
{
    bool success;
    std::string source;
    std::string log;
};

ShaderTranslationResult translate_shader(
    asset::ShaderLanguage language,
    ShaderStage stage,
    const std::string& source
);
}