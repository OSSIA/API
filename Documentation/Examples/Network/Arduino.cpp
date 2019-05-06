#include <ossia-qt/serial/serial_protocol.hpp>
#include <ossia/network/generic/generic_device.hpp>
#include <ossia/context.hpp>
#include <QSerialPortInfo>
#include <QCoreApplication>

int main(int argc, char** argv)
{
  QByteArray code = R"___(import QtQuick 2.0
import Ossia 1.0 as Ossia

Ossia.Serial
{
  function openListening(address) {}
  function closeListening(address) {}
  function onMessage(message, raw) { // evaluated each time a message is received
    console.log(message)
    return [];
  }

  function createTree() {
    return [];
  }
})___";

  ossia::context ctx;

  QCoreApplication app(argc, argv);
  QSerialPortInfo serialport("ttyACM1");

  for(auto p : serialport.availablePorts())
    qDebug() << p.portName();

  qDebug() << serialport.portName() << serialport.manufacturer();
  ossia::net::generic_device dev{
    std::make_unique<ossia::net::serial_protocol>(code, serialport),
    "arduino"};
  app.exec();
}
