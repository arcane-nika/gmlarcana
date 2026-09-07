#include <string>

#include "ogm/asset/ShaderLanguage.hpp"

namespace ogm { namespace interpreter
{
    extern const char* k_vertex_shader_pre;
    extern const char* k_fragment_shader_pre;
    
    extern const char* k_default_vertex_shader_source;
    extern const char* k_default_geometry_shader_source;
    extern const char* k_default_fragment_shader_source;
    
    // CHANGED NAME (signature: annika marie schlögel)
    // split the desktop glsl and essl paths into two functions
    // adjusts source to be compatible with opengl.
    std::string fix_vertex_shader_source_desktop(std::string);
    std::string fix_fragment_shader_source_desktop(std::string);

    // NEW FEATURE (signature: annika marie schlögel)
    // essl pipeline
    std::string fix_vertex_shader_source_essl(std::string);
    std::string fix_fragment_shader_source_essl(std::string);

    std::string prepare_vertex_shader_source(asset::ShaderLanguage language, std::string source);
    std::string prepare_fragment_shader_source(asset::ShaderLanguage language, std::string source);
    // NEW FEATURE END
}}
