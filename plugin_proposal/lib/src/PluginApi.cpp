#include "PluginApi.hpp"
#include <iostream>
#include <map>

namespace PluggableTransport {

using namespace std;


Publisher::Publisher(Transport transport, const std::string& topic)
{
   auto a = transport.get_factory("publisher");
   auto getter = any_cast<PublisherApi::Getter>(a);
   pImpl = (*getter)(transport, topic);
}


Subscriber::Subscriber(Transport transport, const std::string& topic, SubscriberServerCallback callback)
{
   auto a = transport.get_factory("subscriber");
   auto getter = any_cast<SubscriberApi::Getter>(a);
   pImpl = (*getter)(transport, topic, callback);      
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


RpcServer::RpcServer(Transport transport, const std::string& topic, RpcServerCallback callback)
{
   auto a = transport.get_factory("rpc_server");
   auto getter = any_cast<RpcServerApi::Getter>(a);
   pImpl = (*getter)(transport, topic, callback);        
}

}; // PluggableTransport

///////////////////////////////////////////////////////////////////////////////////
// Transport
///////////////////////////////////////////////////////////////////////////////////

namespace Impl_v1 {
   using namespace PluggableTransport;
   std::shared_ptr<PublisherApi> publisher_getter(Transport transport, const std::string& name);
   std::shared_ptr<SubscriberApi> subscriber_getter(Transport transport, const std::string& topic, SubscriberServerCallback callback);
   std::shared_ptr<RpcClientApi> rpc_client_getter(Transport transport, const std::string& topic, const Message& message, const std::chrono::seconds& timeout);
   std::shared_ptr<RpcServerApi> rpc_server_getter(Transport transport, const std::string& topic, RpcServerCallback callback);
};

namespace Impl_v2 {
   using namespace PluggableTransport;
   std::shared_ptr<PublisherApi> publisher_getter(Transport transport, const std::string& name);
   std::shared_ptr<SubscriberApi> subscriber_getter(Transport transport, const std::string& topic, SubscriberServerCallback callback);
   std::shared_ptr<RpcClientApi> rpc_client_getter(Transport transport, const std::string& topic, const Message& message, const std::chrono::seconds& timeout);
   std::shared_ptr<RpcServerApi> rpc_server_getter(Transport transport, const std::string& topic, RpcServerCallback callback);
};

namespace PluggableTransport {
struct Transport::Impl {
   map<string, any>  getters;

    Impl(const string& start_doc) {
      if (start_doc == "v1") {
         getters["publisher"] = ::Impl_v1::publisher_getter;
         getters["subscriber"] = ::Impl_v1::subscriber_getter;
         getters["rpc_client"] = ::Impl_v1::rpc_client_getter;
         getters["rpc_server"] = ::Impl_v1::rpc_server_getter;
      }
      else if (start_doc == "v2") {
         getters["publisher"] = ::Impl_v2::publisher_getter;
         getters["subscriber"] = ::Impl_v2::subscriber_getter;
         getters["rpc_client"] = ::Impl_v2::rpc_client_getter;
         getters["rpc_server"] = ::Impl_v2::rpc_server_getter;
      }
      else throw runtime_error("Unsupported version");
    }

    ~Impl() {

    }

    any get_factory(const string& name)
    {
        return getters[name];
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