#include "cppcheck.h"
#include "filelister.h"
#include "path.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

class CppcheckExecutor : public ErrorLogger {
private:
    CppCheck cppcheck;

public:
    CppcheckExecutor(const Settings &settings)
        : ErrorLogger()
        , cppcheck(*this,false) {
        cppcheck.settings() = settings;
    }

    void check(const std::string &filename) {
        cppcheck.check(filename);
    }

    void reportOut(const std::string &outmsg) {
        std::cout << outmsg << std::endl;
    }

    void reportErr(const ErrorLogger::ErrorMessage &msg) {
        std::cerr << msg.toString(false) << std::endl;
    }
};


int main(int argc, char **argv)
{
    std::cout << "ublinter" << std::endl;

    Settings settings;
    std::map<std::string, std::size_t> files;
    for (int i = 1; i < argc; i++) {
        if (std::strncmp(argv[i], "-D", 2) == 0) {
            std::string define;

            // "-D define"
            if (std::strcmp(argv[i], "-D") == 0) {
                ++i;
                if (i >= argc || argv[i][0] == '-') {
                    std::cerr << "cppcheck: argument to '-D' is missing." << std::endl;
                    return EXIT_FAILURE;
                }

                define = argv[i];
            }
            // "-Ddefine"
            else {
                define = 2 + argv[i];
            }

            // No "=", append a "=1"
            if (define.find("=") == std::string::npos)
                define += "=1";

            // DEF= => empty define
            else if (define.find("=") + 1U == define.size())
                define.erase(define.size() - 1U);

            if (!settings.userDefines.empty())
                settings.userDefines += ";";
            settings.userDefines += define;
        }

        // Include paths
        else if (std::strncmp(argv[i], "-I", 2) == 0) {
            std::string path;

            // "-I path/"
            if (std::strcmp(argv[i], "-I") == 0) {
                ++i;
                if (i >= argc || argv[i][0] == '-') {
                    std::cerr << "cppcheck: argument to '-I' is missing." << std::endl;
                    return false;
                }
                path = argv[i];
            }

            // "-Ipath/"
            else {
                path = 2 + argv[i];
            }
            path = Path::fromNativeSeparators(path);
            path = Path::removeQuotationMarks(path);

            // If path doesn't end with / or \, add it
            if (path[path.length()-1] != '/')
                path += '/';

            settings._includePaths.push_back(path);
        }

        else {
            std::string path = Path::fromNativeSeparators(argv[i]);
            path = Path::removeQuotationMarks(path);
            FileLister::recursiveAddFiles(files, Path::toNativeSeparators(path));
        }
    }

    CppcheckExecutor cppcheckExecutor(settings);
    for (std::map<std::string,std::size_t>::const_iterator it = files.begin(); it != files.end(); ++it) {
        cppcheckExecutor.check(it->first);
    }

    return EXIT_SUCCESS;
}
