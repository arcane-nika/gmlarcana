#pragma once

#include "external.h"
#include "ogm/common/util.hpp"
#include "ogm/sys/util_sys.hpp"

namespace ogm::interpreter::ffi
{
    struct SharedLibraryType
    {
        enum {
            ERROR,
            WINDOWS,
            UNIX,
            APPLE,
        } os = ERROR;
        
        enum {
            UNKNOWN,
            x86,
            x64
        } arch = UNKNOWN;
    
        bool archmatch() const
        {
            return (arch == x86 && is_32_bit()) || (arch == x64 && is_64_bit());
        }
        
        bool osmatch() const
        {
            #if defined(__unix__)
            return os == UNIX;
            #endif
            
            #if defined(_WIN32) || defined(WIN32)
            return os == WINDOWS;
            #endif
            
            #if defined(__APPLE__)
            return os == APPLE;
            #endif
            
            return false;
        }
        
        bool platmatch() const
        {
            return archmatch() && osmatch();
        }
        
        // PROBABLE BUGFIX (signature: annika marie schlögel)

        // LEGACY CODE
        /*bool compatible() const
        {
            if (platmatch()) return true;
            
            #if defined(EMBED_ZUGBRUECKE) && defined(PYTHON)
            if (os == WINDOWS && zugbruecke_init())
            {
                return true;
            }
            #endif
            
            return false;
        }*/

        // NEW CODE
        bool compatible() const
        {
            std::cout << "compatible() entered\n";

            if (platmatch())
            {
                std::cout << "platmatch true\n";
                return true;
            }

            #if defined(EMBED_ZUGBRUECKE) && defined(PYTHON)
            std::cout << "Trying Zugbruecke...\n";

            if (os == WINDOWS)
            {
                bool ok = zugbruecke_init();
                std::cout << "zugbruecke_init returned " << ok << '\n';
                if (ok)
                    return true;
            }
            #else
            std::cout << "EMBED_ZUGBRUECKE/PYTHON not defined\n";
            #endif

            return false;
        }
        // PROBABLE BUGFIX END
    };
    
    SharedLibraryType getSharedLibraryTypeFromPath(const std::string& path);
    
    void path_transform(std::string& path);
}
