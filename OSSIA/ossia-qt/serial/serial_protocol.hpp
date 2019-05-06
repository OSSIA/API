#pragma once
#include <ossia-qt/js_utilities.hpp>
#include <ossia/detail/logger.hpp>
#include <ossia/network/base/protocol.hpp>
#include <ossia/network/generic/wrapped_parameter.hpp>
#include <wobjectdefs.h>
#include <QObject>
#include <QSerialPort>

class QQmlEngine;
class QQmlComponent;

namespace ossia
{
namespace net
{
struct serial_parameter_data_base
{
  serial_parameter_data_base() = default;
  serial_parameter_data_base(const serial_parameter_data_base&) = default;
  serial_parameter_data_base(serial_parameter_data_base&&) = default;
  serial_parameter_data_base& operator=(const serial_parameter_data_base&)
      = default;
  serial_parameter_data_base& operator=(serial_parameter_data_base&&) = default;
  serial_parameter_data_base(const QJSValue& val)
  {
    auto r = val.property("request");
    if (r.isString())
    {
      request = r.toString();
    }
    else
    {
      request = val.property("name").toString();
    }
  }

  QString request;
};

struct serial_parameter_data final
    : public parameter_data
    , public serial_parameter_data_base
{
  using base_data_type = serial_parameter_data_base;
  serial_parameter_data() = default;
  serial_parameter_data(const serial_parameter_data&) = default;
  serial_parameter_data(serial_parameter_data&&) = default;
  serial_parameter_data& operator=(const serial_parameter_data&) = default;
  serial_parameter_data& operator=(serial_parameter_data&&) = default;

  serial_parameter_data(const std::string& name) : parameter_data{name}
  {
  }

  serial_parameter_data(const QJSValue& val)
      : parameter_data{ossia::qt::make_parameter_data(val)}
      , serial_parameter_data_base{val}
  {
  }

  bool valid() const noexcept { return !request.isEmpty() || type; }
};
class OSSIA_EXPORT serial_wrapper final
    : public QObject
{
  W_OBJECT(serial_wrapper)

public:
  serial_wrapper(const QSerialPortInfo& port) noexcept
    : m_port{port}
  {
    bool ok = m_port.open(QIODevice::ReadWrite);
    if(!ok)
    {
      ossia::logger().info(
            "Serial port error: {}", m_port.errorString().toStdString());
    }
    connect(
        this, &serial_wrapper::write, this, &serial_wrapper::on_write,
        Qt::QueuedConnection);

    connect(
        &m_port, &QSerialPort::readyRead, this, &serial_wrapper::on_read,
        Qt::QueuedConnection);
  }

  ~serial_wrapper() noexcept;

  void write(QByteArray arg_1) E_SIGNAL(OSSIA_EXPORT, write, arg_1);
  void read(QString txt, QByteArray raw) E_SIGNAL(OSSIA_EXPORT, read, txt, raw);

  void on_write(QByteArray b) noexcept
  {
    m_port.write(b);
  } W_SLOT(on_write)

  void on_read()
  {
    QByteArray arr = m_port.readAll();
    QString str = QString::fromLatin1(arr);
    read(str, arr);
  } W_SLOT(on_read)

private:
  QSerialPort m_port;

};


using serial_parameter = wrapped_parameter<serial_parameter_data>;
using serial_node = ossia::net::wrapped_node<serial_parameter_data, serial_parameter>;
class OSSIA_EXPORT serial_protocol final
    : public QObject
    , public ossia::net::protocol_base
{
public:
  // Param : the name of the serial port
  serial_protocol(const QByteArray& code, const QSerialPortInfo& bot);
  ~serial_protocol() override;

  bool pull(ossia::net::parameter_base&) override;
  bool push(const ossia::net::parameter_base&, const ossia::value& v) override;
  bool push_raw(const ossia::net::full_parameter_data& parameter_base) override;
  bool observe(ossia::net::parameter_base&, bool) override;
  bool update(ossia::net::node_base& node_base) override;

  void set_device(ossia::net::device_base& dev) override;

  static serial_parameter_data read_data(const QJSValue& js)
  {
    return js;
  }

private:
  void on_read(const QString& txt, const QByteArray&);
  QQmlEngine* m_engine{};
  QQmlComponent* m_component{};

  ossia::net::device_base* m_device{};
  QObject* m_object{};
  mutable serial_wrapper m_serialPort;
  QByteArray m_code;
};
using serial_device = ossia::net::wrapped_device<serial_node, serial_protocol>;

class OSSIA_EXPORT Serial : public QObject {
};

}
}
