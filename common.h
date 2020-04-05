#ifndef COMMON_H
#define COMMON_H
#include <QImage>
#define MAX_FORMAT_NAME 8
typedef struct _fileFormat {
    char format[MAX_FORMAT_NAME];
    int width;
    int height;
    float fps;
    int interlace;//0 progressive, 1 top field first, 2 bottom field first
    int colorspace; //0: YUV420, 1:YUV422, 2YUV444
    void* pBuffer;
    long long length;
    int stride;
    int bitsPerPixel;
    ////
}FileFormat;

typedef QImage* (*LoadFileFunction)(const char* filename, FileFormat* fmt);
typedef bool (*ParserFileHeaderFunction)(const char* filename, FileFormat* fmt);
typedef struct _supportedFileFormat{
    const char* name;
    const char* ext;
    const char* desc;
    ParserFileHeaderFunction headerFunc;
    LoadFileFunction loadFunc;
}SupportedFileFormat;
#define MAX_FILE_FORMAT 5

extern SupportedFileFormat gSupportedFileFormat[MAX_FILE_FORMAT];
#endif // COMMON_H
