#include "PluginApi.hpp"
#include <iostream>

namespace Impl_v2 {

using namespace PluggableTransport;
using namespace std;

///////////////////////////////////////////////////////////////////////////////////
// Publisher
///////////////////////////////////////////////////////////////////////////////////
struct PublisherImpl : public PublisherApi {
   PublisherImpl(Transport transport, const std::string& name)
   {
      cout << __PRETTY_FUNCTION__ << " name=" << name << endl; 
   }

   void operator()(const Message&) override
   {
      cout << __PRETTY_FUNCTION__ << endl;
   }
};

std::shared_ptr<PublisherApi> publisher_getter(Transport transport, const std::string& name)
{
    return make_shared<PublisherImpl>(transport, name);
}

///////////////////////////////////////////////////////////////////////////////////
// Subscriber
///////////////////////////////////////////////////////////////////////////////////
struct SubscriberImpl : public SubscriberApi {
   SubscriberImpl(Transport transport, const std::string& topic, SubscriberServerCallback)
   {
      cout << __PRETTY_FUNCTION__ << " topic=" << topic << endl; 
   }
};

std::shared_ptr<SubscriberApi> subscriber_getter(Transport transport, const std::string& topic, SubscriberServerCallback callback)
{
    return make_shared<SubscriberImpl>(transport, topic, callback);
}

///////////////////////////////////////////////////////////////////////////////////
// RpcClient
///////////////////////////////////////////////////////////////////////////////////
struct RpcClientImpl : public RpcClientApi {
   RpcClientImpl(Transport transport, const std::string& topic, const Message& message, const std::chrono::seconds& timeout)
   {
      cout << __PRETTY_FUNCTION__ << " topic=" << topic << endl; 
   }

   tuple<string, Message> operator()() override
   {
      cout << __PRETTY_FUNCTION__ << endl;
      return make_tuple(string("hello"), Message{});      
   }
};

std::shared_ptr<RpcClientApi> rpc_client_getter(Transport transport, const std::string& topic, const Message& message, const std::chrono::seconds& timeout)
{
    return make_shared<RpcClientImpl>(transport, topic, message, timeout);
}

///////////////////////////////////////////////////////////////////////////////////
// RpcServer
///////////////////////////////////////////////////////////////////////////////////
struct RpcServerImpl : public RpcServerApi {
   RpcServerImpl(Transport transport, const string& topic, RpcServerCallback callback)
   {
      cout << __PRETTY_FUNCTION__ << " topic=" << topic << endl; 
   }
};

std::shared_ptr<RpcServerApi> rpc_server_getter(Transport transport, const string& topic, RpcServerCallback callback)
{
    return make_shared<RpcServerImpl>(transport, topic, callback);
}

}; // ImplV1