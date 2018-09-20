#include <QCoreApplication>
#include "rpimbot.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Rpimbot m_rpimbot;
    m_rpimbot.init();

    return a.exec();
}
