#ifndef WORKER
#define WORKER
#include "utils.hpp"

void workerOutput(std::list<std::string> *domainList, std::string fileName);

void workerMain(int id);

#endif //WORKER