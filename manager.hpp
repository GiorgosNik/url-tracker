#ifndef MANAGER
#define MANAGER
#include "utils.hpp"

void killKids(int id);

void assignToWorker(char *token, std::list<worker *> *workerList, int *workerCounter, std::list<pid_t> *childList);

#endif //MANAGER