//Created by Nickolay Mardanov (aka NeoTheFox)
//2015
//This software is destributed on terms of GNU GPL v3 licence
//For details see "LICENCE"

#include <QTextStream>
#include <QProcess>
#include <QDebug>
#include <QElapsedTimer>
#include <X11/Xlib.h>

qint64 duration = 2;
qint64 accumulation = 12000;
QString command = "glxgears";

QTextStream& qStdOut()
{
    static QTextStream ts( stdout );
    return ts;
}

void printHelp()
{
    qStdOut() << "Shaky\n";
    qStdOut() << "by NeoTheFox, 2015\n";
    qStdOut() << "A window shake detector implementation\n";
    qStdOut() << "\n\n";
    qStdOut() << "USAGE:\n";
    qStdOut() << "-d    :   Duration of shake considered valid\n";
    qStdOut() << "-a    :   Accumulation of shake considered valid\n";
    qStdOut() << "-e    :   Path to executable to run upon shake\n";
    qStdOut() << "-h    :   Print this help\n";
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
        else if(arg->startsWith("-e"))
        {
            command = arg->split(' ')[1];
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
    qint64 time = 0;
    qint32 moved = 0;

    qint32 lastx = 0;
    qint32 lasty = 0;
    Window lastwindow;

    timer->start();

    forever
    {
        if(time >= duration)
        {
            time = 0;
            moved = 0;
        }
        else if(moved >= accumulation)
        {
            QProcess::startDetached(command);

            qDebug() << "Shake detected! Starting " << command << "\n";
            time = 0;
            moved = 0;
        }

        time += timer->restart();

        if(XCheckMaskEvent(display, -1, &event))
        {
            if(event.type == ConfigureNotify)
            {
                XConfigureEvent cevent = event.xconfigure;
                if(!(lastwindow == cevent.window))
                {
                    moved = 0;
                    time = 0;
                    lastx = cevent.x;
                    lasty = cevent.y;
                }

                moved += (abs(lastx - cevent.x) + abs(lasty - cevent.y));

                qDebug() << abs(lastx - cevent.x) << "x" << abs(lasty - cevent.y) << " :" << moved << " @" << time;

                lastx = cevent.x;
                lasty = cevent.y;
                lastwindow = cevent.window;
            }
        }
    }
}
