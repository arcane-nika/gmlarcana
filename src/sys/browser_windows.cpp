#if defined(WIN32) || defined(__WIN32__) || defined(_WIN32)

#include "ogm/sys/util_sys.hpp"
#include "fs_share.hpp"
#include <iostream>

// BUG FIX (signature:annika marie schlögel)
// changed Windows.h to windows.h as cross-compiling w/ Fedora 44, MINGW only works case-sensitive and the MINGW header is called windows.h
#include <windows.h>


namespace ogm
{
    
void browser_open_url(const char* url, const char* target, const char* opts)
{
    std::string url_encoded = encode_url(url);
    ShellExecute(NULL, "open", url_encoded.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

std::string browser_get_url() { return ""; }
std::string browser_get_domain() { return ""; }

}

#endif