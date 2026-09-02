#include "ogm/common/util.hpp"
#include "ogm/interpreter/Filesystem.hpp"
#include "sandbox/FilesystemHook.hpp"

#ifdef _WIN32
#include <shlwapi.h>
#include <windows.h>
#endif

#include <iostream>
#include <filesystem>
#include <optional>
#include <cctype>
#include <unordered_set>

// NEW FEATURE (signature: annika marie schlögel)
namespace
{
constexpr int FILE_FIND_ATTRIBUTE_DIRECTORY = 16;

char file_find_fold_case(char c)
{
    #ifdef _WIN32
    return static_cast<char>(
        std::tolower(static_cast<unsigned char>(c))
    );
    #else
    return c;
    #endif
}

std::string file_find_match_key(std::string value)
{
    for (char& c : value)
    {
        c = file_find_fold_case(c);
    }

    return value;
}

bool file_find_wildcard_match(
    const std::string& pattern,
    const std::string& value
)
{
    size_t pattern_index = 0;
    size_t value_index = 0;
    size_t last_star = std::string::npos;
    size_t star_value_index = 0;

    while (value_index < value.size())
    {
        if (
            pattern_index < pattern.size()
            && (
                pattern[pattern_index] == '?'
                || file_find_fold_case(pattern[pattern_index])
                    == file_find_fold_case(value[value_index])
            )
        )
        {
            ++pattern_index;
            ++value_index;
        }
        else if (
            pattern_index < pattern.size()
            && pattern[pattern_index] == '*'
        )
        {
            last_star = pattern_index++;
            star_value_index = value_index;
        }
        else if (last_star != std::string::npos)
        {
            pattern_index = last_star + 1;
            value_index = ++star_value_index;
        }
        else
        {
            return false;
        }
    }

    while (
        pattern_index < pattern.size()
        && pattern[pattern_index] == '*'
    )
    {
        ++pattern_index;
    }

    return pattern_index == pattern.size();
}

bool file_find_attributes_match(
    const std::filesystem::directory_entry& entry,
    int requested_attributes
)
{
    #ifdef _WIN32
    const DWORD actual_attributes =
        GetFileAttributesA(
            entry.path().string().c_str()
        );

    if (actual_attributes == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    // GameMaker fa_* constants use the classic
    // DOS/Win32 attribute bits 1 through 32.
    constexpr DWORD FILTERABLE_ATTRIBUTES =
        1   // fa_readonly
        | 2   // fa_hidden
        | 4   // fa_sysfile
        | 8   // fa_volumeid
        | 16  // fa_directory
        | 32; // fa_archive

    const DWORD rejected_attributes =
        actual_attributes
        & FILTERABLE_ATTRIBUTES
        & ~static_cast<DWORD>(requested_attributes);

    return rejected_attributes == 0;

    #else

    // The attribute flags are documented as Windows-only.
    // We still retain fa_directory on POSIX for projects
    // shared between the two targets.
    std::error_code error;

    if (entry.is_directory(error))
    {
        return !error
            && (
                requested_attributes
                & FILE_FIND_ATTRIBUTE_DIRECTORY
            );
    }

    return !error && entry.is_regular_file(error);

    #endif
}
}
// NEW FEATURE END

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

// REWRITTEN FEATURE (signature: annika marie schlögel)
// now supports proper platform-aware file handling with wildcards
std::string Filesystem::file_find_first(const std::string& pattern, int attributes)
{
    file_find_close();

    m_file_search.emplace();
    m_file_search->index = 0;

    const std::string clean_pattern = normalize_native_path(pattern);

    const std::filesystem::path search_path(clean_pattern);
    const std::string wildcard = search_path.filename().string();

    if (wildcard.empty())
    {
        m_file_search.reset();
        return "";
    }

    std::filesystem::path requested_directory = search_path.parent_path();

    if (requested_directory.empty())
    {
        requested_directory = ".";
    }

    std::vector<std::filesystem::path> directories;

    bool absolute = false;

    #if defined(_WIN32)
    absolute = !PathIsRelative(requested_directory.string().c_str());
    #else
    absolute = requested_directory.is_absolute();
    #endif

    if (absolute)
    {
        directories.emplace_back(resolve_file_path(requested_directory.string(), false));
    }
    else
    {
        // GameMaker read order:
        // save/working area first, then included files.
        directories.emplace_back(case_insensitive_native_path(m_working_directory, requested_directory.string()));

        if (!m_included_directory.empty())
        {
            directories.emplace_back(case_insensitive_native_path(m_included_directory, requested_directory.string()));
        }
    }

    std::unordered_set<std::string> seen_names;

    for (const std::filesystem::path& directory : directories)
    {
        std::error_code error;

        std::filesystem::directory_iterator iterator(directory, std::filesystem::directory_options::skip_permission_denied, error);

        const std::filesystem::directory_iterator end;

        while (!error && iterator != end)
        {
            const std::filesystem::directory_entry& entry = *iterator;

            const std::string name = entry.path().filename().string();

            if (file_find_wildcard_match(wildcard, name))
            {
                const std::string match_key = file_find_match_key(name);

                if (file_find_attributes_match(entry, attributes) && seen_names.insert(match_key).second)
                {
                    m_file_search->matches.push_back(name);
                }
            }

            iterator.increment(error);
        }
    }

    if (m_file_search->matches.empty())
    {
        m_file_search.reset();
        return "";
    }

    return m_file_search->matches.front();
}
// REWRITTEN FEATURE END

}}
