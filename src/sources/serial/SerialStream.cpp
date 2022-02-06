#include "SerialStream.hpp"

#include <iostream>

#include <qDebug>

#include <QEventLoop>


SerialStream::SerialStream() : m_serial(nullptr), waitTimeout(20) {

    std::cout << "hello Serial Stream" << std::endl;

    QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();

    for(const auto& item : infos) {
        std::cout << "************************************" << std::endl;
        qDebug() << item.portName();
        qDebug() << item.manufacturer();
        qDebug() << item.serialNumber();
        qDebug() << item.manufacturer();
        qDebug() << item.description();
        qDebug() << item.systemLocation();
    }


}

int SerialStream::StreamInit() {
    return 0;
}

int SerialStream::StreamStart() {
    start();
    return 0;
}

void SerialStream::run() {

	//RemoteLog(QString("创建串口连接中...\n端口:%1 波特率:%2").arg(_serialConfig->portName()).arg(_serialConfig->baud()));


    if (m_serial != nullptr) {
		qWarning() << "connect called while already connected";
        return;
    }

    QSerialPort::SerialPortError    terror;
	QString                         errorString;


	// m_serial->setDataBits(static_cast<QSerialPort::DataBits>     (config["dataBits"].toInt()));
	// m_serial->setFlowControl(static_cast<QSerialPort::FlowControl>  (config["flowControl"].toInt()));
	// m_serial->setStopBits(static_cast<QSerialPort::StopBits>     (config["stopBits"].toInt()));
	// m_serial->setParity(static_cast<QSerialPort::Parity>       (config["parity"].toInt()));

	QJsonObject cnf;
	cnf.insert("portName", "tty.usbmodem0001A00000011");
	cnf.insert("dataBits", 8);
	cnf.insert("stopBits", 1);
	cnf.insert("parity", 0);
	cnf.insert("flowControl", 0);
	cnf.insert("baudrate", 115200);

    if (!_hardwareConnect(cnf, terror, errorString)) {

        if (terror == QSerialPort::PermissionError) {
            // Device already open, ignore and fail connect
            return;
        }

        qDebug() << tr("Error connecting: Could not create port. %1").arg(errorString);
        //error(tr("Error connecting: Could not create port. %1").arg(errorString));


		return;
    
    }

	// loop
	////bool isQuit = false;
	QString currentRequset;
	int currentWaitTimeout;

	while (!isQuit)
	{
		mutex.lock();
		currentRequset = request;
		currentWaitTimeout = waitTimeout;
		mutex.unlock();
		mutex.lock();

		if (!request.isEmpty()) {
			// 写请求
			//QByteArray requestData = currentRequset.toLocal8Bit();
			m_serial->write(request);
			if (m_serial->waitForBytesWritten(currentWaitTimeout))
			{
				//request.clear();
				//成功写入
			} else {
				//写请求超时
				QString log = tr("写入操作超时： %1")
					.arg(QTime::currentTime().toString());
				//emit timeout(log);
				//emit OnRemoteLogAvailable(log);
			}
			request.clear();
		}
		mutex.unlock();

		mutex.lock();
		// 读取返回
		if (m_serial->isReadable()) {

			QByteArray responseData = m_serial->readAll();
			//maybe wait x seconds
			while (m_serial->waitForReadyRead(currentWaitTimeout))
				responseData += m_serial->readAll();
		
			if (responseData.size() > 0)
			{
				//QString response(responseData);
				////SIGNALS
				emit DataReceived(responseData);

			}

		}
	}

	m_serial->close();


}

bool SerialStream::_hardwareConnect(const QJsonObject& config, QSerialPort::SerialPortError& error, QString& errorString) {

	if (m_serial) {
		//qCDebug(SerialLinkLog) << "SerialLink:" << QString::number((qulonglong)this, 16) << "closing port";
		m_serial->close();
		//暂停的同时运行主线程循环,防止程序假死
		// Wait 50 ms while continuing to run the event queue
		for (unsigned i = 0; i < 10; i++) {
			//QGC::SLEEP::usleep(5000);
			//qgcApp()->processEvents(QEventLoop::ExcludeUserInputEvents);
		}
		delete m_serial;
		m_serial = nullptr;
    }
    //objectname
    m_serial = new QSerialPort(config["portName"].toString());

    qRegisterMetaType<QSerialPort::SerialPortError>("SerialStream");
	QObject::connect(m_serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error), this, &SerialStream::linkError);
	//connect(serialPort, SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this, SLOT(linkError(QSerialPort::SerialPortError)));

	// Try to open the port three times
	for (int openRetries = 0; openRetries < 3; openRetries++) {
		if (!m_serial->open(QIODevice::ReadWrite)) {
			//qCDebug(SerialLinkLog) << "Port open failed, retrying";
			//-//RemoteLog(tr("Port open failed, retrying %1").arg(openRetries));
			// Wait 250 ms while continuing to run the event loop
			for (unsigned i = 0; i < 50; i++) {
				QThread::msleep(5);
				//qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
			}
			//qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
		}
		else {
			break;
		}
	}

    if (!m_serial->isOpen()) {
		qDebug() << "open failed" << m_serial->errorString() << m_serial->error();//; << m_serial->;// << "autconnect:" << _config->isAutoConnect();

		//RemoteLog(QString("端口打开失败:%1 错误代码:%2").arg(m_serial->errorString()).arg(m_serial->error()));

		error = m_serial->error();
		errorString = m_serial->errorString();
		m_serial->close();
		delete m_serial;
		m_serial = nullptr;
		return false; // couldn't open serial port
	}

	m_serial->setDataTerminalReady(true);

    

	m_serial->setBaudRate(config["baudrate"].toInt());
	m_serial->setDataBits(static_cast<QSerialPort::DataBits>     (config["dataBits"].toInt()));
	m_serial->setFlowControl(static_cast<QSerialPort::FlowControl>  (config["flowControl"].toInt()));
	m_serial->setStopBits(static_cast<QSerialPort::StopBits>     (config["stopBits"].toInt()));
	m_serial->setParity(static_cast<QSerialPort::Parity>       (config["parity"].toInt()));

	// emit connected();

	// //绑定信号,给其它类提供跨线程发送能力(暂时放在这个位置)
	// connect(this, &SerialThreadLink::SendBytes, this, &SerialThreadLink::_writeBytes);

	// _serialIsConnected = true;

	// qDebug() << "Connection SeriaLink: " << "with settings" << _serialConfig->portName()
	// 	<< _serialConfig->baud() << _serialConfig->dataBits() << _serialConfig->parity() << _serialConfig->stopBits();

	// RemoteLog(QString("串口连接成功!\n端口:%1 波特率:%2").arg(_serialConfig->portName()).arg(_serialConfig->baud()));

	// return true; // successful connection
	return true;
}


void SerialStream::linkError(QSerialPort::SerialPortError error) {

	switch (error) {
	case QSerialPort::NoError:
		break;
	case QSerialPort::ResourceError:
		// This indicates the hardware was pulled from the computer. For example usb cable unplugged.
		//_connectionRemoved();
		qDebug() << error << m_serial->errorString();
		break;
	default:
		qDebug() << error;// << m_serial->errorString();
		// You can use the following qDebug output as needed during development. Make sure to comment it back out
		// when you are done. The reason for this is that this signal is very noisy. For example if you try to
		// connect to a PixHawk before it is ready to accept the connection it will output a continuous stream
		// of errors until the Pixhawk responds.
		//qCDebug(SerialLinkLog) << "SerialLink::linkError" << error;
		break;
	}

};

void SerialStream::Request(const QByteArray& request) {

	QMutexLocker m(&mutex);
	if (isRunning())
		this->request = request;
	else
		emit error("not running");

}

void SerialStream::writeData(const QByteArray& s)
{
	if (this->isRunning())
		Request(s);
	else
		emit error("not running");
}