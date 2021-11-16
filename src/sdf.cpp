/* http://www.codersnotes.com/algorithms/signed-distance-fields */

#include <QImage>
#include <QTime>
#include <QDebug>
#include <math.h>

#define MULTIPLER 8

struct Point
{
    short dx, dy;
    int f;
};

struct Grid
{
    int w, h;
    Point *grid;
    Grid(int width, int height) : w(width), h(height)
    {
        grid = new Point[(w + 2) * (h + 2)];
    }
    ~Grid()
    {
        delete[] grid;
    }
};
static const Point pointInside = { 0, 0, 0 };
static const Point pointEmpty = { SHRT_MAX, SHRT_MAX, INT_MAX/2 };

static inline Point Get(Grid &g, int x, int y)
{
    return g.grid[y * (g.w + 2) + x];
}

static inline void Put(Grid &g, int x, int y, const Point &p)
{
    g.grid[y * (g.w + 2) + x] = p;
}

static inline void Compare(Grid &g, int x, int y, int offsetx, int offsety)
{
    int add;
    Point p = Get(g, x, y);
    Point other = Get(g, x + offsetx, y + offsety);
    if(offsety == 0) {
        add = 2 * other.dx + 1;
    }
    else if(offsetx == 0) {
        add = 2 * other.dy + 1;
    }
    else {
        add = 2 * (other.dy + other.dx + 1);
    }
    other.f += add;
    if (other.f < p.f)
    {
        p.f = other.f;
        if(offsety == 0) {
            p.dx = other.dx + 1;
            p.dy = other.dy;
        }
        else if(offsetx == 0) {
            p.dy = other.dy + 1;
            p.dx = other.dx;
        }
        else {
            p.dy = other.dy + 1;
            p.dx = other.dx + 1;
        }
    }
    Put(g, x, y, p);
}

static void GenerateSDF(Grid &g)
{
    /* forward scan */
    // #pragma omp parallel for
    for(int y = 1; y <= g.h; y++)
    {
        for(int x = 0; x <= g.w; x++)
        {
            Compare(g, x, y,  0, -1);
            Compare(g, x, y,  0, -1);
        }
        for(int x = 1; x <= g.w; x++)
        {
            Compare(g, x, y, -1,  0);
            Compare(g, x, y, -1, -1);
        }
        for(int x = g.w; x >= 0; x--)
        {
            Compare(g, x, y,  1,  0);
            Compare(g, x, y,  1, -1);
            x--;
            Compare(g, x, y,  1,  0);
            Compare(g, x, y,  1, -1);
        }
    }

    /* backward scan */
    // #pragma omp parallel for
    for(int y = g.h-1; y > 0; y--)
    {
        for(int x = 0; x <= g.w; x++)
        {
            Compare(g, x, y,  0,  1);
        }
        for(int x = 1; x <= g.w; x++)
        {
            Compare(g, x, y, -1,  0);
            Compare(g, x, y, -1,  1);
        }
        for(int x = g.w-1; x >= 0; x--)
        {
            Compare(g, x, y,  1,  0);
            Compare(g, x, y,  1,  1);
        }
    }
}



QImage dfcalculate(QImage &img, bool transparent = false)
{
    int w = img.width(), h = img.height();
    QImage result(w, h, QImage::Format_Indexed8);
    for(int i = 0; i < 256; i++)
    {
        result.setColor(i, qRgb(i,i,i));
    }
    Grid grid1(w, h);
    Grid grid2(w, h);
    /* create 1-pixel gap */
    QTime myTimer;
    myTimer.start();
    for(int x = 0; x < w + 2; x++)
    {
        //Put(grid1, x, 0, pointInside);
        Put(grid2, x, 0, pointEmpty);
        //Put(grid1, x, h + 1, pointInside);
        Put(grid2, x, h + 1, pointEmpty);
    }
    uchar *data = img.bits();
    uchar pixel = img.bytesPerLine() / w;
    #pragma omp parallel for
    for(int y = 1; y <= h; y++)
    {
        Put(grid1, 0, y, pointInside);
        //Put(grid2, 0, y, pointEmpty);
        // #pragma omp parallel for
        for(int x = 1; x <= w; x++)
        {
            //if(qGreen(img.pixel(x - 1, y - 1)) > 128)
            if(data[((y - 1) * w + (x - 1)) * pixel ] > 128)
            {
                //Put(grid1, x, y, pointEmpty);
                Put(grid2, x, y, pointInside);
            }
            else
            {
                //Put(grid1, x, y, pointInside);
                Put(grid2, x, y, pointEmpty);
            }
        }
        //Put(grid1, w + 1, y, pointInside);
        Put(grid2, w + 1, y, pointEmpty);
    }
    qDebug() << "Prepare:\t" << myTimer.elapsed() << "ms";
    GenerateSDF(grid2);
    myTimer.start();
    data  = result.bits();
    #pragma omp parallel for
    for(int y = 1; y <= h; y++)
    {
        quint8 linecache[w];
        for(int x = 1; x <= w; x++)
        {
            const double dist = sqrt((double)Get(grid2, x, y).f);
            quint8 c = dist * MULTIPLER;
            data[(y-1)*w + (x-1)] = c;
        }
    }
    qDebug() << "Write:\t\t" << myTimer.elapsed() << "ms";
    return result;
}
