#ifndef _FILE_INFO_H_
#define _FILE_INFO_H_

// C
#include <stdlib.h>

class FileInfo
{
    public:

        FileInfo(const char* relativeFilePath, const char* currentDirectory = NULL);
        ~FileInfo();

    protected:

        // ensure defult/copy constructor can't get called
        FileInfo();
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