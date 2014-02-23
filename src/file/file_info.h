#ifndef _FILE_INFO_H_
#define _FILE_INFO_H_

class FileInfo
{
    public:

        FileInfo();
        FileInfo(const char* relativeFilePath, const char* currentDirectory = 0);
        ~FileInfo();

        void LoadFile(const char* relativeFilePath, const char* currentDirectory = 0);
        const char* FileContents();

        const char* fullPath;
        const char* folderPath;

    protected:

        // ensure copy constructor can't get called
        FileInfo(FileInfo const&);
        void operator=(FileInfo const&);

    private:

        void GetFileInfo(const char* relativeFilePath, const char* currentDirectory = 0);
        void GetFileContents(const char* fullFilePath);

        const char* fileName;
        const char* fileBuffer;
        int         fileBufferLength;

};

#endif /* _FILE_INFO_H_ */