#include "manager.hpp"
#include "worker.hpp"

using namespace std;

void killKids(int id)
{
    cout << "Got here\n";
    delete[] directory;
    list<pid_t>::iterator childIt;
    list<worker *>::iterator workerIt;
    int status = 0;
    cout << "Before loop\n";
    for (childIt = childList.begin(); childIt != childList.end(); ++childIt)
    {
        kill(*childIt, SIGINT);
        if (waitpid(*childIt, &status, WNOHANG) != 0)
        {
            if (!WIFSIGNALED(status))
            {
                cout << "PID inside if is: " << *childIt << "\n";
                waitpid(*childIt, &status, 0);
            }
            cout << "Killed a kid\n";
        }
        cout << "PID of kill is: " << *childIt << "\n";
    }
    for (workerIt = workerList->begin(); workerIt != workerList->end(); ++workerIt)
    {
        delete *workerIt;
    }
    workerList->clear();
    delete workerList;
    exit(0);
}


void assignToWorker(char *token, list<worker *> *workerList, int *workerCounter, list<pid_t> *childList)
{
    int status;
    list<worker *>::iterator workerIt;
    worker *newWorker;
    int fd, i, nwrite;
    char msgbuf[MSGSIZE + 1];
    memset(msgbuf, 0, MSGSIZE + 1);
    strcpy(msgbuf, token);
    char fifo[256];
    strcpy(fifo, (fifoFolder + fifoNameBase).c_str());

    for (workerIt = workerList->begin(); workerIt != workerList->end(); ++workerIt)
    {
        if ((*workerIt)->busy == false)
        {
            // Give work
            strcat(fifo, to_string((*workerIt)->fifoId).c_str());
            kill((*workerIt)->id, SIGCONT);
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

    // If we get here, no worker is free
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
        workerMain(newWorker->fifoId);
        exit(EXIT_SUCCESS);
    }
    else
    {
        // Create a new worker and fifo for him to use
        cout << "New procc pid is: " << pid << "\n";
        workerList->back()->id = pid;
        childList->push_back(pid);

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
