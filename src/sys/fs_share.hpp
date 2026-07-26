#pragma once

// shared between the various fs implementations and fs_common.cpp
namespace ogm::fs
{
    extern int terminal_colours_are_supported;
}

// NEW FEATURE (signature: annika marie schlögel)
std::vector<std::string> __glob(const std::string& search_path);
