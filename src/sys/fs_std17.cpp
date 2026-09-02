// implementation of util_sys.hpp using standard library functions from c++17

#ifdef CPP_FILESYSTEM_ENABLED

#include "ogm/sys/util_sys.hpp"
#include "fs_share.hpp"

#include <filesystem>
#include <random>

namespace ogm {

// MODIFIED FEATURE (signature: annika marie schlögel)
// upgraded to work with the new normalize_native_path function, which handles all paths correctly
bool create_directory(const std::string& path)
{
    return std::filesystem::create_directory(
        std::filesystem::path(normalize_native_path(path))
    );
}

bool remove_directory(const std::string& path)
{
    return std::filesystem::remove_all(
        std::filesystem::path(normalize_native_path(path))
    );
}

// NEW FEATURE (signature: annika marie schlögel)
bool delete_file(const std::string& path)
{
    try
    {
        return std::filesystem::remove(
            std::filesystem::path(normalize_native_path(path))
        );
    }
    catch (...)
    {
        return false;
    }
}

bool copy_file(
    const std::string& source,
    const std::string& destination
)
{
    try
    {
        return std::filesystem::copy_file(
            std::filesystem::path(normalize_native_path(source)),
            std::filesystem::path(normalize_native_path(destination)),
            std::filesystem::copy_options::overwrite_existing
        );
    }
    catch (...)
    {
        return false;
    }
}

bool rename_file(
    const std::string& source,
    const std::string& destination
)
{
    try
    {
        std::filesystem::rename(
            std::filesystem::path(normalize_native_path(source)),
            std::filesystem::path(normalize_native_path(destination))
        );

        return true;
    }
    catch (...)
    {
        return false;
    }
}
// NEW FEATURE END
// MODIFIED FEATURE END

std::string get_temp_root()
{
    return std::filesystem::temp_directory_path().string();
}

namespace {
    // https://stackoverflow.com/a/7114482
    typedef std::mt19937 MyRNG;  // the Mersenne Twister with a popular choice of parameters
    unsigned int seed_val = static_cast<unsigned int>(time(nullptr));           // populate somehow
    MyRNG rng{ seed_val };
    std::uniform_int_distribution<uint32_t> uint_dist;
}

std::string create_temp_directory()
{
    std::string root = std::filesystem::temp_directory_path().string();
    static int c = 0;

    while (true)
    {
        std::string subfolder = "ogm-tmp" + std::to_string(uint_dist(rng)) + "-" + std::to_string(c++);
        std::string joined = path_join(root, subfolder);
        if (!path_exists(joined))
        {
            create_directory(joined);
            return joined + std::string(1, PATH_SEPARATOR);
        }
    }
}

// MODIFIED FEATURE (signature: annika marie schlögel)
// upgraded to work with the new normalize_native_path function, which handles all paths correctly
void list_paths(const std::string& base, std::vector<std::string>& out)
{
    std::string normalized_base = normalize_native_path(base);

    if (!path_exists(normalized_base)) return;
    for (auto& p : std::filesystem::directory_iterator(normalized_base))
    {
        out.emplace_back(p.path().string());
    }
}

void list_paths_recursive(const std::string& base, std::vector<std::string>& out)
{
    std::string normalized_base = normalize_native_path(base);

    if (!path_exists(normalized_base)) return;
    for (auto& p : std::filesystem::recursive_directory_iterator(base))
    {
        out.emplace_back(p.path().string());
    }
}
// MODIFIED FEATURE END

}

#endif