#include <ui/TexSyn.h>
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TexSyn w;
    w.show();
    return a.exec();
}
