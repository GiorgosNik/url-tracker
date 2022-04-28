#include "utils.hpp"
#include "worker.hpp"
#include "manager.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    string pathArgument = "-p";
    workerList = new list<worker *>;
    int workerCounter = 0;
    char *token, *saveptr;
    char inbuf[MSGSIZE];
    char outbuf[MSGSIZE];
    int listenerPipe[2], i = 0, size = 0;
    pid_t pid;

    // Clear the buffers we will use
    memset(inbuf, 0, MSGSIZE);
    memset(outbuf, 0, MSGSIZE);

    // Set directory to monitor, based on user input
    // Default to the current directory if no arguments are given
    if (argc == 1)
    {   
        string defaultDir = "./";
        directory = new char[256];
        memset(directory, 0, 256);
        strcpy(directory, defaultDir.c_str());
    }
    else if (argc == 3)
    {   // If the user has provied arguments, check the format is valid
        if (strcmp(argv[1], pathArgument.c_str()) != 0)
        {
            cout << "Usage: ./sniffer [-p path]\n";
            exit(23);
        }
        //If it is, set the directory
        directory = new char[strlen(argv[2]) + 100];
        memset(directory, 0, strlen(argv[2]) + 100);
        strcpy(directory, argv[2]);
    }
    else
    {   
        // If the user provided bad input, give usage instructions
        cout << "Usage: ./sniffer [-p path]\n";
        exit(23);
    }

    // Add the "/" to the end of the directory if not present
    fixDir(directory);

    // Delete previous output directory
    if(system(("rm -rf "+outputFolder).c_str()) ==-1 ){
        perror("Unable to delete output directory");
        exit(1);
    }

    // Create new output directory
    if (mkdir(outputFolder.c_str(), 0777) < 0)
    {
        perror("Unable to create output directory");
        exit(1);
    }

    // Create Pipe for lustener
    if (pipe(listenerPipe) == -1)
    {
        perror(" pipe call ");
        exit(1);
    }

    // Create the listener
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return -1;
    }
    else if (pid == 0)
    {
        // Listener Code
        // Set the write end of the pipe to get the output of inotifywait
        close(listenerPipe[0]);
        dup2(listenerPipe[1], STDOUT_FILENO);
        close(listenerPipe[1]);

        // Monitor Changes
        execl("/usr/bin/inotifywait", "/usr/bin/inotifywait", "-m", directory, "-e", "create", NULL);
    }

    // Manager Code from here on
    // Set the singal handling functions
    signal(SIGCHLD, procStop);
    signal(SIGINT, killKids);

    // Set the write end of the listener pipe
    close(listenerPipe[1]);

    // Add the Listener to the list of spawned processes
    childList.push_back(pid);

    // Main Manager loop
    while (1)
    {
        if (read(listenerPipe[0], inbuf, MSGSIZE) < 0)
        {
            perror(" Read from pipe ");
            exit(1);
        }

        token = strtok_r(inbuf, "\n", &saveptr);
        while (token != NULL)
        {
            strcpy(outbuf, getFileName(token));
            assignToWorker(outbuf, workerList, &workerCounter, &childList);
            token = strtok_r(NULL, "\n", &saveptr);
        }

        // Clear the input buffer
        memset(inbuf,0 ,MSGSIZE);
    }

    return 0;
}
