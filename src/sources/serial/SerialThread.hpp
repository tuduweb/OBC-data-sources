#pragma once
#include <QThread>

class SerialThread : public QThread{
    Q_OBJECT
public:
    SerialThread(QObject* parent = nullptr) : QThread(parent) {};
    
};