#define _FILE_INFO_CC_

#include "file_info.h"

// C++
#include <string>

// C
#include <stdio.h>
#include <string.h>
#include <limits.h>

// FILE and fxxx()
#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#endif

FileInfo::FileInfo(const char* relativeFilePath, const char* currentDirectory)
{
    this->GetFileInfo(relativeFilePath, currentDirectory);

    // read contents of file
    this->GetFileContents(this->fullPath);
}

FileInfo::~FileInfo()
{
    free((void*)this->fullPath);
    free((void*)this->folderPath);

    free((void*)this->fileBuffer);
    this->fileBufferLength = 0;

    // don't need to delete this because
    // it was a reference into fullPath
    this->fileName = NULL;
}

void FileInfo::GetFileInfo(const char* relativeFilePath, const char* currentDirectory)
{

    // check if relative path should be combined with current directory
    std::string filePath(relativeFilePath);
    if((currentDirectory != NULL) && (strlen(relativeFilePath) > 2))
    {
        // combine current directory to relative path if it starts with './' or '../'
        if(((relativeFilePath[0] == '.') && ((relativeFilePath[1] == '\\') || (relativeFilePath[1] == '/'))) ||
            (relativeFilePath[0] == '.' && relativeFilePath[1] == '.' && ((relativeFilePath[2] == '\\') || (relativeFilePath[2] == '/'))))
        {
            filePath.insert(0, currentDirectory);
        }
    }

    // get the full path of the file
    #ifdef _WIN32
        // http://msdn.microsoft.com/en-us/library/506720ff.aspx
        this->fullPath = (const char*)malloc(_MAX_PATH + 1);
        memset((void*)this->fullPath, 0, _MAX_PATH + 1);
        _fullpath((char*)this->fullPath, filePath.c_str(), _MAX_PATH);
        
        // http://msdn.microsoft.com/en-us/library/a2xs1dts.aspx
        if((_access_s((char*)this->fullPath, 0)) != 0)
        {
            //fprintf(stdout, "[ Utilities - Error ] Invalid File: %s\n", filePath.c_str());
            free((void*)this->fullPath);
            this->fullPath = 0;
        }
    #else
        this->fullPath = (char*)malloc(PATH_MAX + 1);
        memset((void*)this->fullPath, 0, PATH_MAX + 1);
        if(realpath(filePath.c_str(), (char *)this->fullPath) == NULL)
        {
            //fprintf(stdout, "[ Utilities - Error ] Invalid File: %s\n", filePath.c_str());
            free((void*)this->fullPath);
            this->fullPath = 0;
        }
    #endif

    // path is valid
    if(this->fullPath != 0)
    {
        // get the file name only
        // http://stackoverflow.com/a/5902743
        const char *charPtr = this->fullPath + strlen(this->fullPath);
        for (; charPtr > this->fullPath; charPtr--)
        {
            if ((*charPtr == '\\') || (*charPtr == '/'))
            {
                this->fileName = ++charPtr;
                break;
            }
        }

        // allocate and store the folder only path
        unsigned int folderPathLength = strlen(this->fullPath) - strlen(this->fileName);
        this->folderPath = (const char*)malloc(folderPathLength + 1);
        memset((void*)this->folderPath, 0, folderPathLength + 1);
        memcpy((void*)this->folderPath, this->fullPath, folderPathLength);
    }

    fprintf(stdout, "[ FileInfo::GetFileInfo ] Full Path: %s\n", this->fullPath);
    fprintf(stdout, "[ FileInfo::GetFileInfo ] Folder Path: %s\n", this->folderPath);
    fprintf(stdout, "[ FileInfo::GetFileInfo ] File Name: %s\n", this->fileName);
}

void FileInfo::GetFileContents(const char* fullFilePath)
{
    // attempt to open the file
    FILE* fd = fopen(fullFilePath, "rb");

    // clear file size
    this->fileBufferLength = 0;

    // file was valid
    if(fd != 0)
    {
        // get size of file
        fseek(fd, 0, SEEK_END);
        this->fileBufferLength = ftell(fd);
        rewind(fd);

        // allocate file buffer for file contents
        this->fileBuffer = (const char*)malloc(this->fileBufferLength + 1);
        ((char *)this->fileBuffer)[this->fileBufferLength] = 0;

        // copy file contents
        for (int charCount = 0; charCount < this->fileBufferLength;)
        {
            int charRead = static_cast<int>(
                fread((void*)&(this->fileBuffer[charCount]), 1, this->fileBufferLength - charCount, fd)
            );
            charCount += charRead;
        }

        // close the file
        fclose(fd);
    }
}