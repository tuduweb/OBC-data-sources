#include "base/interface/StreamInterface.hpp"

#include <QSerialPort>
#include <QSerialPortInfo>

#include <QJsonObject>

#include <QByteArray>
#include <QMutex>

class SerialStream : public StreamInterface {
    Q_OBJECT

public:
    SerialStream();

    int StreamInit() override;
    int StreamStart() override;

    void writeData(const QByteArray& s);


    //bool _connect(void) override;

protected:
    void run() override;

    bool _hardwareConnect(const QJsonObject& config, QSerialPort::SerialPortError& error, QString& errorString);

    void Request(const QByteArray& request);

private slots:
    void linkError(QSerialPort::SerialPortError error);

signals:
    //三个信号量，数据返回，出现错误，超时错误
    void response(const QString& s);
    void error(const QString& s);
    void timeout(const QString& s);

protected:
    QSerialPort* m_serial;


    QMutex mutex;
    bool isQuit;
    QString portName;
    int waitTimeout;
    //data to sent
    QByteArray request;


};