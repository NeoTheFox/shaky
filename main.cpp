#include <QThread>
#include <QTextStream>
#include <QElapsedTimer>
#include <X11/Xlib.h>

qint64 duration = 3000000000;
qint64 accumulation = 10000;

QTextStream& qStdOut()
{
    static QTextStream ts( stdout );
    return ts;
}

void printHelp()
{
    qStdOut() << "Shaky";
    return;
}

int main(int argc, char *argv[])
{
    for(qint32 i = 0; i < argc; i++)
    {
        QString *arg = new QString(argv[i]);
        if(arg->startsWith("-d"))
        {
            duration = arg->split(' ')[1].toInt();
        }
        else if(arg->startsWith("-a"))
        {
            accumulation = arg->split(' ')[1].toInt();
        }
        else if(arg->startsWith("-h"))
        {
            printHelp();
            return 0;
        }
    }

    Display *display;
    display = XOpenDisplay(NULL);
    if(!display)
    {
        qFatal("Cant open display");
        return -1;
    }
    else XSelectInput(display, XDefaultRootWindow(display), SubstructureNotifyMask );

    QElapsedTimer *timer = new QElapsedTimer();

    XEvent event;
    qint32 time = 0;
    qint32 moved = 0;

    qint32 lastx = 0;
    qint32 lasty = 0;
    Window *lastwindow;

    timer->start();

    forever
    {
        if(time > duration)
        {
            time = 0;
            moved = 0;
        }
        else if(moved > accumulation)
        {
            qStdOut() << "Collapse all\n";
            time = 0;
            moved = 0;
        }
        time += timer->restart();
        if(XCheckMaskEvent(display, -1, &event))
        {
            if(event.type == ConfigureNotify)
            {
                XConfigureEvent *cevent = &event.xconfigure;
                if(!(lastwindow == &cevent->window))
                {
                    moved = 0;
                    time = 0;
                }
                moved += (abs(lastx - cevent->x) + abs(lasty - cevent->y));

                lastx = cevent->x;
                lasty = cevent->y;
                lastwindow = &cevent->window;
            }
        }
    }
}
