#pragma once
#include <ossia/network/base/parameter.hpp>
#include <ossia/network/osc/detail/bundle.hpp>
#include <ossia/network/osc/detail/osc_packet_processor.hpp>
#include <ossia/network/osc/detail/osc_value_write_visitor.hpp>
#include <ossia/network/osc/detail/bidir.hpp>
#include <ossia/network/osc/detail/message_generator.hpp>
#include <ossia/network/osc/detail/osc_receive.hpp>
#include <ossia/network/osc/detail/osc_1_0_policy.hpp>

namespace ossia::net
{

template<typename OscVersion>
struct osc_protocol_common
{
  template<typename T>
  static void init(T& self)
  {
    self.from_client.open();
    self.to_client.connect();

    self.from_client.receive(
          [&self] (const char* data, std::size_t sz) {
        auto on_message = [&self] (auto&& msg) { self.on_received_message(msg); };
        osc_packet_processor<decltype(on_message)>{on_message}({data, sz});
      }
    );
  }

  template<typename T, typename Value_T>
  static bool push(T& self, const ossia::net::parameter_base& addr, Value_T&& v)
  {
    auto val = filter_value(addr, std::forward<Value_T>(v));
    if (val.valid())
    {
      using send_visitor = osc_value_send_visitor<ossia::net::parameter_base, OscVersion, typename T::socket_type>;
      send_visitor vis{addr, addr.get_node().osc_address(), self.to_client};
      val.apply(vis);
      return true;
    }
    return false;
  }

  template<typename T>
  static bool push_raw(T& self, const ossia::net::full_parameter_data& addr)
  {
    auto val = filter_value(addr, addr.value());
    if (val.valid())
    {
      using send_visitor = osc_value_send_visitor<ossia::net::full_parameter_data, OscVersion, typename T::socket_type>;
      val.apply(send_visitor{addr, addr.address, self.to_client});
      return true;
    }
    return false;
  }

  template<typename T>
  static bool echo_incoming_message(
      T& self,
      const message_origin_identifier& id,
      const parameter_base& addr,
      const value& val)
  {
    if(&id.protocol == &self)
      return true;

    using send_visitor = osc_value_send_visitor<ossia::net::parameter_base, OscVersion, typename T::socket_type>;
    val.apply(send_visitor{addr, addr.get_node().osc_address(), self.to_client});
    return true;
  }

  template<typename T>
  static bool observe(T& self, ossia::net::parameter_base& address, bool enable)
  {
    if (enable)
      self.m_listening.insert(
          std::make_pair(address.get_node().osc_address(), &address));
    else
      self.m_listening.erase(address.get_node().osc_address());

    return true;
  }

  template<typename T>
  static void on_received_message(T& self, const oscpack::ReceivedMessage& m)
  {
    if (!self.learning())
    {
      ossia::net::on_input_message<false>(
            m.AddressPattern(),
            ossia::net::osc_message_applier{self.m_id, m},
            self.m_listening, *self.m_device, self.m_logger);
    }
    else
    {
      ossia::net::osc_learn(&self.m_device->get_root_node(), m);
    }
  }
};

// Client can't push to GET addresses
template<typename OscVersion>
struct osc_protocol_client : osc_protocol_common<OscVersion>
{
  template<typename T, typename Val_T>
  static bool push(T& self, const ossia::net::parameter_base& addr, Val_T&& v)
  {
    if (addr.get_access() == ossia::access_mode::GET)
      return false;

    return osc_protocol_common<OscVersion>::push(self, addr, std::forward<Val_T>(v));
  }

  template<typename T>
  static bool push_raw(T& self, const ossia::net::full_parameter_data& addr)
  {
    if (addr.get_access() == ossia::access_mode::GET)
      return false;

    return osc_protocol_common<OscVersion>::push_raw(self, addr);
  }

  template<typename T, typename Impl, typename Addresses>
  static bool push_bundle(T& self, Impl& socket, const Addresses& addresses)
  {
    if(auto bundle = make_bundle(bundle_client_policy<OscVersion>{}, addresses)) {
      socket.write(bundle->data.data(), bundle->data.size());
      ossia::buffer_pool::instance().release(std::move(bundle->data));
      return true;
    }
    return false;
  }
};

// Servers can push to GET addresses
template<typename OscVersion>
struct osc_protocol_server : osc_protocol_common<OscVersion>
{
  template<typename T, typename Val_T>
  static bool push(T& self, const ossia::net::parameter_base& addr, Val_T&& v)
  {
    return osc_protocol_common<OscVersion>::push(self, addr, std::forward<Val_T>(v));
  }

  template<typename T>
  static bool push_raw(T& self, const ossia::net::full_parameter_data& addr)
  {
    return osc_protocol_common<OscVersion>::push_raw(self, addr);
  }

  template<typename T, typename Impl, typename Addresses>
  static bool push_bundle(T& self, Impl& socket, const Addresses& addresses)
  {
    if(auto bundle = make_bundle(bundle_server_policy<OscVersion>{}, addresses)) {
      socket.write(bundle->data.data(), bundle->data.size());
      ossia::buffer_pool::instance().release(std::move(bundle->data));
      return true;
    }
    return false;
  }
};

}
