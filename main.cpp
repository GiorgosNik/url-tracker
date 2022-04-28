#include "utils.hpp"
#include "worker.hpp"
#include "manager.hpp"

using namespace std;

int main(int argc, char *argv[])
{

    char pathDirective[10];
    string pathArg = "-p";
    strcpy(pathDirective, pathArg.c_str());
    workerList = new list<worker *>;

    // Set working dir
    if (argc == 1)
    {
        string defaultDir = "./";
        directory = new char[256];
        memset(directory, 0, 256);
        strcpy(directory, defaultDir.c_str());
    }
    else if (argc == 3)
    {
        if (strcmp(argv[1], pathDirective) != 0)
        {
            cout << "Usage: ./sniffer [-p path]\n";
            exit(23);
        }
        directory = new char[strlen(argv[2]) + 100];
        memset(directory, 0, strlen(argv[2]) + 100);
        strcpy(directory, argv[2]);
    }
    else
    {
        cout << "Usage: ./sniffer [-p path]\n";
        exit(23);
    }

    fixDir(directory);
    cout << "Directory is: " << directory << "\n";

    // Delete previous output directory
    if(system(("rm -rf "+outputFolder).c_str()) ==-1 ){
        cout << "Unable to delete output directory\n";
        exit(1);
    }

    // Create new output directory
    if (mkdir(outputFolder.c_str(), 0777) < 0)
    {
        cout << "Unable to create output directory\n";
        exit(1);
    }

    // Delete previous output directory
    if(system(("rm -rf "+fifoFolder).c_str()) ==-1 ){
        cout << "Unable to delete fifo directory\n";
        exit(1);
    }
    // Create new fifo directory
    if (mkdir(fifoFolder.c_str(), 0777) < 0)
    {
        cout << "Unable to create fifo directory\n";
        exit(1);
    }

    int workerCounter = 0;
    char inbuf[MSGSIZE];
    memset(inbuf, 0, MSGSIZE);
    char outbuf[MSGSIZE];
    memset(outbuf, 0, MSGSIZE);
    char *token;
    char *saveptr1;

    list<pid_t>::iterator childIt;
    list<worker>::iterator workerIt;

    pid_t endingPID;

    // Create Pipe
    int listenerPipe[2], i = 0, size = 0;

    if (pipe(listenerPipe) == -1)
    {
        perror(" pipe call ");
        exit(1);
    }

    // Create the listener
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return -1;
    }
    else if (pid == 0) // I am the listener
    {
        cout << "Listener PID: " << getpid() << "\n";
        close(listenerPipe[0]);
        dup2(listenerPipe[1], STDOUT_FILENO);
        close(listenerPipe[1]);

        // Monitor Changes
        execl("/usr/bin/inotifywait", "/usr/bin/inotifywait", "-m", directory, "-e", "create", NULL);
    }
    signal(SIGCHLD, procStop);
    signal(SIGINT, killKids);
    close(listenerPipe[1]);
    childList.push_back(pid);
    while (1)
    {
        close(listenerPipe[1]);
        if (read(listenerPipe[0], inbuf, MSGSIZE) < 0)
        {
            perror(" Read from pipe ");
            exit(1);
        }

        token = strtok_r(inbuf, "\n", &saveptr1);
        while (token != NULL)
        {
            strcpy(outbuf, getFileName(token));
            assignToWorker(outbuf, workerList, &workerCounter, &childList);
            token = strtok_r(NULL, "\n", &saveptr1);
        }

        // Clear the input buffer
        cleanBuffer(inbuf, MSGSIZE);
    }

    return 0;
}
