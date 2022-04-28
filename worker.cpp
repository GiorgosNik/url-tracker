#include "worker.hpp"

using namespace std;

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