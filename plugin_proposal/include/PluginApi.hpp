#pragma once

#include "FactoryPlugin.hpp"

#include <chrono>
#include <optional>
#include <future>

namespace PluggableTransport {
//
// This is the base type for the common session object.
//
struct TransportApi {
   virtual ~TransportApi() {}
};

//
// This is a type alias for subscriber server processing.
//
struct Message {
   std::string payload;
   std::string attributes;
};

//
// This is a type alias for subscriber server callaback function signature.
//
using SubscriberServerCallback = std::function<void (const std::string& sending_topic, const std::string& listening_topic, const Message&)>;

//
// This is a type alias for RPC server callaback function signature.
//
using RpcReply = Message;
using RpcServerCallback = std::function<std::optional<RpcReply> (const std::string& sending_topic, const std::string& listening_topic, const Message&)>;

//
// This makes a type alias for the getter lambdas implemented in the implementation module.
//
template <typename CLS, class... Args>
using FactoryGetter = std::function< std::shared_ptr<CLS> (std::shared_ptr<TransportApi>, Args...) >;


//
// These are the base types for the concepts provided by the implementation plugin.
// The type alias at the top of each is the signature of the factory function for the implementation.
//
struct PublisherApi {
   using FactoryParams = FactoryGetter<PublisherApi, const std::string&, const std::string&>;
   virtual void operator()(const Message&) = 0;
};

struct SubscriberApi {
   using FactoryParams = FactoryGetter<SubscriberApi, const std::string&, SubscriberServerCallback, size_t, const std::string&>;
};

struct RpcClientApi {
   using FactoryParams = FactoryGetter<RpcClientApi, const std::string&, const Message&, const std::chrono::seconds&>;
   virtual std::tuple<std::string, Message> operator()() = 0;
};

struct RpcServerApi {
   using FactoryParams = FactoryGetter<RpcServerApi, const std::string&, RpcServerCallback, size_t, const std::string&>;
};

//
// The next declaration is the factory struct that the implementation will return at load time.
//

struct Factories {
   std::function< std::shared_ptr<TransportApi> (const std::string&, const std::string&)> get_session;
   PublisherApi::FactoryParams get_publisher;
   SubscriberApi::FactoryParams get_subscriber;
   RpcClientApi::FactoryParams get_rpc_client;
   RpcServerApi::FactoryParams get_rpc_server;
};

using PluginApi = FactoryPlugin<Factories>;

//
// Below this point are the thin wrapper classes that hold the pImpls pointing back to the dll implementation
//
class Transport {
   PluginApi plugin;
   std::shared_ptr<TransportApi> pImpl;
public:
   friend class Publisher;
   friend class Subscriber;
   friend class RpcClient;
   friend class RpcServer;

   Transport(PluginApi plugin, const std::string& start_doc, const std::string& trace_name = "session")
      : plugin(plugin), pImpl(plugin->get_session(start_doc, trace_name)) {}
};


class Publisher {
   std::shared_ptr<PublisherApi> pImpl;
public:
   Publisher(Transport transport, const std::string& topic, const std::string& trace_name = "publisher")
      : pImpl(transport.plugin->get_publisher(transport.pImpl, topic, trace_name)) {}

   void operator()(const Message& message) { (*pImpl)(message); }
};

class Subscriber {
   std::shared_ptr<SubscriberApi> pImpl;
public:
   Subscriber(Transport transport, const std::string& topic, SubscriberServerCallback callback, size_t thread_count = 4, const std::string& trace_name = "subscriber")
      : pImpl(transport.plugin->get_subscriber(transport.pImpl, topic, callback, thread_count, trace_name)) {}
};

class RpcClient {
   std::shared_ptr<RpcClientApi> pImpl;
public:
   RpcClient(Transport transport, const std::string& topic, const Message& message, const std::chrono::seconds& timeout)
      : pImpl(transport.plugin->get_rpc_client(transport.pImpl, topic, message, timeout)) {}

   std::tuple<std::string, Message> operator()() { return (*pImpl)(); }
};

std::future<std::tuple<std::string, Message>> queryCall(Transport transport, std::string expr, const Message& message, const std::chrono::seconds& timeout)
{
    auto msg = std::make_shared<Message>(message);
    return std::async([=]() { return RpcClient(transport, expr, *msg, timeout)(); } );
}

class RpcServer {
   std::shared_ptr<RpcServerApi> pImpl;
public:
   RpcServer(Transport transport, const std::string& topic, RpcServerCallback callback, size_t thread_count = 4, const std::string& trace_name = "subscriber")
      : pImpl(transport.plugin->get_rpc_server(transport.pImpl, topic, callback, thread_count, trace_name)) {}

};

}; // PluggableTransport