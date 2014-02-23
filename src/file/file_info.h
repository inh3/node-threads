#ifndef _FILE_INFO_H_
#define _FILE_INFO_H_

// C
#include <stdlib.h>

class FileInfo
{
    public:

        FileInfo();
        FileInfo(const char* relativeFilePath, const char* currentDirectory = NULL);
        ~FileInfo();

        void LoadFile(const char* relativeFilePath, const char* currentDirectory = NULL);
        const char* FileContents();

    protected:

        // ensure copy constructor can't get called
        FileInfo(FileInfo const&);
        void operator=(FileInfo const&);

    private:

        void GetFileInfo(const char* relativeFilePath, const char* currentDirectory = NULL);
        void GetFileContents(const char* fullFilePath);

        const char* fileName;
        const char* folderPath;
        const char* fullPath;
        const char* fileBuffer;
        int         fileBufferLength;

};

#endif /* _FILE_INFO_H_ */