#include "filter.hpp"

#include <stdlib.h>

#include <iostream>
#include <stdexcept>

inum_filter::inum_filter(const char* arg) {
    try {
        ino = (ino_t)std::stoll(arg, nullptr, 0);
    } catch (std::exception& e) {
        throw std::runtime_error(std::string("bad -inum argument: ") + e.what());
    }
}

bool inum_filter::apply(const std::string& path, const linux_dirent* de, const struct stat* st) const {
    return ino == st->st_ino;
}

name_filter::name_filter(const char* arg): name(arg) {}

bool name_filter::apply(const std::string& path, const linux_dirent* de, const struct stat* st) const {
    return name == de->d_name;
}

size_filter::size_filter(const char* arg) {
    switch (*arg) {
        case '+': 
            mod = modifier::GREATER;
            break;
        case '-':
            mod = modifier::LESS;
            break;
        case '=':
            mod = modifier::EQUAL;
            break;
        default:
            throw std::runtime_error("bad -size argument: should start with [-=+]");
    }
    
    try {
        size = (size_t)std::stoll(arg + 1, nullptr, 0);
    } catch (std::exception& e) {
        throw std::runtime_error(std::string("bad -size argument: ") + e.what());
    }
}

bool size_filter::apply(const std::string& path, const linux_dirent* de, const struct stat* st) const {
    switch (mod) {
        case modifier::GREATER:
            return st->st_size > size;
        case modifier::EQUAL:
            return st->st_size == size;
        case modifier::LESS:
            return st->st_size < size;
        default:
            throw std::runtime_error("program got in inconsistent state: size modifier is unknown");
    }
}

nlinks_filter::nlinks_filter(const char* arg) {
    try {
        nlink = (nlink_t)std::stoll(arg, nullptr, 0);
    } catch (std::exception& e) {
        throw std::runtime_error(std::string("bad -nlinks argument: ") + e.what());
    }
}

bool nlinks_filter::apply(const std::string& path, const linux_dirent* de, const struct stat* st) const {
    return nlink == st->st_nlink;
}

exec_filter::exec_filter(const char* arg): program(arg) {}

bool exec_filter::apply(const std::string& path, const linux_dirent* de, const struct stat* st) const {
    return system((program + " " + path).c_str()) == 0;
}

filter_chain::filter_chain(char** argv) {
    has_exec = false;
    
    while (*argv != nullptr) {
        std::string arg = *argv;
        
        ++argv;
        if (*argv == nullptr) {
            throw std::runtime_error(std::string("no value supplied for argument ") + arg);
        }
        
        if (arg == "-inum") {
            filters.push_back(new inum_filter(*argv));
        } else if (arg == "-name") {
            filters.push_back(new name_filter(*argv));
        } else if (arg == "-size") {
            filters.push_back(new size_filter(*argv));
        } else if (arg == "-nlinks") {
            filters.push_back(new nlinks_filter(*argv));
        } else if (arg == "-exec") {
            has_exec = true;
            filters.push_back(new exec_filter(*argv));
        } else {
            throw std::runtime_error(std::string("unexpected argument: ") + arg);
        }
        ++argv;
    }
}

filter_chain::~filter_chain() {
    for (auto& element: filters) {
        delete element;
    }
}

bool filter_chain::apply(const std::string& path, const linux_dirent* de, const struct stat* st) const {
    for (auto& element: filters)
        if (!element->apply(path, de, st))
            return false;
    
    if (!has_exec)
        std::cout << path << '\n';
    
    return true;
}
