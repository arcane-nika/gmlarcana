#pragma once

#include "external.h"
#include "ogm/common/util.hpp"
#include "ogm/sys/util_sys.hpp"

namespace ogm::interpreter::ffi
{
    struct SharedLibraryType
    {
        // BUG FIX (signature: annika marie schlögel)

        // LEGACY CODE
        /*enum {
            ERROR,
            WINDOWS,
            UNIX,
            APPLE,
        } os = ERROR;
         
        enum {
            UNKNOWN,
            x86,
            x64
        } arch = UNKNOWN;*/

        // NEW CODE
        enum {
            OS_ERROR,
            OS_WINDOWS,
            OS_UNIX,
            OS_APPLE,
        } os = OS_ERROR;

        enum {
            UNKNOWN,
            ARCH_x86,
            ARCH_x64
        } arch = UNKNOWN;

        // BUG FIX END
    
        bool archmatch() const
        {
            // changed x86, x64 TO ARCH_x86, ARCH_x64 according to the above fix
            return (arch == ARCH_x86 && is_32_bit()) || (arch == ARCH_x64 && is_64_bit());
        }
        
        bool osmatch() const
        {
            #if defined(__unix__)
            return os == OS_UNIX; // changed UNIX TO OS_UNIX according to the above fix
            #endif
            
            #if defined(_WIN32) || defined(WIN32)
            return os == OS_WINDOWS; // same here
            #endif
            
            #if defined(__APPLE__)
            return os == OS_APPLE; // same here
            #endif
            
            return false;
        }
        
        bool platmatch() const
        {
            /*std::cout << "platmatch: arch=" << arch
              << " os=" << os
              << " is32=" << is_32_bit()
              << " is64=" << is_64_bit()
              << "\n";*/

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
            //std::cout << "compatible() entered\n";

            bool pm = platmatch();
            //std::cout << "AFTER platmatch: " << pm << "\n";

            if (pm)
            {
                //std::cout << "platmatch true\n";
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
