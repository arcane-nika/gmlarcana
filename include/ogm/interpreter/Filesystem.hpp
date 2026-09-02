#pragma once

#include "ogm/common/types.hpp"
#include "ogm/sys/util_sys.hpp"
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <optional>
#include <iostream>

namespace ogm { namespace interpreter
{
    typedef int32_t file_handle_id_t;

    enum class FileAccessType
    {
        read,
        write,
        append
    };

    // scheme by which filesystem sandbox is Implemented
    // (this matters for DLLs)
    enum class SandboxImpl
    {
        // make no attempt to respect datafiles directory
        NONE,

        // read from datafiles/ directory, write to present directory
        // this only affects functions OGM knows about, so
        // DLLs may not find files they are looking for.
        WORKINGDIR,


        // as above, but hook into the filesystem IO functions to
        // ensure even DLLs find the files they are looking for.
        HOOK_WORKINGDIR,

        // create a temporary directory to host datafiles,
        // let that be the working directory.
        TEMPDIR
    };

    struct FileHandle
    {
        bool m_active;
        FileAccessType m_type;
        std::string m_path;
        union
        {
            std::ifstream* m_ifstream;
            std::ofstream* m_ofstream;
        };

        inline void cleanup()
        {
            m_active = false;
            switch (m_type)
            {
            case FileAccessType::read:
                if (m_ifstream) delete m_ifstream;
                m_ifstream = nullptr;
                break;
            case FileAccessType::write:
            case FileAccessType::append:
                if (m_ofstream) delete m_ofstream;
                m_ofstream = nullptr;
                break;
            }
        }

        ~FileHandle()
        {
            cleanup();
        }

        FileHandle()=default;
        FileHandle(FileHandle&& fh)
            : m_active(fh.m_active)
            , m_type(fh.m_type)
            , m_path(fh.m_path)
            , m_ifstream(fh.m_ifstream)
        {
            fh.m_ifstream = nullptr;
        }
    };

    // resolves paths to "working directory paths", which are either in
    // the save folder or in the included files directory.
    class Filesystem
    {
        // NEW FEATURE (signature: annika marie schlögel)
        // FILE SEARCH
        struct FileSearch
        {
            std::vector<std::string> matches;
            size_t index = 0;
        };

        std::optional<FileSearch> m_file_search;
        // NEW FEATURE END

        // open files list
        std::vector<FileHandle> m_files;

        std::string m_working_directory = "." + std::string(1, PATH_SEPARATOR);
        std::string m_included_directory = "";

        // TODO: move this to %APPDATA% or wherever.
        // NEW FEATURE (signature: annika marie schlögel)
        // actually did this over project name and constructing the appdata path in game_save_id
        std::string m_project_name = "";
        // NEW FEATURE END

        SandboxImpl m_sandbox_impl;
        bool is_init=false;

        void init();
    public:

        Filesystem();

        bool file_exists(const std::string&);

        // returns 0 on failure
        template<FileAccessType, bool binary=false>
        file_handle_id_t open_file(const std::string&);

        FileHandle& get_file_handle(file_handle_id_t id)
        {
            return m_files.at(id - 1);
        }

        void close_file(file_handle_id_t);

        // resolves sandbox path.
        std::string resolve_file_path(const std::string& path, bool write=false);

        // NEW FEATURE (signature: annika marie schlögel)
        // FUNCTION DECLARATIONS FOR FILE I/O
        std::string file_find_first(const std::string& pattern, int attributes);
        std::string file_find_next();
        void file_find_close();
        // NEW FEATURE END

        std::string get_included_directory()
        {
            return m_included_directory;
        }

        // MODIFIED FEATURE (signature: annika marie schlögel)
        // upgraded to work with the new normalize_native_path function, which handles all paths correctly
        void set_included_directory(const std::string& str)
        {
            m_included_directory = normalize_native_path(str);
        }
        // MODIFIED FEATURE END

        // NEW FEATURE (signature: annika marie schlögel)
        void set_project_name(const std::string& str)
        {
            std::cout << "Filesystem project name = " << str << '\n'; // DEBUG LINE (signature: annika marie schlögel)
            m_project_name = str;
        }

        const std::string& get_project_name() const
        {
            return m_project_name;
        }
        // NEW FEATURE END

    private:
        // checks for included files first, then checks other directories.
        bool file_is_included(const std::string& path);
    };
}}
