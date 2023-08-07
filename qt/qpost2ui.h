#pragma once

#include <QCoreApplication>
#include <QEvent>

template <typename F>
void qPost2Ui(F f)
{
    class Event : public QEvent
    {
        using Fn = std::decay_t<F>;

    public:
        explicit Event(Fn fn)
            : QEvent{QEvent::None}, fn_{fn}
        {
        }

        ~Event() override
        {
            fn_();
        }

    private:
        Fn fn_;
    };

    QCoreApplication::postEvent(qApp, new Event{std::forward<F>(f)});
}

//#include <QThread>

//#include "qpost2ui.h"

//int main(int argc, char *argv[])
//{
//    QCoreApplication a(argc, argv);
//    qDebug() << QThread::currentThreadId() << " main";
//    std::async([](){
//        qDebug() << QThread::currentThreadId() << " async";
//        qPost2Ui([](){
//            qDebug() << QThread::currentThreadId() << " lambda";
//        });

//    });
//    return a.exec();
//}

// Result:
// 0x1e5f3de00  main
// 0x16b0fb000  async
// 0x1e5f3de00  lambda
