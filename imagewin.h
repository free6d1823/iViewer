#ifndef IMAGEVWIN_H
#define IMAGEVWIN_H

#include <QtWidgets/qscrollarea.h>

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>

#define RULER_BREADTH 20

class QDRuler : public QWidget
{
Q_OBJECT
Q_ENUMS(RulerType)
Q_PROPERTY(qreal origin READ origin WRITE setOrigin)
Q_PROPERTY(qreal rulerUnit READ rulerUnit WRITE setRulerUnit)
Q_PROPERTY(qreal rulerZoom READ rulerZoom WRITE setRulerZoom)
public:
  enum RulerType { Horizontal, Vertical };
QDRuler(QDRuler::RulerType rulerType, QWidget* parent);

QSize minimumSizeHint() const
{
  return QSize(RULER_BREADTH,RULER_BREADTH);
}

QDRuler::RulerType rulerType() const
{
  return mRulerType;
}

qreal origin() const
{
  return mOrigin;
}

qreal rulerUnit() const
{
  return mRulerUnit;
}

qreal rulerZoom() const
{
  return mRulerZoom;
}

public slots:

void setOrigin(const qreal origin);
void setRulerUnit(const qreal rulerUnit);
void setRulerZoom(const qreal rulerZoom)
{
  if (mRulerZoom != rulerZoom)
  {
    mRulerZoom = rulerZoom;
    update();
  }
}

void setCursorPos(const QPoint cursorPos)
{
//  mCursorPos = this->mapFromGlobal(cursorPos);
//  mCursorPos += QPoint(RULER_BREADTH,RULER_BREADTH);
    mCursorPos = cursorPos;
  update();
}

void setMouseTrack(const bool track)
{
  if (mMouseTracking != track)
  {
    mMouseTracking = track;
    update();
  }
}

protected:
void mouseMoveEvent(QMouseEvent* event);
void paintEvent(QPaintEvent* /*event*/);
private:
void drawAScaleMeter(QPainter* painter, QRectF rulerRect, qreal scaleMeter, qreal startPositoin);

void drawFromOriginTo(QPainter* painter, QRectF rulerRect, qreal startMark, qreal endMark, int startTickNo, qreal step, qreal startPosition, qreal unScaleStep);
void drawMousePosTick(QPainter* painter);
private:
  RulerType mRulerType;
  qreal mOrigin;
  qreal mRulerUnit;
  qreal mRulerZoom;
  QPoint mCursorPos;
  bool mMouseTracking;
  bool mDrawText;
};

class QLabel;
class ImageWin : public QScrollArea
{
    Q_OBJECT
//    Q_PROPERTY(bool widgetResizable READ widgetResizable WRITE setWidgetResizable)
//    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)

public:
    explicit ImageWin(QWidget *parent = 0);
    ~ImageWin();
    void scrollContentsBy(int dx, int dy);
    void mouseMoveEvent(QMouseEvent* event);

    void setImage(const QImage &newImage);
    void adjustSize( );
    void scaleImage(double factor);
    QImage* getImage(){ return &mImage;}
    void showRulers(bool bShow);
    bool isRulersShown();
protected:

private:
    QDRuler* mHorzRuler;
    QDRuler* mVertRuler;
    QWidget* mRulerCorner;

    QImage mImage;
    QLabel *mImageLabel;
};

#endif // IMAGEVWIN_H
