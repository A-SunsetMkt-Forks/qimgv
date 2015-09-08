#include "thumbnaillabel.h"

ThumbnailLabel::ThumbnailLabel(QGraphicsPixmapItem *parent) :
    QGraphicsLayoutItem(),
    QGraphicsPixmapItem(parent),
    hovered(false),
    loaded(false),
    showLabel(false),
    state(EMPTY),
    highlighted(false),
    borderW(3),
    borderH(5)
{
    thumbnailSize = globalSettings->s.value("thumbnailSize", "120").toInt();
    setGraphicsItem(this);
    this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    this->setOffset(QPointF(borderW, borderH));
    this->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);

    labelRect = QRectF(QPointF(borderW, borderH),
                       QPointF(borderW+24, borderH+16));
    highlightRect.setTopLeft(QPointF(borderW, 0));
    highlightRect.setBottomRight(QPointF(borderW+thumbnailSize, borderH));
    shadowRect.setTopLeft(QPointF(borderW, borderH));
    shadowRect.setBottomRight(QPointF(borderW+thumbnailSize, borderH+SHADOW_HEIGHT));
    highlightColor = new QColor(48, 140, 171);
    highlightColorBorder = new QColor(20, 26, 17); //unused for now
    outlineColor = new QColor(Qt::black);
    shadowGradient = new QLinearGradient(shadowRect.topLeft(),
                                         shadowRect.bottomLeft());
    shadowGradient->setColorAt(0, QColor(0,0,0,200));
    shadowGradient->setColorAt(1, QColor(0,0,0,0));
    font.setPixelSize(11);
}

void ThumbnailLabel::setThumbnail(const Thumbnail *_thumbnail){
    thumbnail = _thumbnail;
    QGraphicsPixmapItem::setPixmap(*thumbnail->image);
    loaded = true;
    if(!thumbnail->name.isEmpty()) {
        showLabel = true;
    }
}

void ThumbnailLabel::setHighlighted(bool x) {
    bool toRepaint = (highlighted==x);
    highlighted = x;
    if(toRepaint) {
        this->update(boundingRect());
    }
}

bool ThumbnailLabel::isHighlighted() {
    return highlighted;
}

void ThumbnailLabel::setOpacityAnimated(qreal amount, int speed) {
    if(speed == 0.0) {
        setOpacity(amount);
        return;
    }
    QPropertyAnimation *anim = new QPropertyAnimation(this, "opacity");
    anim->setDuration(speed);
    anim->setStartValue(this->opacity());
    anim->setEndValue(amount);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
void ThumbnailLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    QGraphicsPixmapItem::paint(painter,option,widget);
    if(isHighlighted()) {
        painter->fillRect(highlightRect, *highlightColor);
        painter->fillRect(shadowRect, *shadowGradient);
    }
    if(showLabel) {
        painter->setFont(font);
        QFontMetrics fm(font);
        int textWidth = fm.width(thumbnail->name);
        labelRect.setWidth(textWidth + 4);
        painter->fillRect(labelRect, *highlightColor);
        QPointF textPos = labelRect.bottomLeft()+QPointF(2,-5);
        painter->setPen(QColor(10,10,10,200));
        painter->drawText(textPos+QPointF(1,1), thumbnail->name);
        painter->setPen(QColor(255,255,255,255));
        painter->drawText(textPos, thumbnail->name);
    }
}

QSizeF ThumbnailLabel::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    switch (which) {
    case Qt::MinimumSize:
    case Qt::PreferredSize:
        return QSize(thumbnailSize, thumbnailSize)
               + QSize(borderW*2, borderH*2);
    case Qt::MaximumSize:
        return QSize(thumbnailSize, thumbnailSize)
               + QSize(borderW*2, borderH*2);
    default:
        break;
    }
    return constraint;
}

void ThumbnailLabel::setGeometry(const QRectF &rect) {
    prepareGeometryChange();
    QGraphicsLayoutItem::setGeometry(rect);
    setPos(rect.topLeft());
}

QRectF ThumbnailLabel::boundingRect() const
{
    return QRectF(QPointF(0,0), geometry().size());
}

ThumbnailLabel::~ThumbnailLabel() {

}

