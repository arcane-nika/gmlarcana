
#include "ogm/interpreter/display/shader_translate.hpp"

namespace ogm {

interpreter::ShaderTranslationResult translate_shader(
    asset::ShaderLanguage language,
    interpreter::ShaderStage,
    const std::string& source
)
{
    if (language == asset::ShaderLanguage::GLSL ||
        language == asset::ShaderLanguage::Unknown)
    {
        return {true, source, ""};
    }

    return {
        false,
        "",
        "No translator implemented for this shader language."
    };
}

}