#include "BasicMinuit.h"

namespace impl
{
Minuit2::Minuit2(std::string ip, int in_port, int out_port) :
mIp(ip),
mInPort(in_port),
mOutPort(out_port)
{
  // create a Minuit protocol unit
  /* example of a basic logger for debugging purposes
  mLogger = std::make_shared<NetworkLogger>();
  mLogger->setInboundLogCallback([] (std::string str) { std::cerr << str << std::endl; });
  mLogger->setOutboundLogCallback([] (std::string str) { std::cerr << str << std::endl; });
  */
}

Minuit2::~Minuit2()
{}

std::string Minuit2::getIp()
{
  return mIp;
}

Protocol & Minuit2::setIp(std::string ip)
{
  mIp = ip;
  return *this;
}

int Minuit2::getInPort()
{
  return mInPort;
}

Protocol & Minuit2::setInPort(int in_port)
{
  mInPort = in_port;
  return *this;
}

int Minuit2::getOutPort()
{
  return mOutPort;
}

Protocol & Minuit2::setOutPort(int out_port)
{
  mOutPort = out_port;
  return *this;
}

bool Minuit2::updateChildren(Node& node) const
{
  BasicNode& n = dynamic_cast<BasicNode&>(node);

  return n.updateChildren();
}
}
