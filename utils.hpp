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
#define MSGSIZE 1000
#define PERMS 0644

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

std::string link_remove_www(std::string link);

std::string link_remove_space(std::string link);

std::string link_getLocation(std::string link);

bool checkLink(std::string link);

void fixDir(char *givenPath);

bool cleanBuffer(char *buffer, int size);

bool isEmpty(char *buffer, int size);

char *getFileName(char *token);


extern std::string fifoNameBase;
extern std::string outputFolder;
extern std::string fifoFolder;
extern std::list<pid_t> childList;
extern std::list<std::string> ignoreList;
extern std::list<worker *> *workerList;
extern char *directory;

#endif // UTILS