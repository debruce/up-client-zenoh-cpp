#include "PluginApi.hpp"
#include <iostream>

namespace PluggableTransport {

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

Publisher::Publisher(Transport transport, const std::string& topic)
{
   auto a = transport.get_factory("publisher");
   auto getter = any_cast<PublisherApi::Getter>(a);
   pImpl = (*getter)(transport, topic);
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

Subscriber::Subscriber(Transport transport, const std::string& topic, SubscriberServerCallback callback)
{
   auto a = transport.get_factory("subscriber");
   auto getter = any_cast<SubscriberApi::Getter>(a);
   pImpl = (*getter)(transport, topic, callback);      
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

std::shared_ptr<RpcClientImpl> rpc_client_getter(Transport transport, const std::string& topic, const Message& message, const std::chrono::seconds& timeout)
{
    return make_shared<RpcClientImpl>(transport, topic, message, timeout);
}

RpcClient::RpcClient(Transport transport, const std::string& topic, const Message& message, const std::chrono::seconds& timeout)
{
   auto a = transport.get_factory("rpc_client");
   auto getter = any_cast<RpcClientApi::Getter>(a);
   pImpl = (*getter)(transport, topic, message, timeout);       
}

future<tuple<string, Message>> queryCall(Transport transport, string expr, const Message& message, const chrono::seconds& timeout)
{
    auto msg = make_shared<Message>(message);
    return async([=]() { return RpcClient(transport, expr, *msg, timeout)(); } );
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

RpcServer::RpcServer(Transport transport, const std::string& topic, RpcServerCallback callback)
{
   auto a = transport.get_factory("rpc_server");
   auto getter = any_cast<RpcServerApi::Getter>(a);
   pImpl = (*getter)(transport, topic, callback);        
}

///////////////////////////////////////////////////////////////////////////////////
// Transport
///////////////////////////////////////////////////////////////////////////////////
struct Transport::Impl {
    Impl(const string& start_doc) {

    }

    ~Impl() {

    }

    any get_factory(const string& name)
    {
        if (name == "publisher") return publisher_getter;
        if (name == "subscriber") return subscriber_getter;
        if (name == "rpc_client") return rpc_client_getter;
        if (name == "rpc_server") return rpc_server_getter;
        return any();
    }
};

Transport::Transport(const std::string& start_doc) : pImpl(new Impl(start_doc))
{}

any Transport::get_factory(const string& name)
{
    return pImpl->get_factory(name);
}

}; // PluggableTransport