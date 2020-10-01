// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <ossia-max/src/address_router.hpp>
#include <ossia/network/common/websocket_log_sink.hpp>
#include <ossia-max/src/ossia-max.hpp>
#include <ossia/detail/thread.hpp>

#include <boost/algorithm/string/predicate.hpp>

using namespace ossia::max;

#pragma mark -
#pragma mark ossia_address_router class methods

extern "C" void ossia_address_router_setup()
{
  auto& ossia_library = ossia_max::instance();

  // instantiate the ossia.address_router class
  ossia_library.ossia_address_router_class = class_new(
      "ossia.address_router", (method)ossia_address_router_new, (method)address_router::free,
      (long)sizeof(address_router), 0L, A_GIMME, 0);

  auto& c = ossia_library.ossia_address_router_class;
  class_addmethod(
      c, (method)address_router::in_anything,
      "anything", A_GIMME, 0);
  class_addmethod(
      c, (method)address_router::assist,
      "assist", A_CANT, 0);

  class_register(CLASS_BOX, ossia_library.ossia_address_router_class);
}

extern "C" void* ossia_address_router_new(t_symbol* s, long argc, t_atom* argv)
{
  auto x = make_ossia<address_router>(argc, argv);

  x->m_patterns.reserve(argc);
  x->m_outlets.reserve(argc+1);

  // extra outlet for non matching addresses
  x->m_outlets.push_back(outlet_new(x, nullptr));

  while(argc--)
  {
    if(argv[argc].a_type == A_SYM)
    {
      std::string pattern(argv[argc].a_w.w_sym->s_name);
      if(pattern.back() != '/')
        pattern += '/';

      x->m_patterns.push_back(pattern);
      x->m_outlets.push_back(outlet_new(x, nullptr));
    }
    else
    {
      object_error(&x->m_object, "wrong arg type, should be symbol");
    }
  }

  return x;
}

void address_router::in_anything(address_router* x, t_symbol* s, long argc, t_atom* argv)
{
  std::string address(s->s_name);

  bool match = false;
  for(int i = 0 ; i < x->m_patterns.size(); i++)
  {
    std::string pattern = x->m_patterns[i];
    if(boost::algorithm::starts_with(address, pattern))
    {
      std::string sub = address.substr(pattern.size());
      outlet_anything(x->m_outlets[i+1], gensym(sub.c_str()), argc, argv);
      match = true;
    }
  }

  if(!match)
    outlet_anything(x->m_outlets[0], s, argc, argv);
}

void address_router::free(address_router* x)
{
  if (x)
  {
    x->~address_router();
  }
}

void address_router::assist(address_router *x, void *b, long m, long a, char *s)
{
  if (m == ASSIST_INLET)
  {
    sprintf(s, "Address input");
  }
  else if(m == ASSIST_OUTLET)
  {
    sprintf(s, "Outlet number %ld", a);
  }
}

#pragma mark -
#pragma mark t_address_router structure functions

address_router::address_router(long argc, t_atom *argv)
{
}