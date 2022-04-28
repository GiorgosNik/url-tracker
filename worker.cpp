#include "worker.hpp"

using namespace std;


void workerOutput(list<string> *siteList, string fileName)
{
    // Given a list of locations, count their number of appearances and print to output file
    list<site *> siteCounter;
    list<site *>::iterator siteIt;
    list<string>::iterator domainIt;
    site *newSite;
    string fileNameOut = outputFolder + fileName + ".out";
    string toWrite = "";
    int outFile;
    bool flag;

    // If the list is empty, create empty file
    if (siteList->size() == 0)
    {
        outFile = open(fileNameOut.c_str(), O_CREAT | O_RDWR | O_TRUNC, PERMS);
        if (outFile == -1)
        {
            perror(" Creating .out file ");
            exit(1);
        }
        close(outFile);

        return;
    }

    // Loop for every site in the list
    for (domainIt = siteList->begin(); domainIt != siteList->end(); ++domainIt)
    {
        flag = false;
        // Check if we have found it at least once before
        for (siteIt = siteCounter.begin(); siteIt != siteCounter.end(); ++siteIt)
        {
            // If we have, increase the appearances count by one
            if ((*siteIt)->domain.compare(*domainIt) == 0)
            {
                (*siteIt)->counter++;
                flag = true;
                break;
            }
        }
        // Else, add the site to a list, containing the name of the site plus the number of appearances
        if (flag == false)
        {
            newSite = new site;
            newSite->counter = 1;
            newSite->domain = *domainIt;
            siteCounter.push_front(newSite);
        }
    }

    // For every site, write to the output file its name and number of appearances, using the list we created
    // Create a big string, write to the file in one go
    for (siteIt = siteCounter.begin(); siteIt != siteCounter.end(); ++siteIt)
    {
        toWrite += (*siteIt)->domain + " " + to_string((*siteIt)->counter) + "\n";
    }
    
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
    strcpy(fifo, fifoNameBase.c_str());
    strcat(fifo, to_string(id).c_str());
    string text = "";
    string fileName;
    string linkStart = "http://";
    string link;
    size_t spotStart = 0;
    list<string> siteList;
    bool flag;
    while (1)
    {
        flag = false;
        // Read the filename from the fifo
        fd = open(fifo, O_RDONLY);
        if (read(fd, msgbuf, MSGSIZE) < 0)
        {
            perror(" Error in reading filename from fifo");
            exit(5);
        }
        close(fd);
        fileName = string(msgbuf);
        

        // Combine the filename and the path
        strcpy(localDir, directory);
        strcat(localDir, msgbuf);

        // Open the file, read every line and combine in a string
        memset(msgbuf, 0, MSGSIZE + 1);
        textFile = open(localDir, O_RDONLY);
        readReturn = read(textFile, msgbuf, 1);
        while (readReturn > 0)
        {
            text = text + string(msgbuf);
            readReturn = read(textFile, msgbuf, 1);
        }
        if (readReturn < 0)
        {
            perror(" Error in reading from input file");
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
            link = text.substr(0, spotStart);
            link = link_remove_www(link);
            link = link_remove_space(link);
            link = link_getLocation(link);
            siteList.push_front(link);

            text.erase(0, spotStart + linkStart.length());
        }

        // Process last link
        if (flag)
        {
            link = text;
            link = link_remove_www(link);
            link = link_remove_space(link);
            link = link_getLocation(link);
            siteList.push_front(link);
        }
        workerOutput(&siteList, fileName);
        raise(SIGSTOP);
        siteList.clear();
    }

    exit(0);
}