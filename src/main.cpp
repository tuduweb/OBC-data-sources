#include <iostream>
#include <QApplication>

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    std::cout << "hello app" << std::endl;

    return app.exec();
}