#include "utils.hpp"

using namespace std;
string fifoNameBase = "fifo";
string outputFolder = "/tmp/snifferOut/";
string fifoFolder = "/tmp/fifo/";
list<pid_t> childList;
list<string> ignoreList;
list<worker *> *workerList;
char *directory;

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
}

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
}

char *getFileName(char *token)
{
    strtok(token, " ");
    strtok(NULL, " ");
    token = strtok(NULL, " ");
    return token;
}

