#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
// #include <sys/types.h>
#include <sys/syscall.h>

#include "linux_dirent.h"

#include <iostream>
#include <vector>

#include "filter.hpp"


void print_error(const char* reason) {
    std::cerr << "find: " << reason;
    if (errno) {
        std::cerr << ": " << strerror(errno);
    }
    std::cerr << std::endl;
}


void process_file(const std::string& path, int dirfd, linux_dirent* de, const filter_chain& filters) {
    struct stat st;
    int status = fstatat(dirfd, de->d_name, &st, AT_SYMLINK_NOFOLLOW);
    if (status == -1) {
        print_error("can't stat file");
    }
    
    filters.apply(path, de, &st);
}


void process_dir(const std::string& path, int fd, const filter_chain& filters) {
    long read;
    char buf[DIRENT_SIZE];
    do {
        read = syscall(SYS_getdents, fd, buf, DIRENT_SIZE);
        
        if (read == -1) {
            print_error("can't read directory");
        }
        
        for (long pos = 0; pos < read;) {
            linux_dirent *de = (linux_dirent*)(buf + pos);
            char d_type = *(buf + pos + de->d_reclen - 1);
            pos += de->d_reclen;
            
            std::string name(de->d_name);
            if (name == "..") {
                continue;
            }
            if (name == ".") {
                process_file(path, fd, de, filters);
                continue;
            }
            
            std::string filepath = path + "/" + name;
            
            if (d_type == DT_DIR) {
                int childfd = openat(fd, name.c_str(), O_RDONLY | O_DIRECTORY);
                
                if (childfd == -1) {
                    print_error("can't open directory");
                    continue;
                }
                
                process_dir(filepath, childfd, filters);
            } else {
                process_file(filepath, fd, de, filters);
            }
        }
    } while (read != 0);
    
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        print_error("no directory to search in");
        return EXIT_FAILURE;
    }
    
    ++argv;
    std::string path(*argv);
    
    try {
        ++argv;
        filter_chain filters(argv);
        
        int fd = open(path.c_str(), O_RDONLY | O_DIRECTORY);
    
        if (fd == -1) {
            print_error("can't open directory");
            return EXIT_FAILURE;
        }
        
        process_dir(path, fd, filters);
    } catch (std::exception& e) {
        print_error(e.what());
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}