#ifndef UTILS
#define UTILS
#include <iostream>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <iterator>
#include <list>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <filesystem>

typedef struct worker
{
    pid_t id;
    int fifoId;
    bool busy;
} worker;

typedef struct site
{
    int counter;
    std::string domain;
} site;


#endif