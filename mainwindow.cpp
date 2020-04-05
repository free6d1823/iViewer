#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QScrollBar>
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QScreen>

#include <QImageWriter>
#include <QStandardPaths>
#include <QByteArray>
#include <QList>
#include <QClipboard>
#include <QMimeData>
#include <QScrollArea>
#include "imagewin.h"
#include <string.h>
#include "propertydialog.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),ui(new Ui::MainWindow),
    mImageView(new ImageWin),
    mZoomFactor(1)
{
    ui->setupUi(this);
    setCentralWidget(mImageView);

    createMenuAndToolbar();
    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);
    memset(&mSourceFile, 0, sizeof(mSourceFile));
    mSourceFileName[0] = 0;
}

MainWindow::~MainWindow()
{
    freeFile(&mSourceFile);
    delete ui;
}


void MainWindow::setImage(const QImage &newImage)
{
    mZoomFactor = 1.0;
    mImageView->setImage(newImage);
    mFitToWindowAct->setEnabled(true);
    updateActions();

    if (!mFitToWindowAct->isChecked())
        mImageView->adjustSize();
}
bool MainWindow::saveFile(const QString &fileName)
{
    QImageWriter writer(fileName);

    if (!writer.write(*mImageView->getImage())) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot write %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName)), writer.errorString());
        return false;
    }
    const QString message = tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
    statusBar()->showMessage(message);
    return true;
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

void MainWindow::onFileSaveAs()
{
    QFileDialog dialog(this, tr("Save File As"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptSave);

    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first())) {}

}
void MainWindow::onEditCopy()
{
#ifndef QT_NO_CLIPBOARD
    QGuiApplication::clipboard()->setImage(* mImageView->getImage());
#endif // !QT_NO_CLIPBOARD
}
#ifndef QT_NO_CLIPBOARD
static QImage clipboardImage()
{
    if (const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData()) {
        if (mimeData->hasImage()) {
            const QImage image = qvariant_cast<QImage>(mimeData->imageData());
            if (!image.isNull())
                return image;
        }
    }
    return QImage();
}
#endif // !QT_NO_CLIPBOARD

void MainWindow::onEditPaste()
{
#ifndef QT_NO_CLIPBOARD
    const QImage newImage = clipboardImage();
    if (newImage.isNull()) {
        ui->statusBar->showMessage(tr("No image in clipboard"));
    } else {
        setImage(newImage);
        setWindowFilePath(QString());
        const QString message = tr("Obtained image from clipboard, %1x%2, Depth: %3")
            .arg(newImage.width()).arg(newImage.height()).arg(newImage.depth());
        ui->statusBar->showMessage(message);
    }
#endif // !QT_NO_CLIPBOARD
}

void MainWindow::onViewZoomin()
{
    scaleImage(1.25);
}

void MainWindow::onViewZoomout()
{
    scaleImage(0.8);
}

void MainWindow::onViewNormalSize()
{
    mImageView->adjustSize();
    mZoomFactor = 1.0;
}

void MainWindow::onViewFitToWindow()
{
    bool fitToWindow = mFitToWindowAct->isChecked();
    mImageView->setWidgetResizable(fitToWindow);
    if (!fitToWindow)
        onViewNormalSize();
    updateActions();
}
void MainWindow::onViewShowRuler()
{
    mImageView->showRulers(!mImageView->isRulersShown());
}

void MainWindow::onHelpAbout()
{
    QMessageBox::about(this, tr("About Application"),
             tr("The <b>Application</b>  Copyrights 2018."));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}
void MainWindow::createMenuAndToolbar()
{
    QToolBar *fileToolBar = ui->mainToolBar;
    QMenu* fileMenu = ui->menuBar->addMenu(tr("&File"));
    const QIcon openIcon = QIcon(":/images/open.png");
    QAction *openAct = fileMenu->addAction(openIcon, tr("&Open..."), this,
                    SLOT(onFileOpen()), QKeySequence::Open);
    openAct->setStatusTip(tr("Open an image file."));
    const QIcon saveIcon = QIcon(":/images/save.png");
    mSaveAsAct = fileMenu->addAction(saveIcon, tr("&Save..."), this,
                                     SLOT(onFileSaveAs()));
    mSaveAsAct->setStatusTip(tr("Save file."));
    mSaveAsAct->setEnabled(false);
    fileMenu->addSeparator();
    mFilePropertyAct = fileMenu->addAction(tr("&Property..."), this,
            SLOT(onFileProperty()));
    mFilePropertyAct->setStatusTip(tr("Show or modify image property."));
    mFilePropertyAct->setEnabled(true);
    fileMenu->addSeparator();
    const QIcon closeIcon =  QIcon(":/images/exit.png");
    QAction* closeAct = fileMenu->addAction(closeIcon,tr("E&xit"), this,
            SLOT(close()), QKeySequence::Quit);
    closeAct->setStatusTip(tr("Exit this program."));
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(mSaveAsAct);
    fileMenu->addSeparator();
    /*******************************************/
    QMenu *editMenu = ui->menuBar->addMenu(tr("&Edit"));
    const QIcon copyIcon =  QIcon(":/images/copy.png");
    mCopyAct = editMenu->addAction(copyIcon, tr("&Copy"), this,
                         SLOT(onEditCopy()),QKeySequence::Copy);
    mCopyAct->setEnabled(false);
    const QIcon pasteIcon =  QIcon(":/images/paste.png");
    QAction *pasteAct = editMenu->addAction(pasteIcon, tr("&Paste"), this,
                         SLOT(onEditPaste()), QKeySequence::Paste);
    pasteAct->setStatusTip(tr("paste from clipboard."));
    /*******************************************/
    QMenu* viewMenu = ui->menuBar->addMenu(tr("&View"));
    const QIcon zoominIcon =  QIcon(":/images/zoomin.png");
    mZoomInAct = viewMenu->addAction(zoominIcon,tr("Zoom &In"), this,
                                    SLOT(onViewZoomin()), QKeySequence::ZoomIn );
    mZoomInAct->setEnabled(false);
    mZoomInAct->setStatusTip(tr("Magfying image."));

    const QIcon zoomoutIcon = QIcon(":/images/zoomout.png");
    mZoomOutAct = viewMenu->addAction(zoomoutIcon,tr("Zoom &Out"), this,
                     SLOT(onViewZoomout()), QKeySequence::ZoomOut );
    mZoomOutAct->setStatusTip(tr("zoom out image."));
    mZoomOutAct->setEnabled(false);
    const QIcon normalSizeIcon = QIcon(":/images/normalsize.png");
    mNormalSizeAct = viewMenu->addAction(normalSizeIcon, tr("Normal &Size"), this,
                     SLOT(onViewNormalSize()), tr("Ctrl+S") );
    viewMenu->addSeparator();
    mNormalSizeAct->setStatusTip(tr("set normal size image."));
    mNormalSizeAct->setEnabled(false);
     const QIcon fitIcon = QIcon(":/images/fittowindow.png");
    mFitToWindowAct = viewMenu->addAction(fitIcon, tr("Fit to Window"), this,
                     SLOT(onViewFitToWindow()), tr("Ctrl+S") );
    mFitToWindowAct->setStatusTip(tr("set normal size image."));
    mFitToWindowAct->setEnabled(false);
    mFitToWindowAct->setCheckable(true);
    mFitToWindowAct->setShortcut(tr("Ctrl+F"));

    const QIcon rulerIcon = QIcon(":/images/ruler.png");
    QAction* rulerAct = viewMenu->addAction(rulerIcon, tr("Show &Ruler"), this,
                     SLOT(onViewShowRuler()), tr("Ctrl+R") );
    viewMenu->addSeparator();
    rulerAct->setStatusTip(tr("Toggle rulers."));
    rulerAct->setEnabled(true);

    fileToolBar->addAction(mZoomInAct);
    fileToolBar->addAction(mZoomOutAct);
    fileToolBar->addAction(mNormalSizeAct);
    fileToolBar->addSeparator() ;
    fileToolBar->addAction(mFitToWindowAct);
    fileToolBar->addAction(rulerAct);
    fileToolBar->addSeparator() ;

    QMenu* helpMenu = ui->menuBar->addMenu(tr("&Help"));
    const QIcon aboutIcon = QIcon(":/images/about.png");
    QAction* aboutAct = helpMenu->addAction(aboutIcon, tr("A&bout"), this, SLOT(onHelpAbout()));
    aboutAct->setStatusTip(tr("Copyrights."));
    fileToolBar->addAction(aboutAct);


}
bool MainWindow::UpdateImage()
{

}
void MainWindow::onFileOpen()
{
    PropertyDialog dlg;
    dlg.setFileName(mSourceFileName);
    dlg.set(&mSourceFile);
    if (dlg.exec() == QDialog::Accepted) {
        dlg.get(&mSourceFile);
        strncpy(mSourceFileName, dlg.getFileName(), 256);
        QImage* newImage = dlg.CreateImage(mSourceFileName, &mSourceFile);
        if(newImage) {
            setImage(*newImage);
            setWindowFilePath(mSourceFileName);
            QString file1 = QDir::toNativeSeparators(mSourceFileName);
            QString message = tr("Opened \"%1\", %2x%3, Depth: 32")
                .arg(file1).arg(mSourceFile.width).arg(mSourceFile.height);
            statusBar()->showMessage(message);

        }
    }
}
void MainWindow::updateActions()
{
    mSaveAsAct->setEnabled(!mImageView->getImage()->isNull());
    //mFilePropertyAct->setEnabled(!mImageView->getImage()->isNull());
    mCopyAct->setEnabled(!mImageView->getImage()->isNull());
    mZoomInAct->setEnabled(!mFitToWindowAct->isChecked());
    mZoomOutAct->setEnabled(!mFitToWindowAct->isChecked());
    mNormalSizeAct->setEnabled(!mFitToWindowAct->isChecked());
}
void MainWindow::scaleImage(double factor)
{
//    Q_ASSERT(mImageLabel->pixmap());
    mZoomFactor *= factor;
    mImageView->scaleImage(mZoomFactor);
    adjustScrollBar(mImageView->horizontalScrollBar(), factor);
    adjustScrollBar(mImageView->verticalScrollBar(), factor);

    mZoomInAct->setEnabled(mZoomFactor < 10.0);
    mZoomOutAct->setEnabled(mZoomFactor > 0.1);
}
void MainWindow::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}
void MainWindow::freeFile(FileFormat* format)
{
    if (format)
    {
        if (format->pBuffer)
            free(format->pBuffer);
        memset(format, 0, sizeof(FileFormat));
    }
}
