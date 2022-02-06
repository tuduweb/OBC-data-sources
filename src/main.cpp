#include <iostream>
#include <QApplication>

#include <QSerialPort>
#include <QSerialPortInfo>

#include "qdebug.h"

#include "sources/serial/SerialStream.hpp"

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    std::cout << "hello app" << std::endl;

    SerialStream* stream = new SerialStream();

    stream->StreamInit();
    stream->StreamStart();

    return app.exec();
}