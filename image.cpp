#include "image.h"
#include <time.h>


//use this one
Image::Image(QString _path) : QObject()
{
    path = _path;
    inUseFlag = false;
   // image = NULL;
  //  movie = NULL;
    loaded = false;
    movie = new QMovie();
    image = new QImage();
    type = NONE;
    extension = NULL;
}

Image::~Image()
{
    delete image;
    delete movie;
}

void Image::detectType() {
    QFile file(path);
    file.open(QIODevice::ReadOnly);
    //read first 2 bytes to determine file format
    QByteArray startingBytes= file.read(2).toHex();
    file.close();

    if(startingBytes=="4749") {
        type=GIF;
        extension="GIF";
    }
    else if(startingBytes=="ffd8") {
        type=STATIC;
        extension="JPG";
    }
    else if(startingBytes=="8950") {
        type=STATIC;
        extension="PNG";
    }
    else if(startingBytes=="424d") {
        type=STATIC;
        extension="BMP";
    }
    else type = STATIC;
}

//load image data from disk
void Image::loadImage()
{

    mutex.lock();
    if(isLoaded()) {
        //qDebug() << "already loaded: " << path;
        mutex.unlock();
        return;
    }
    detectType();
    if(type!=NONE)  {
        if(getType() == GIF) {
            movie->setFormat("GIF");
            movie->setFileName(path);
            movie->jumpToFrame(0);
        }
        else if(getType() == STATIC) {
            if(extension) {
                image = new QImage(path, extension);
            }
            else {
                image = new QImage(path); // qt will guess format
            }
        }
        loaded = true;
    }
    generateThumbnail();
    mutex.unlock();
}

void Image::unloadImage() {
    mutex.lock();
    if(isLoaded()) {
        delete movie;
        delete image;
        movie = new QMovie();
        image = new QImage();
        loaded = false;
    }
    mutex.unlock();
}

FileInfo *Image::getInfo() {
    return info;
}

void Image::attachInfo(FileInfo *_info) {
    info = _info;
}

void Image::save(QString destinationPath) {
    if(type == STATIC && isLoaded()) {
        image->save(destinationPath, extension, 100);
    }
}

void Image::save() {
    if(type == STATIC && isLoaded()) {
        image->save(path, extension, 100);
    }
}

QImage* Image::generateThumbnail() {
    int size = globalSettings->s.value("thumbnailSize", 100).toInt();
    QImage *thumbnail = new QImage();
    if(!isLoaded()) {
        QImage *tmp = new QImage(path);
        *thumbnail = tmp->scaled(size*2, size*2, Qt::KeepAspectRatio, Qt::FastTransformation)
            .scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        delete tmp;
    } else if(getType() == GIF) {
        *thumbnail = movie->currentImage()
                .scaled(size*2, size*2, Qt::KeepAspectRatio)
                .scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else if(getType() == STATIC) {
        if(!image->isNull()) {
            *thumbnail = image->scaled(size*2, size*2, Qt::KeepAspectRatio, Qt::FastTransformation)
                .scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }
    return thumbnail;
}

bool Image::isLoaded() {
    return loaded;
}

int Image::ramSize() {
    if(getType() == GIF) { // maybe wrong
        return movie->frameCount()*movie->currentImage().byteCount()/(1024*1024);
    }
    else if(getType() == STATIC) {
        return image->byteCount()/(1024*1024);
    }
    else return 0;
}

QMovie* Image::getMovie()
{
    return movie;
}

const QImage* Image::getImage()
{
    return const_cast<const QImage*>(image);
}

int Image::height() {
    if(isLoaded()) {
        if(type == GIF) {
            return movie->currentImage().height();
        }
        if(type == STATIC) {
            return image->height();
        }
    }
    return 0;
}

int Image::width() {
    if(isLoaded()) {
        if(type == GIF) {
            return movie->currentImage().width();
        }
        if(type == STATIC) {
            return image->width();
        }
    }
    return 0;
}

QSize Image::size() {
    if(isLoaded()) {
        if(type == GIF) {
            return movie->currentImage().size();
        }
        if(type == STATIC) {
            return image->size();
        }
    }
    return QSize(0,0);
}

int Image::getType()
{
    return type;
}

QString Image::getPath()
{
    return path;
}

/*
bool Image::compare(Image* another) {
    if(getFileName() == another->getFileName() &&
       info.getLastModifiedDate() == another->getFileInfo().getLastModifiedDate() ) {
        return true;
    }
    return false;
}
*/

QImage* Image::rotated(int grad) {
    if(isLoaded()) {
        if(type==STATIC) {
            QImage *img = new QImage();
            QTransform transform;
            transform.rotate(grad);
            *img = image->transformed(transform, Qt::SmoothTransformation);
            return img;
        }
    }
    else return NULL;
}

void Image::rotate(int grad) {
    if(isLoaded()) {
        if(type==STATIC) {
            mutex.lock();
            QImage *img = rotated(grad);
            delete image;
            image = img;
            mutex.unlock();
        }
    }
}

void Image::crop(QRect newRect) {
    mutex.lock();
    if(isLoaded()) {
        if(getType() == STATIC) {
            QImage *tmp = new QImage(newRect.size(), QImage::Format_ARGB32_Premultiplied);
            *tmp = image->copy(newRect);
            delete image;
            image = tmp;
        }
        if(getType() == GIF) {
            // someday later
        }
    }
    mutex.unlock();
}
