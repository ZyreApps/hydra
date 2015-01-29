
#include <QApplication>
#include <QQmlApplicationEngine>

#include <QQmlContext>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("qmlContextHomeDirectory", QDir::homePath());
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    
    return app.exec();
}
