#ifndef FIND_FILTER
#define FIND_FILTER

// #include <sys/types.h>
#include <sys/stat.h>

#include <string>
#include <vector>

#include "linux_dirent.h"

class filter {
public:
    virtual ~filter() = default;
    virtual bool apply(const std::string&, const linux_dirent*, const struct stat*) const = 0;
};

class inum_filter: public filter {
private:
    unsigned long long ino;
public:
    inum_filter(const char*);
    bool apply(const std::string&, const linux_dirent*, const struct stat*) const override;
};

class name_filter: public filter {
private:
    std::string name;
public:
    name_filter(const char*);
    bool apply(const std::string&, const linux_dirent*, const struct stat*) const override;
};

class size_filter: public filter {
private:
    enum class modifier {
        LESS, EQUAL, GREATER
    };
    
    off_t size;
    modifier mod;
public:
    size_filter(const char*);
    bool apply(const std::string&, const linux_dirent*, const struct stat*) const override;
};

class nlinks_filter: public filter {
private:
    nlink_t nlink;
public:
    nlinks_filter(const char*);
    bool apply(const std::string&, const linux_dirent*, const struct stat*) const override;
};

class exec_filter: public filter {
private:
    std::string program;
public:
    exec_filter(const char*);
    bool apply(const std::string&, const linux_dirent*, const struct stat*) const override;
};

class filter_chain {
private:
    bool has_exec;
    
    std::vector<filter*> filters;
public:
    filter_chain(char**);
    ~filter_chain();
    
    bool apply(const std::string&, const linux_dirent*, const struct stat*) const;
};

#endif
