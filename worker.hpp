#ifndef WORKER
#define WORKER
#include "utils.hpp"

void procStop(int id);

void workerOutput(std::list<std::string> *domainList, std::string fileName);

void workerMain(int id);

#endif //WORKER