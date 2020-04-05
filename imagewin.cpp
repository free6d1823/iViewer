#include <QAbstractScrollArea>
#include <QLabel>
#include "imagewin.h"
#include <QGridLayout>
/*******************************************/
QDRuler::QDRuler(QDRuler::RulerType rulerType, QWidget* parent):
            QWidget(parent), mRulerType(rulerType), mOrigin(0.), mRulerUnit(1.),
            mRulerZoom(1.), mMouseTracking(true), mDrawText(false)
{
    setMouseTracking(true);
    QFont txtFont("Courier", 5,QFont::Light);
    txtFont.setStyleHint(QFont::TypeWriter,QFont::PreferOutline);
    setFont(txtFont);
}
void QDRuler::setOrigin(const qreal origin)
{
  if (mOrigin != origin)
  {
    mOrigin = origin;
    update();
  }
}
void QDRuler::setRulerUnit(const qreal rulerUnit)
{
  if (mRulerUnit != rulerUnit)
  {
    mRulerUnit = rulerUnit;
    update();
  }
}
void QDRuler::mouseMoveEvent(QMouseEvent* event)
{
  mCursorPos = event->pos();
  update();
  QWidget::mouseMoveEvent(event);
}
void QDRuler::paintEvent(QPaintEvent* /*event*/)
{
  QPainter painter(this);
    painter.setRenderHints(QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing);
    QPen pen(Qt::black,0); // zero width pen is cosmetic pen
    //pen.setCosmetic(true);
    painter.setPen(pen);
  // We want to work with floating point, so we are considering
  // the rect as QRectF
  QRectF rulerRect = this->rect();

  // at first fill the rect
  //painter.fillRect(rulerRect,QColor(220,200,180));
  painter.fillRect(rulerRect,QColor(236,233,200));

  // drawing a scale of 25
  drawAScaleMeter(&painter,rulerRect,25,(Horizontal == mRulerType ? rulerRect.height()
        : rulerRect.width())/2); //TODO: start position ahould be changed when scroll
  // drawing a scale of 50
  drawAScaleMeter(&painter,rulerRect,50,(Horizontal == mRulerType ? rulerRect.height()
        : rulerRect.width())/4);
  // drawing a scale of 100
  mDrawText = true;
  drawAScaleMeter(&painter,rulerRect,100,0);
  mDrawText = false;

  // drawing the current mouse position indicator
    painter.setOpacity(0.4);
  drawMousePosTick(&painter);
    painter.setOpacity(1.0);

  // drawing no man's land between the ruler & view
  QPointF starPt = Horizontal == mRulerType ? rulerRect.bottomLeft()
      : rulerRect.topRight();
  QPointF endPt = Horizontal == mRulerType ? rulerRect.bottomRight()
      : rulerRect.bottomRight();
  painter.setPen(QPen(Qt::black,2));
  painter.drawLine(starPt,endPt);
}
void QDRuler::drawAScaleMeter(QPainter* painter, QRectF rulerRect, qreal scaleMeter, qreal startPositoin)
{
  // Flagging whether we are horizontal or vertical only to reduce
  // to cheching many times
  bool isHorzRuler = Horizontal == mRulerType;
  qreal unscaledMeter = scaleMeter;
  scaleMeter  = scaleMeter * mRulerUnit * mRulerZoom;

  // Ruler rectangle starting mark
  qreal rulerStartMark = isHorzRuler ? rulerRect.left() : rulerRect.top();
  // Ruler rectangle ending mark
  qreal rulerEndMark = isHorzRuler ? rulerRect.right() : rulerRect.bottom();

  // Condition A # If origin point is between the start & end mard,
  //we have to draw both from origin to left mark & origin to right mark.
  // Condition B # If origin point is left of the start mark, we have to draw
  // from origin to end mark.
  // Condition C # If origin point is right of the end mark, we have to draw
  // from origin to start mark.
  if (mOrigin >= rulerStartMark && mOrigin <= rulerEndMark)
  {
    drawFromOriginTo(painter, rulerRect, mOrigin, rulerEndMark, 0, scaleMeter, startPositoin, unscaledMeter);
    drawFromOriginTo(painter, rulerRect, mOrigin, rulerStartMark, 0, -scaleMeter, startPositoin, unscaledMeter);
  }
  else if (mOrigin < rulerStartMark)
  {
        int tickNo = int((rulerStartMark - mOrigin) / scaleMeter);
        drawFromOriginTo(painter, rulerRect, mOrigin + scaleMeter * tickNo,
            rulerEndMark, tickNo, scaleMeter, startPositoin, unscaledMeter);
  }
  else if (mOrigin > rulerEndMark)
  {
        int tickNo = int((mOrigin - rulerEndMark) / scaleMeter);
    drawFromOriginTo(painter, rulerRect, mOrigin - scaleMeter * tickNo,
            rulerStartMark, tickNo, -scaleMeter, startPositoin, unscaledMeter);
  }
}
void QDRuler::drawFromOriginTo(QPainter* painter, QRectF rulerRect, qreal startMark, qreal endMark, int startTickNo, qreal step, qreal startPosition, qreal unScaleStep)
{
  bool isHorzRuler = Horizontal == mRulerType;
  int iterate = 0;

  for (qreal current = startMark;
      (step < 0 ? current >= endMark : current <= endMark); current += step)
  {
    qreal x1 = isHorzRuler ? current : rulerRect.left() + startPosition;
    qreal y1 = isHorzRuler ? rulerRect.top() + startPosition : current;
    qreal x2 = isHorzRuler ? current : rulerRect.right();
    qreal y2 = isHorzRuler ? rulerRect.bottom() : current;
    painter->drawLine(QLineF(x1,y1,x2,y2));
    if (mDrawText)
    {
      QPainterPath txtPath; //CJ: the draw scale is uncaled value, say, 100, not 100*zoom
      txtPath.addText(x1 + 1,y1 + (isHorzRuler ? 7 : -2),this->font(),QString::number(qAbs(int(unScaleStep) * startTickNo++)));
      painter->drawPath(txtPath);
      iterate++;
    }
  }
}
void QDRuler::drawMousePosTick(QPainter* painter)
{
  if (mMouseTracking)
  {
    QPoint starPt = mCursorPos;
    QPoint endPt;
    if (Horizontal == mRulerType)
    {
      starPt.setY(this->rect().top());
      endPt.setX(starPt.x());
      endPt.setY(this->rect().bottom());
    }
    else
    {
      starPt.setX(this->rect().left());
      endPt.setX(this->rect().right());
      endPt.setY(starPt.y());
    }
    painter->drawLine(starPt,endPt);
  }
}
/*******************************************/
ImageWin::ImageWin(QWidget *parent) : QScrollArea(parent),
    mImageLabel(new QLabel)
{

    mImageLabel->setBackgroundRole(QPalette::Base);
    mImageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    mImageLabel->setScaledContents(true);

    setBackgroundRole(QPalette::Dark);
    setWidget(mImageLabel);

    setViewportMargins(RULER_BREADTH,RULER_BREADTH,0,0);
    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);

    mHorzRuler = new QDRuler(QDRuler::Horizontal, this);
    mVertRuler = new QDRuler(QDRuler::Vertical, this);

    mRulerCorner = new QWidget();
    mRulerCorner->setBackgroundRole(QPalette::Window);
    mRulerCorner->setFixedSize(RULER_BREADTH,RULER_BREADTH);
    gridLayout->addWidget(mRulerCorner,0,0);
    gridLayout->addWidget(mHorzRuler,0,1);
    gridLayout->addWidget(mVertRuler,1,0);
    gridLayout->addWidget(this->viewport(),1,1);

    this->setLayout(gridLayout);


    setVisible(false);
    setMouseTracking(true);//capture mouse event and trigger draw event
}

ImageWin::~ImageWin()
{

}
void ImageWin::showRulers(bool bShow)
{
     mHorzRuler->setVisible(bShow);
     mVertRuler->setVisible(bShow);
     mRulerCorner->setVisible(bShow);
     if(bShow)
        setViewportMargins(RULER_BREADTH,RULER_BREADTH,0,0);
     else
         setViewportMargins(0,0,0,0);
}

bool ImageWin::isRulersShown()
{
      return mHorzRuler->isVisible();
}

void ImageWin::mouseMoveEvent(QMouseEvent* event)
{
    mHorzRuler->setCursorPos(event->pos());
    mVertRuler->setCursorPos(event->pos());
    QScrollArea::mouseMoveEvent(event);
}

void ImageWin::scrollContentsBy(int dx, int dy)
{
    QScrollArea::scrollContentsBy(dx,dy);
    mHorzRuler->setOrigin(mHorzRuler->origin() + dx);
    mVertRuler->setOrigin(mVertRuler->origin()+dy);
}

void ImageWin::setImage(const QImage &newImage)
{
    mImage = newImage;
    mImageLabel->setPixmap(QPixmap::fromImage(mImage));
    setVisible(true);
}
void ImageWin::adjustSize( )
{
     mImageLabel->adjustSize();
}

void ImageWin::scaleImage(double factor)
{
    mImageLabel->resize(factor * mImageLabel->pixmap()->size());
    mHorzRuler->setRulerZoom(factor);
    mVertRuler->setRulerZoom(factor);

}
