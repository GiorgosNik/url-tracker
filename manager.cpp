#include "manager.hpp"
#include "worker.hpp"

using namespace std;

void killKids(int id)
{
    // SIGINT handler
    delete[] directory;
    list<pid_t>::iterator childIt;
    list<worker *>::iterator workerIt;
    int status = 0;

    // Loop through every child of the manager
    for (childIt = childList.begin(); childIt != childList.end(); ++childIt)
    {
        // Send SIGINT signal, wait for status
        kill(*childIt, SIGINT);
        if (waitpid(*childIt, &status, WNOHANG) != 0)
        {
            if (!WIFSIGNALED(status))
            {
                waitpid(*childIt, &status, 0);
            }
        }
    }

    // Delete the "worker" structs in the list to free up space
    for (workerIt = workerList->begin(); workerIt != workerList->end(); ++workerIt)
    {
        delete *workerIt;
    }
    workerList->clear();
    delete workerList;
    exit(0);
}

void procStop(int id)
{
    // SIGCHLD handler
    int status;
    list<worker *>::iterator workerIt;
    int pid = waitpid(-1, &status, WUNTRACED | WCONTINUED);

    // Wait for the child that changed status
    if (WIFSTOPPED(status))
    {
        // If it is a worker that stopped, change its status to free
        for (workerIt = workerList->begin(); workerIt != workerList->end(); ++workerIt)
        {
            if ((*workerIt)->id == pid)
            {
                (*workerIt)->busy = false;
            }
        }
    }

    return;
}

void assignToWorker(char *token, list<worker *> *workerList, int *workerCounter, list<pid_t> *childList)
{
    // Given the name of a new file in the monitored directory, assign it to a worker
    int status;
    list<worker *>::iterator workerIt;
    worker *newWorker;
    int fd, i, nwrite;
    char msgbuf[MSGSIZE + 1];
    memset(msgbuf, 0, MSGSIZE + 1);
    strcpy(msgbuf, token);
    char fifo[256];
    strcpy(fifo, fifoNameBase.c_str());

    // First, search for free workers 
    for (workerIt = workerList->begin(); workerIt != workerList->end(); ++workerIt)
    {
        if ((*workerIt)->busy == false)
        {
            // If a free worker is found, assign the file
            strcat(fifo, to_string((*workerIt)->fifoId).c_str());
            
            // Raise the worker
            kill((*workerIt)->id, SIGCONT);

            // Write the filename to the named pipe of the worker
            fd = open(fifo, O_WRONLY);
            if (write(fd, msgbuf, MSGSIZE + 1) < 0)
            {
                perror(" Problem with writing ");
                exit(5);
            }
            close(fd);
            return;
        }
    }

    // If we get here, no worker is free, must create a new one
    // First, add the information of the new worker to the list
    (*workerCounter) = (*workerCounter) + 1;
    newWorker = new worker;
    newWorker->busy = true;
    newWorker->fifoId = (*workerCounter);
    workerList->push_back(newWorker);
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
    }
    else if (pid == 0)
    {
        // This is the new worker, work on the assigned file
        workerMain(newWorker->fifoId);
        exit(EXIT_SUCCESS);
    }
    else
    {
        // This is the manager, add the pid of the worker to the relevant lists
        workerList->back()->id = pid;
        childList->push_back(pid);

        // Create a fifo for the new worker and write the name of the file for the worker to proccess
        strcat(fifo, to_string((*workerCounter)).c_str());
        mkfifo(fifo, 0666);
        fd = open(fifo, O_WRONLY);
        if (write(fd, msgbuf, MSGSIZE + 1) < 0)
        {
            perror(" Problem with writing ");
            exit(5);
        }
        close(fd);
    }
}
