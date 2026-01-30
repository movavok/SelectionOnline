#include <QCoreApplication>
#include "server.h"

static QString argValue(const QStringList& arguments, const QString& key, const QString& defaultValue) {
    int index = arguments.indexOf(key);
    if (index != -1 && index + 1 < arguments.size())
        return arguments[index + 1];
    return defaultValue;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QStringList arguments = QCoreApplication::arguments();
    QString bindStr = argValue(arguments, "--bind", "0.0.0.0");
    QString portStr = argValue(arguments, "--port", "8081");

    QHostAddress bind(bindStr);
    unsigned short port = portStr.toUShort();

    Server server;
    if (!server.start(bind, port)) return 1;

    return app.exec();
}
