#pragma once

#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QImage>
class FontView : public QWidget
{
Q_OBJECT
public:
    FontView(QWidget *parent);
protected:
    void paintEvent(QPaintEvent *event);
private:
    QPixmap texture;
    int scale;
private slots:
    void updatePixmap(const QImage &image);
    void rescale(int);
};
