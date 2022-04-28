#include "../include/utils.hpp"
#define MSGSIZE 1000
#define PERMS 0644
using namespace std;

string fifoNameBase = "fifo";
string outputFolder = "/tmp/snifferOut/";
string fifoFolder = "/tmp/fifo/";
list<pid_t> childList;
list<string> ignoreList;
list<worker *> *workerList;
char *directory;
using namespace std;
string link_remove_www(string link)
{
    string linkOptional = "www.";
    size_t spotOptional = 0;
    if ((spotOptional = link.find(linkOptional)) != string::npos)
    {
        link.erase(0, spotOptional + linkOptional.length());
    }
    return link;
}

string link_remove_space(string link)
{
    string linkEnd = " ";
    size_t spotEnd = 0;
    if ((spotEnd = link.find(linkEnd)) != string::npos)
    {
        link = link.substr(0, spotEnd);
    }
    return link;
}

string link_getLocation(string link)
{
    string linkEnd = ":";
    size_t spotEnd = 0;
    if ((spotEnd = link.find(linkEnd)) != string::npos)
    {
        link = link.substr(0, spotEnd);
    }
    linkEnd = "/";
    spotEnd = 0;
    if ((spotEnd = link.find(linkEnd)) != string::npos)
    {
        link = link.substr(0, spotEnd);
    }
    linkEnd = "?";
    spotEnd = 0;
    if ((spotEnd = link.find(linkEnd)) != string::npos)
    {
        link = link.substr(0, spotEnd);
    }
    linkEnd = "#";
    spotEnd = 0;
    if ((spotEnd = link.find(linkEnd)) != string::npos)
    {
        link = link.substr(0, spotEnd);
    }
    return link;
}

void procStop(int id)
{
    int status;
    list<worker *>::iterator workerIt;
    int pid = waitpid(-1, &status, WUNTRACED | WCONTINUED);
    cout << "Pid is: " << pid << " status is " << status << "\n";
    if (WIFSTOPPED(status))
    {
        for (workerIt = workerList->begin(); workerIt != workerList->end(); ++workerIt)
        {
            if ((*workerIt)->id == pid)
            {
                cout << "Reset worker\n";
                (*workerIt)->busy = false;
            }
        }
    }

    return;
}

bool checkLink(string link)
{
    string linkDomain = link;
    string domainDel = ".";
    size_t domainSpot = 0;
    char *domainArray = new char[7];
    while ((domainSpot = linkDomain.find(domainDel)) != string::npos)
    {
        linkDomain.erase(0, domainSpot + domainDel.length());
    }
    if (linkDomain.size() >= 2 && linkDomain.size() <= 6)
    {
        strcpy(domainArray, linkDomain.c_str());
        for (int i = 0; i < linkDomain.size(); i++)
        {
            if (domainArray[i] < 'a' || domainArray[i] > 'z')
            {
                cout << link << " BAD CHAR\n";
                return false;
            }
        }
        return true;
    }
    else
    {
        cout << link << " DOMAIN TO SMALL\n";
        return false;
    }
}

void fixDir(char *givenPath)
{
    if (strcmp(&givenPath[strlen(givenPath) - 1], "/") != 0)
    {
        givenPath[strlen(givenPath)] = '/';
        givenPath[strlen(givenPath) + 1] = '\n';
    }
}

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

bool cleanBuffer(char *buffer, int size)
{
    bool flag = false;
    for (int i = 0; i < size; i++)
    {
        if (buffer[i] != (char)0)
        {
            flag = true;
        }
        buffer[i] = (char)0;
    }
    return flag;
};

bool isEmpty(char *buffer, int size)
{
    bool flag = true;
    for (int i = 0; i < size; i++)
    {
        if (buffer[i] != (char)0)
        {
            flag = false;
        }
    }
    return flag;
};

char *getFileName(char *token)
{
    strtok(token, " ");
    strtok(NULL, " ");
    token = strtok(NULL, " ");
    return token;
};

void workerOutput(list<string> *domainList, string fileName)
{
    list<site *> siteCounter;
    list<site *>::iterator siteIt;
    list<string>::iterator domainIt;
    site *newSite;
    string fileNameOut = outputFolder + fileName + ".out";
    string toWrite = "";
    int outFile;
    bool flag;

    if (domainList->size() == 0)
    {
        cout << "Gets here\n";
        outFile = open(fileNameOut.c_str(), O_CREAT | O_RDWR | O_TRUNC, PERMS);
        if (outFile == -1)
        {
            perror(" Creating .out file ");
            exit(1);
        }
        close(outFile);

        return;
    }
    for (domainIt = domainList->begin(); domainIt != domainList->end(); ++domainIt)
    {
        flag = false;
        for (siteIt = siteCounter.begin(); siteIt != siteCounter.end(); ++siteIt)
        {
            if ((*siteIt)->domain.compare(*domainIt) == 0)
            {
                (*siteIt)->counter++;
                flag = true;
                break;
            }
        }
        if (flag == false)
        {
            newSite = new site;
            newSite->counter = 1;
            newSite->domain = *domainIt;
            siteCounter.push_front(newSite);
        }
    }

    for (siteIt = siteCounter.begin(); siteIt != siteCounter.end(); ++siteIt)
    {
        toWrite += (*siteIt)->domain + " " + to_string((*siteIt)->counter) + "\n";
        cout << "TO WRITE " << toWrite << "\n";
    }
    cout << toWrite;

    outFile = open(fileNameOut.c_str(), O_CREAT | O_RDWR | O_TRUNC, PERMS);
    if (outFile == -1)
    {
        perror(" Creating .out file ");
        exit(1);
    }
    if (write(outFile, toWrite.c_str(), toWrite.size()) < 0)
    {
        perror(" Problem with writing ");
        exit(5);
    }
    for (siteIt = siteCounter.begin(); siteIt != siteCounter.end(); ++siteIt)
    {
        delete *siteIt;
    }
    close(outFile);
}

void workerMain(int id)
{
    char msgbuf[MSGSIZE + 1];
    char localDir[1000 + MSGSIZE + 1];
    int fd, i, textFile, readReturn;
    char fifo[256];
    char *saveptr1;
    strcpy(fifo, (fifoFolder + fifoNameBase).c_str());
    strcat(fifo, to_string(id).c_str());
    string text = "";
    string fileName;
    string linkStart = "http://";
    string link;
    size_t spotStart = 0;
    list<string> domainList;
    bool flag;
    while (1)
    {
        flag = false;
        // Read the filename from the fifo
        fd = open(fifo, O_RDONLY);
        if (read(fd, msgbuf, MSGSIZE) < 0)
        {
            perror(" Error in reading ");
            exit(5);
        }
        close(fd);

        fileName = string(msgbuf);
        // Combine the filename and the path
        strcpy(localDir, directory);
        strcat(localDir, msgbuf);

        // Open the file, read every line and combine in a string
        cleanBuffer(msgbuf, MSGSIZE + 1);

        textFile = open(localDir, O_RDONLY);
        readReturn = read(textFile, msgbuf, 1);
        while (readReturn > 0)
        {
            text = text + string(msgbuf);
            readReturn = read(textFile, msgbuf, 1);
        }
        if (readReturn < 0)
        {
            perror(" Error in reading ");
            exit(5);
        }
        close(textFile);

        // Remove text before first link
        if ((spotStart = text.find(linkStart)) != string::npos)
        {
            link = text.substr(0, spotStart);
            text.erase(0, spotStart + linkStart.length());
        }

        // Process links in the midle
        while ((spotStart = text.find(linkStart)) != string::npos)
        {
            flag = true;
            cout << "IN\n";
            link = text.substr(0, spotStart);
            link = link_remove_www(link);
            link = link_remove_space(link);
            link = link_getLocation(link);
            domainList.push_front(link);

            text.erase(0, spotStart + linkStart.length());
        }

        // Process last link
        if (flag)
        {
            link = text;
            link = link_remove_www(link);
            link = link_remove_space(link);
            link = link_getLocation(link);
            domainList.push_front(link);
        }
        workerOutput(&domainList, fileName);
        raise(SIGSTOP);
        domainList.clear();
        cout << "Continue\n";
    }

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
};

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
    std::filesystem::remove_all(outputFolder);
    // Create new output directory
    if (mkdir(outputFolder.c_str(), 0777) < 0)
    {
        cout << "Unable to create output directory\n";
        exit(1);
    }

    // Delete previous fifo directory
    std::filesystem::remove_all(fifoFolder);
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
