#ifndef FILE_H_
#define FILE_H_ 1

#include <aquabotics_defines.h>

#include "FS.h"
#include "SPIFFS.h"

#include <stdio.h>
#include <stdlib.h>

AQUABOTICS_BEGIN_NAMESPACE

class FileSystem
{
public:
    void begin();
    void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
    String read_file(const char *path);
    String readWholeFile(fs::FS &fs, const char *path);
    void writeFile(fs::FS &fs, const char *path, const char *message);
    void appendFile(fs::FS &fs, const char *path, const char *message);
    void renameFile(fs::FS &fs, const char *path1, const char *path2);
    void deleteFile(fs::FS &fs, const char *path);
    int deleteFileWithPrefix(const char *_Prefix);
    void testFileIO(fs::FS &fs, const char *path);
    FileSystem();
    FileSystem(const FileSystem &orig);
    virtual ~FileSystem();
};

AQUABOTICS_END_NAMESPACE

#endif