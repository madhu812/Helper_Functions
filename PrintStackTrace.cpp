#include <iostream>
#include <fstream>
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>

void printStackTrace(const char* fileName) {
    constexpr int maxStackTraceSize = 100;
    void* stackTrace[maxStackTraceSize];

    // Obtain the backtrace
    int stackTraceSize = backtrace(stackTrace, maxStackTraceSize);

    // Resolve the function names and source code location
    Dl_info info;
    std::ofstream outfile(fileName); // Open the file for writing
    if (!outfile.is_open()) {
        std::cerr << "Failed to open file: " << fileName << std::endl;
        return;
    }

    for (int i = 0; i < stackTraceSize; ++i) {
        dladdr(stackTrace[i], &info);
        char* symbolName = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, nullptr);

        if (symbolName != nullptr) {
            // Demangle the symbol name
            outfile << "#" << i << ": " << symbolName;

            // Print the address offset within the symbol
            if (info.dli_saddr != nullptr) {
                unsigned long offset = static_cast<char*>(stackTrace[i]) - static_cast<char*>(info.dli_saddr);
                outfile << " + " << offset;
            }

            outfile << " [" << info.dli_fname << "]";

            // Print the source code file and line number
            if (info.dli_fname != nullptr && info.dli_fbase != nullptr) {
                std::ifstream file(info.dli_fname);
                if (file) {
                    int lineNum = 0;
                    std::string line;
                    while (std::getline(file, line)) {
                        ++lineNum;
                        if (line.find(symbolName) != std::string::npos) {
                            outfile << ":" << lineNum;
                            break;
                        }
                    }
                }
            }

            outfile << std::endl;

            free(symbolName);
        }
    }

    outfile.close(); // Close the file
}