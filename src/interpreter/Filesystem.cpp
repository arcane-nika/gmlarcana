#include "ogm/common/util.hpp"
#include "ogm/interpreter/Filesystem.hpp"
#include "sandbox/FilesystemHook.hpp"

#ifdef _WIN32
#include <shlwapi.h>
#endif

#include <iostream>
#include <filesystem>
#include <optional>

namespace ogm { namespace interpreter
{
using namespace ogm;

Filesystem::Filesystem()
{
    #ifdef OGM_FS_HOOK
    if (can_hook())
    {
        m_sandbox_impl = SandboxImpl::HOOK_WORKINGDIR;
    }
    else
    #endif
    {
        m_sandbox_impl = SandboxImpl::TEMPDIR;
    }
}

void Filesystem::init()
{
    if (!is_init)
    {
        is_init = true;
        #ifdef OGM_FS_HOOK
        if (m_sandbox_impl == SandboxImpl::HOOK_WORKINGDIR)
        {
            hook_fs_open();
        }
        #endif
    }
}

bool Filesystem::file_exists(const std::string& path)
{
    init();
    std::ifstream ifs(resolve_file_path(path));
    return ifs.good();
}

template<FileAccessType type, bool binary>
file_handle_id_t Filesystem::open_file(const std::string& _path)
{
    init();

    // TODO: investigate why this is necessary
    #ifdef _MSC_VER
    int
    #else
    auto
    #endif
    access_mode = (type == FileAccessType::read)
        ? std::ios::in
        : std::ios::out;
    if (type == FileAccessType::append)
    {
        access_mode |= std::ios::app;
    }

    std::string path = resolve_file_path(_path, type != FileAccessType::read);

    // TODO: instead of emplacing at back, scan for first non-active (m_active) file,
    // and replace it.
    m_files.emplace_back();
    FileHandle& fh = m_files.back();
    fh.m_active = true;
    fh.m_type = type;
    if (type == FileAccessType::read)
    {
        fh.m_ifstream = new std::ifstream(path, access_mode);
    }
    else
    {
        fh.m_ofstream = new std::ofstream(path, access_mode);
    }
    if (type == FileAccessType::read && !fh.m_ifstream->good())
    {
        m_files.pop_back();
        return -1;
    }
    else if (type != FileAccessType::read && !fh.m_ofstream->good())
    {
        m_files.pop_back();
        return -1;
    }
    else
    {
        return m_files.size();
    }
}

template
file_handle_id_t Filesystem::open_file<FileAccessType::read, false>(const std::string& path);
template
file_handle_id_t Filesystem::open_file<FileAccessType::write, false>(const std::string& path);
template
file_handle_id_t Filesystem::open_file<FileAccessType::append, false>(const std::string& path);
template
file_handle_id_t Filesystem::open_file<FileAccessType::read, true>(const std::string& path);
template
file_handle_id_t Filesystem::open_file<FileAccessType::write, true>(const std::string& path);
template
file_handle_id_t Filesystem::open_file<FileAccessType::append, true>(const std::string& path);

void Filesystem::close_file(file_handle_id_t id)
{
    if (id <= m_files.size())
    {
        m_files.at(id - 1).cleanup();
    }
}

// helper function for resolve_file_path
bool Filesystem::file_is_included(const std::string& path)
{
    init();
    if (m_included_directory == "") return false;
    std::ifstream infile(case_insensitive_native_path(m_included_directory, path));
    return infile.good();
}

std::string Filesystem::resolve_file_path(const std::string& path, bool write)
{
    init();

    // BUG FIX (signature: annika marie schlögel)
    // prepare the file paths for POSIX based systems if necessary
    std::string clean_path = normalize_native_path(path);
    // BUG FIX END

    if (m_sandbox_impl == SandboxImpl::HOOK_WORKINGDIR) return clean_path;

    if (m_sandbox_impl == SandboxImpl::NONE) return clean_path;

    bool absolute = false;

    #if defined(_WIN32)
    if (!PathIsRelative(clean_path.c_str()))
    {
        absolute = true;
    }
    #else
    if (starts_with(clean_path, "/"))
    {
        absolute = true;
    }
    #endif


    // look up case-insensitive path for absolute paths, but don't modify otherwise.
    if (absolute)
    {
        #if defined(_WIN32)
        char buff[MAX_PATH];
        for (size_t i = 0; i < MAX_PATH; ++i)
        {
            buff[i] = 0;
        }
        strcpy(buff, clean_path.c_str());
        PathStripToRootA(buff);
        return case_insensitive_native_path(buff, clean_path.substr(strlen(buff)));
        #else
        return case_insensitive_native_path("/", clean_path.substr(1));
        #endif
    }

    // look up case-insensitive path in working directory.
    if (write)
    {
    working_path:
        return case_insensitive_native_path(m_working_directory, clean_path);
    }
    else
    {
        if (file_is_included(clean_path))
        {
            return case_insensitive_native_path(m_included_directory, clean_path);
        }
        else
        {
            goto working_path;
        }
    }
}

// NEW FEATURE (signature: annika marie schlögel)
// FUNCTION DEFINITIONS
void Filesystem::file_find_close()
{
    m_file_search.reset();
}

std::string Filesystem::file_find_next()
{
    if (!m_file_search)
        return "";

    ++m_file_search->index;

    if (m_file_search->index >= m_file_search->matches.size())
    {
        m_file_search.reset();
        return "";
    }

    return m_file_search->matches[m_file_search->index];
}

// TODO: VISIT LATER, NOT FULLY IMPLEMENTED
std::string Filesystem::file_find_first(const std::string& pattern, int attributes)
{
    file_find_close();

    m_file_search.emplace();
    m_file_search->index = 0;

    // PROBABLE BUG FIX (signature: annika marie schlögel)

    // LEGACY CODE
    //std::string resolved = resolve_file_path(pattern, false);

    // NEW CODE (changing write mode to true, basically every imported api/resource would not want the project dir as which the false writing flag is basically redirecting to) NEVERMIND
    std::string resolved = resolve_file_path(pattern, false);

    // PROBABLE BUG FIX END

    // DEBUG PRINT (signature: annika marie schlögel)
    /*std::cout << "pattern   = " << pattern << '\n';
    std::cout << "resolved  = " << resolved << '\n';
    std::cout << "working   = " << m_working_directory << '\n';
    std::cout << "included  = " << m_included_directory << '\n';*/
    // DEBUG PRINT END

    //std::cout << "ENTERING search_path\n"; // DEBUG PRINT (signature: annika marie schlögel)
    std::filesystem::path search_path(resolved);
    //std::cout << "SUCCESSFUL EXECUTION search_path\n"; // DEBUG PRINT (signature: annika marie schlögel)

    //std::cout << "ENTERING directory = search_path.parent_path\n"; // DEBUG PRINT (signature: annika marie schlögel)
    std::filesystem::path directory = search_path.parent_path();
    //std::cout << "SUCCESSFUL EXECUTION directory = search_path.parent_path\n"; // DEBUG PRINT (signature: annika marie schlögel)

    //std::cout << "ENTERING wildcard = search_path...\n"; // DEBUG PRINT (signature: annika marie schlögel)
    std::string wildcard = search_path.filename().string();
    //std::cout << "SUCCESSFUL EXECUTION wildcard = search_path...\n"; // DEBUG PRINT (signature: annika marie schlögel)

    for (const auto& entry : std::filesystem::directory_iterator(directory))
    {
        if (wildcard != "*")
            continue;

        bool accept = true;

        if (attributes & 16)
            accept &= entry.is_directory();

        if (!accept)
            continue;

        m_file_search->matches.push_back(
            entry.path().filename().string()
        );
    }

    if (m_file_search->matches.empty())
    {
        m_file_search.reset();
        return "";
    }

    return m_file_search->matches.front();
}
// NEW FEATURE END

}}
