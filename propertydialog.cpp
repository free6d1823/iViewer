#include "propertydialog.h"
#include "ui_propertydialog.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QGuiApplication>
#include <QMessageBox>
#include <QImageReader>

void Yuy444ToRgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb, bool uFirst);
QImage* LoadFileRgb(const char* filename, FileFormat* fmt);
QImage* LoadFileY4m(const char* filename, FileFormat* fmt);
QImage* LoadFileYuv(const char* filename, FileFormat* fmt);
QImage* LoadFileQImage(const char* filename, FileFormat* fmt);
bool ParserFileName(const char* filename, FileFormat* fmt);
bool ParserFileY4m(const char* filename, FileFormat* fmt);
bool ParserFileQImage(const char* filename, FileFormat* fmt);

SupportedFileFormat gSupportedFileFormat[] =
{
    {"Y4M", "y4m", "Y4M lossless", ParserFileY4m, LoadFileY4m},
    {"YUV420p", "yuv", "YUV 420 plan mode", ParserFileName, LoadFileYuv},
    {"RGBAp", "rgba", "RGBA plan mode", ParserFileName, LoadFileRgb},
    {"JPEG", "jpg", "JPEG", ParserFileQImage, LoadFileQImage},
    {"PNG", "png", "PNG file", ParserFileQImage, LoadFileQImage}
};

#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)


// YUV -> RGB
#define C(Y) ( (Y) - 16  )
#define D(U) ( (U) - 128 )
#define E(V) ( (V) - 128 )

#define YUV2R(Y, U, V) CLIP(( 298 * C(Y)              + 409 * E(V) + 128) >> 8)
#define YUV2G(Y, U, V) CLIP(( 298 * C(Y) - 100 * D(U) - 208 * E(V) + 128) >> 8)
#define YUV2B(Y, U, V) CLIP(( 298 * C(Y) + 516 * D(U)              + 128) >> 8)
//////
/// \brief YuyvToRgb32 YUV420 to RGBA 32
/// \param pYuv
/// \param width
/// \param stride
/// \param height
/// \param pRgb     output RGB32 buffer
/// \param uFirst   true if pYuv is YUYV, false if YVYU
///
static void YuyvToRgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb, bool uFirst)
{
    //YVYU - format
    int nBps = width*4;
    unsigned char* pY1 = pYuv;

    unsigned char* pV;
    unsigned char* pU;

    if (uFirst) {
        pU = pY1+1; pV = pU+2;
    } else {
        pV = pY1+1; pU = pV+2;
    }


    unsigned char* pLine1 = pRgb;

    unsigned char y1,u,v;
    for (int i=0; i<height; i++)
    {
        for (int j=0; j<width; j+=2)
        {
            y1 = pY1[2*j];
            u = pU[2*j];
            v = pV[2*j];
            pLine1[j*4] = YUV2B(y1, u, v);//b
            pLine1[j*4+1] = YUV2G(y1, u, v);//g
            pLine1[j*4+2] = YUV2R(y1, u, v);//r
            pLine1[j*4+3] = 0xff;

            y1 = pY1[2*j+2];
            pLine1[j*4+4] = YUV2B(y1, u, v);//b
            pLine1[j*4+5] = YUV2G(y1, u, v);//g
            pLine1[j*4+6] = YUV2R(y1, u, v);//r
            pLine1[j*4+7] = 0xff;
        }
        pY1 += stride;
        pV += stride;
        pU += stride;
        pLine1 += nBps;

    }
}
void Yuy444ToRgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb, bool uFirst)
{
    //YVU420 - format 4Y:1V:1U
    int nBps = width*4;
    unsigned char* pY1 = pYuv;
    unsigned char* pY2 = pYuv+stride;

    unsigned char* pV;
    unsigned char* pU;
//try 422 plan
    if (uFirst) {
        pU = pY1+stride*height; pV = pU+width*height/4;
    } else {
        pV = pY1+stride*height; pU = pV+stride*height/4;
    }


    unsigned char* pLine1 = pRgb;
    unsigned char* pLine2 = pRgb+nBps;

    unsigned char y1,y2,u,v;
    for (int i=0; i<height; i+=2)
    {
        for (int j=0; j<width; j++)
        {
            y1 = pY1[j];
            u = pU[j/2];
            v = pV[j/2];
            pLine1[j*4] = YUV2B(y1, u, v);//b
            pLine1[j*4+1] = YUV2G(y1, u, v);//g
            pLine1[j*4+2] = YUV2R(y1, u, v);//r
            pLine1[j*4+3] = 0xff;

            y2 = pY2[j];

            pLine2[j*4] = YUV2B(y2, u, v);//b
            pLine2[j*4+1] = YUV2G(y2, u, v);//g
            pLine2[j*4+2] = YUV2R(y2, u, v);//r
            pLine2[j*4+3] = 0xff;

            j++;

            y1 = pY1[j];
            pLine1[j*4] = YUV2B(y1, u, v);//b
            pLine1[j*4+1] = YUV2G(y1, u, v);//g
            pLine1[j*4+2] = YUV2R(y1, u, v);//r
            pLine1[j*4+3] = 0xff;

            y2 = pY2[j];

            pLine2[j*4] = YUV2B(y2, u, v);//b
            pLine2[j*4+1] = YUV2G(y2, u, v);//g
            pLine2[j*4+2] = YUV2R(y2, u, v);//r
            pLine2[j*4+3] = 0xff;

        }
        pY1 = pY2 + stride;
        pY2 = pY1 + stride;
        pV += stride/2;
        pU += stride/2;
        pLine1 = pLine2 + nBps;
        pLine2 = pLine1 + nBps;

    }
}

bool ParserFileName(const char* filename, FileFormat* fmt)
{
    char value[32];
    char* p2 = strrchr(filename, '_');
    if(!p2)
        return false;
    char* p1 = strrchr(filename, '.');
    if (!p1)
        p1 = (char*) filename+strlen(filename);
    long long length = p1-p2-1;
    memcpy(value, p2+1, (size_t)length);
    value[length] = 0;
    p1 = strrchr(value, 'x');
    if(!p1)
        return false;
    *p1 = 0;
    p2 = p1+1;
    int width = atoi(value);
    int height = atoi(p2);
    if (width <= 0 || height <=0) {
        return false;
    }
    fmt->width = width;
    fmt->height = height;
    return true;

}
bool ParserFileY4m(const char* filename, FileFormat* fmt)
{
    FILE* fp;

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        return false;
    }

    char line[256];
    bool bOK = false;
    while(fgets(line, sizeof(line), fp) != NULL){
        char* p1 = strtok(line, " ");
        if(0!= strcmp(p1, "YUV4MPEG2")) {
             break;
        }
       while((p1 = strtok(NULL, " \n")) != NULL){
           switch(p1[0]){
           case 'W':
               fmt->width = atoi(p1+1);
               break;
           case 'H':
               fmt->height = atoi(p1+1);

               break;
           case 'I':
               if (p1[1] == 'p')
                   fmt->interlace = 0;
               else if(p1[1] == 't')
                   fmt->interlace = 1;
               else if(p1[1] == 'b')
                   fmt->interlace = 2;
               break;
           case 'F': //Fdd:nn
           {
               char* p2 = strchr(p1, ':');
               if (p2!= NULL){
                   *p2 = 0;
                   int dd = atoi(p1+1);
                   int nn = atoi(p2+1);
                   if (nn>0)
                       fmt->fps = (float)dd/(float)nn;
               }
               break;
           }
           case 'C':
           {
               char* p2 = p1+1;
               if ( strcmp(p2, "444") == 0) {
                   fmt->colorspace = 2;
               } else if (strcmp(p2, "422") == 0) {
                   fmt->colorspace = 1;
               } else  { //420
                   fmt->colorspace = 0;
               }
               break;
           }
           default:
               break;

           }
           //check next param
       }
       //parser until find FRAME data
       if(fgets(line, sizeof(line), fp) == NULL) {
           break;
       }
       if (strstr(line, "FRAME"))
           bOK = true;
       break;
    }
    return bOK;
}
bool ParserFileQImage(const char* filename, FileFormat* fmt)
{
    QImageReader reader;
    reader.setFileName(filename);
    reader.setAutoDetectImageFormat(true);
    QImage newImage = reader.read();
    if (newImage.isNull())
        return false;
    fmt->width = newImage.width();
    fmt->height = newImage.height();
    fmt->stride = newImage.bytesPerLine();
    fmt->length = newImage.sizeInBytes();
    return true;
}
QImage* LoadFileQImage(const char* filename, FileFormat* fmt)
{
    QImageReader reader;
    (void)fmt;
    reader.setFileName(filename);
    reader.setAutoDetectImageFormat(true);
    QImage* newImage = new QImage(fmt->width, fmt->height, QImage::Format_RGBA8888);
    reader.read(newImage);

    return newImage;
}
QImage*LoadFileRgb(const char* filename, FileFormat* fmt)
{
    FILE* fp;
    QImage* newImage = nullptr;
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        return nullptr;
    }
    fmt->bitsPerPixel = 4*8;
    fmt->stride = fmt->width*fmt->bitsPerPixel>>3; //check alignment
    fmt->length = fmt->stride*fmt->height;
    unsigned char* pRgb32 = nullptr;
    void* pYuv = malloc(fmt->length);
    while (pYuv) {
        fmt->pBuffer = pYuv;
        if(fread(pYuv, 1, fmt->length, fp) != fmt->length) {
            break;
        }
        pRgb32 = (unsigned char*) malloc(fmt->width*4*fmt->height);
        if (!pRgb32)
            break;
        memcpy(pRgb32, pYuv, fmt->length);
        newImage = new QImage(pRgb32,
                        fmt->width, fmt->height, QImage::Format_RGBA8888);
        break;
    }


    fclose(fp);
    return newImage;
}
QImage*LoadFileYuv(const char* filename, FileFormat* fmt)
{
    FILE* fp;
    QImage* newImage = nullptr;
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        return nullptr;
    }
    fmt->bitsPerPixel = 2*8;
    fmt->stride = fmt->width*fmt->bitsPerPixel>>3; //check alignment
    fmt->length = fmt->stride*fmt->height;

    unsigned char* pRgb32 = nullptr;
    void* pYuv = malloc(fmt->length);
    while (pYuv) {
        fmt->pBuffer = pYuv;
        if(fread(pYuv, 1, fmt->length, fp) != fmt->length) {
            break;
        }
        pRgb32 = (unsigned char*) malloc(fmt->width*4*fmt->height);
        if (!pRgb32)
            break;
        YuyvToRgb32((unsigned char*)fmt->pBuffer,
                    fmt->width, fmt->stride, fmt->height,
                    pRgb32, true);
        newImage = new QImage(pRgb32,
                        fmt->width, fmt->height, QImage::Format_RGBA8888);
        break;
    }


    fclose(fp);
    return newImage;
}
QImage* LoadFileY4m(const char* filename, FileFormat* fmt)
{
    FILE* fp;
    QImage* newImage = nullptr;
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        return nullptr;
    }

    char line[256];
    bool bOK = false;
    while(fgets(line, sizeof(line), fp) != NULL){

        if(!bOK){
            if (strstr(line, "FRAME")==NULL)
                continue;
            bOK = true;
        }

       fmt->bitsPerPixel = 3*8/2;
       fmt->stride = fmt->width*fmt->bitsPerPixel>>3; //check alignment
       fmt->length = fmt->stride*fmt->height;


       void* pYuv = malloc(fmt->length);
       if (!pYuv) {
           break;
       }
       if(fread(pYuv, 1, fmt->length, fp) != fmt->length) {
           free(pYuv);
           break;
       }
       fmt->pBuffer = pYuv;
       unsigned char* pRgb32 = (unsigned char*) malloc(fmt->width*4*fmt->height);
       if (!pRgb32)
               break;
       Yuy444ToRgb32((unsigned char*)fmt->pBuffer, fmt->width, fmt->width, fmt->height,
                                       pRgb32, false);
       newImage = new QImage(pRgb32,
                       fmt->width, fmt->height, QImage::Format_RGBA8888);
       if(!newImage)
           free(pRgb32);
       break;
    }


    fclose(fp);
    return newImage;
}
QImage* PropertyDialog::CreateImage(const char* filename, FileFormat* fmt)
{
    int i;
    for(i=0; i< MAX_FILE_FORMAT; i++) {
        if( strcmp(gSupportedFileFormat[i].name, fmt->format)==0){
            break;
        }
    }
    QImage* pImg = nullptr;

    if (i < MAX_FILE_FORMAT && gSupportedFileFormat[i].loadFunc)
      pImg =  gSupportedFileFormat[i].loadFunc(filename, fmt);
    return pImg;

}
void PropertyDialog::setFileName(const char* path)
{
    char dir[256];
    strcpy(mFilePath, path);
    strcpy(dir, mFilePath);
    char* p2 = dir;
    char* p1 = strrchr(dir, '/');
    if(p1){
        *p1 = 0;
        p2 = p1+1;
        ui->editDir->setText(dir);
    } else  {
        ui->editDir->setText("");
    }
    ui->editFile->setText(p2);

}
const char* PropertyDialog::getFileName()
{
    return mFilePath;
}
PropertyDialog::PropertyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PropertyDialog)
{
    ui->setupUi(this);

    memset(&mFmt, 0, sizeof(mFmt));

    for (int i=0; i< MAX_FILE_FORMAT; i++)
        ui->listFormat->addItem(gSupportedFileFormat[i].name);
    ui->listFormat->setCurrentRow(0);
    onClickList(0);
    connect(ui->btnBrowse, SIGNAL(clicked()), SLOT(onBrowse()));
    connect(ui->listFormat, SIGNAL(currentRowChanged(int)), SLOT(onClickList(int)));
    connect(ui->listFormat, SIGNAL(itemSelectionChanged()), SLOT(onListChanged()));



}
void PropertyDialog::onListChanged()
{
    int i = ui->listFormat->currentRow();
    onClickList(i);
}
PropertyDialog::~PropertyDialog()
{
    delete ui;
}
void PropertyDialog::on_buttonBox_accepted()
{

    //update UI to mFmt
    get(&mFmt);
    if(mFmt.width<0 || mFmt.height < 0) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Parameters error!"));

        return;
    }
    QDialog::accept();
}
void PropertyDialog::set(FileFormat* fmt)
{
    ui->editWidth->setText(QString::number(fmt->width));
    ui->editHeight->setText(QString::number(fmt->height));
    for(int i=0; i< MAX_FILE_FORMAT; i++) {
        if( strcmp(gSupportedFileFormat[i].name, fmt->format)==0){
            ui->listFormat->setCurrentRow(i);
            break;
        }
    }
}
void PropertyDialog::get(FileFormat* fmt)
{

    fmt->width = ui->editWidth->toPlainText().toInt();
    fmt->height = ui->editHeight->toPlainText().toInt();
    int i = ui->listFormat->currentRow();
    if(i>=0 && i< MAX_FILE_FORMAT)
        strncpy(fmt->format, gSupportedFileFormat[i].name, MAX_FORMAT_NAME);

}
void PropertyDialog::onClickList(int i)
{
    if (i >=0 && i < MAX_FILE_FORMAT) {
        ui->labelDesc->setText(gSupportedFileFormat[i].desc);
    }

}

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }
    QStringList filters;

    for (int i=0; i< sizeof(gSupportedFileFormat)/sizeof(gSupportedFileFormat[0]); i++){
        QString item = QString("%1 (*.%2)").arg(gSupportedFileFormat[i].desc).arg(gSupportedFileFormat[i].ext);
        filters << item;
    }

    dialog.setNameFilters(filters);;
   if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("jpg");
}
int PropertyDialog::parserFileName(const char* pathname, FileFormat* fmt)
{
    int index = -1;
    int i;
    char ext[32];

    const char* p1 = strrchr(pathname, '.');
    if (!p1)
        return index;
    strncpy(ext, p1+1, 32);

    //check extension name
    for (i=0; i< MAX_FILE_FORMAT; i++){
        if (stricmp(ext, gSupportedFileFormat[i].ext)==0)
            break;
    }
    if (i >= MAX_FILE_FORMAT){
        //unsupported format
        return index;
    }
    index = i;
    strncpy(fmt->format, gSupportedFileFormat[index].name, MAX_FORMAT_NAME);

    if(!gSupportedFileFormat[index].headerFunc(pathname, fmt)) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Filename format not correct!"));
        return -1;
    }

}

void PropertyDialog::onBrowse()
{
    QFileDialog dialog(this, tr("Open Image"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString filename = dialog.selectedFiles().first();
        setFileName(filename.toUtf8().data());
        FileFormat fmt;
        memset(&fmt, 0, sizeof(fmt));

        int id = parserFileName(mFilePath, &fmt);
        if (id >=0)
            set(&fmt);
    }

}


