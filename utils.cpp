#include "utils.hpp"

using namespace std;
string fifoNameBase = "/tmp/fifo";
string outputFolder = "/tmp/snifferOut/";
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

void fixDir(char *givenPath)
{
    // Add the "/" at the end of a directory path, if it does not exist
    if (strcmp(&givenPath[strlen(givenPath) - 1], "/") != 0)
    {
        givenPath[strlen(givenPath)] = '/';
        givenPath[strlen(givenPath) + 1] = '\n';
    }
}

char *getFileName(char *token)
{
    // Get the name of the changed file, returned from inotifywait
    strtok(token, " ");
    strtok(NULL, " ");
    token = strtok(NULL, " ");
    return token;
}

