#include "mainwindow.h"

#include <QApplication>
#include <QTextCodec>
#include <QDebug>

int main(int argc, char *argv[])
{
    QTextCodec* utf8 = QTextCodec::codecForName("utf8");
    QTextCodec::setCodecForLocale(utf8);

    QApplication a(argc, argv);
    MainWindow w;
    g_MainWindow_ = &w;
    w.show();
    auto ret = a.exec();

    return ret;
}
