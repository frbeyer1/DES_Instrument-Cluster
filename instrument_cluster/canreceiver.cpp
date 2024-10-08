#include "canreceiver.h"
#include "canbusexception.h"
#include <QDebug>

CANReceiver::CANReceiver(QObject *parent) : QObject(parent), canDevice(nullptr) {}

CANReceiver::~CANReceiver()
{
    disconnectFromBus();
}


/**
 * @brief Connect to CAN Bus using specified interface name
 *
 * @param interfaceName
 */
void CANReceiver::connectToBus(const QString &interfaceName)
{
    if (canDevice)
        throw CanBusException("Already connected to CAN bus. " + canDevice->errorString().toStdString());

    canDevice = QCanBus::instance()->createDevice("socketcan", interfaceName);

    if (!canDevice)
        throw CanBusException("Failed to create CAN device for interface:" + interfaceName.toStdString()
                                  + "\n" + canDevice->errorString().toStdString());

    if (!canDevice->connectDevice())
    {
        delete canDevice;
        canDevice = nullptr;
        throw CanBusException("Failed to connect to CAN device. " + canDevice->errorString().toStdString());
    }

    qDebug() << "canDevice->busStatus : " << canDevice->busStatus();

    connect(canDevice, &QCanBusDevice::framesReceived, this, &CANReceiver::handleNewData);
}


/**
 * @brief Disconnect with CAN bus device
 */
void CANReceiver::disconnectFromBus()
{
    if (canDevice)
    {
        canDevice->disconnectDevice();
        delete canDevice;
        canDevice = nullptr;
    }
}


/**
 * @brief Emit signal if new CAN message arrived
 */
void CANReceiver::handleNewData()
{
    while (canDevice->framesAvailable())
    {
        const QCanBusFrame frame = canDevice->readFrame();
        emit newMessageReceived(frame);
    }
}
