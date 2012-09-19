//  Copyright (c) 2007-2012 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/hpx_fwd.hpp>
#include <hpx/util/stringstream.hpp>

#include <boost/config.hpp>

#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>

#if defined(BOOST_WINDOWS)
#include <windows.h>

#elif __ APPLE__
#include <stdio.h>
#include <crt_externs.h>
#include <mach-o/dyld.h>
#define environ (*_NSGetEnviron())

#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__AIX__)
#include <unistd.h>
#include <sys/stat.h>
extern char **environ;

#endif

///////////////////////////////////////////////////////////////////////////////
#if defined(__APPLE__)
int get_executable_filename(char *exepath, uint32_t len)
{
  /* First appeared in Mac OS X 10.2 */
  return _NSGetExecutablePath(exepath, &len);
}  
  
#elif defined(linux) || defined(__linux) || defined(__linux__) 
bool linux_get_cmd_line (std::vector <std::string>& cmdline)
{
    std::string   elem;
    hpx::util::osstream procfile;
    
    procfile << "/proc/" << getpid () << "/cmdline";
    std::ifstream proc;
    proc.open (hpx::util::osstream_get_string(procfile), std::ios::binary);

    if (!proc.is_open())
        return false;       // proc fs does not exist on this machine
        
    while (!proc.eof ())
    {
        char c = proc.get();
        if (c == '\0')
        {
            cmdline.push_back(elem);
            elem.clear();
        }
        else
        {
            elem += c;
        }
    }
    
    if (!elem.empty())
        cmdline.push_back(elem);

    return true;
}

bool linux_get_args (std::vector <std::string>& args)
{
    std::vector <std::string> cmdline;
    if (!linux_get_cmd_line (cmdline))
        return false;

    if (cmdline.size() > 1) 
        std::copy(cmdline.begin()+1, cmdline.end(), std::back_inserter(args));

    return true;
}

bool linux_get_cmd (std::string& result)
{
    std::vector <std::string> cmdline; 
    if (linux_get_cmd_line(cmdline) && cmdline.size() > 0)
    {
        result = "./" + cmdline[0];
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
int get_executable_filename(char *exepath, int len, char* argv[])
{
    if (len <= 0)
        return 0;   // buffer is too small
        
    // check for /proc/self/exe which is an symbolic link to the exe
    // if ( 0 > readlink ("/proc/self/exe", exe, MAX_PATH) )
    exepath[0] = '\0';

    if ( argv == NULL )
      return (0); // no chance to find exec path

    if (true) {
    /* some more heuristics:
      *    if argv[0] starts with '/'
      *    then
      *       absolute path - done
      *    else if argv[0] contains '/'
      *       relative path - append to pwd - done
      *    else
      *       run through $PATH looking for the first entry
      *       that yields a file that exists when pre-pended
      *       to argv[0]
      *    fi
      */
        if (strlen (argv[0]) > 0) {
            if (argv[0][0] == '/') {                  // absolute path
              snprintf (exepath, len, "%s", argv[0]);
            }
            else if (NULL != strchr (argv[0], '/')) { // relative path
              char pwd[MAX_PATH + 1] = { '\0' };
              if (NULL != getcwd (pwd, MAX_PATH))
                  snprintf (exepath, len, "%s/%s", pwd, argv[0]);
            }
            else {
            // need to search $PATH 
                char* epath = getenv("PATH");     // FIXME: not thread-safe
                if (NULL != epath) {
                // get first path element 
                    char* path_elem = strtok (epath, ":");

                    // try each path element
                    while (NULL != path_elem) {
                        struct stat s;
                        snprintf (exepath, len, "%s/%s", path_elem, argv[0]);

                        if (0 == stat (exepath, &s)) {
                        // exe exists - check if it is executable 
                            if ((s.st_uid == geteuid() && (s.st_mode & S_IXUSR)) ||
                                (s.st_gid == getegid() && (s.st_mode & S_IXGRP )) ||
                                (                          s.st_mode & S_IXOTH ))
                            {
                                break;
                            }
                        }
                        // prepare next try
                        exepath[0] = '\0';
                        path_elem = strtok(NULL, ":");
                    }
                }
            }
        }
    }
    return strlen(exepath);
}
#endif  // apple/linux

///////////////////////////////////////////////////////////////////////////////
std::string get_executable_filename(int argc, char* argv[])
{
    std::string executable_path;

// initialize the path to the executable
    char exepath[MAX_PATH + 1] = { '\0' };
        
#if defined(BOOST_WINDOWS)
    GetModuleFileName(NULL, exepath, sizeof(exepath));
    executable_path = exepath;

#elif defined(linux) || defined(__linux) || defined(__linux__)
    if (!linux_get_cmd(executable_path)) 
    {
        if (0 == argv ||
            0 == get_executable_filename(exepath, sizeof(exepath), argv))
        {
            HPX_THROW_EXCEPTION(hpx::not_implemented,
                "Unable to extract executable path for this job "
                "on this platform.", "get_executable_filename");
        }
        executable_path = exepath;
    }

#elif defined(__APPLE__)
    get_executable_filename(exepath, sizeof(exepath));
    executable_path = exepath;
#else

#error "Don't know, how to access the executable path on this platform"
#endif
    return executable_path;
}
