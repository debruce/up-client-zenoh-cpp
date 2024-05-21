#pragma once

#include <chrono>
#include <optional>
#include <future>

#include "Transport.hpp"

namespace PluggableTransport {
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


struct PublisherApi {
   typedef std::shared_ptr<PublisherApi> (*Getter)(Transport, const std::string&);
   virtual void operator()(const Message&) = 0;
};

class Publisher {
   std::shared_ptr<PublisherApi> pImpl;
public:
   Publisher(Transport transport, const std::string& topic);
   void operator()(const Message& message) { (*pImpl)(message); }
};


struct SubscriberApi {
   typedef std::shared_ptr<SubscriberApi> (*Getter)(Transport, const std::string&, SubscriberServerCallback);
};

class Subscriber {
   std::shared_ptr<SubscriberApi> pImpl;
public:
   Subscriber(Transport transport, const std::string& topic, SubscriberServerCallback callback);
};


struct RpcClientApi {
   typedef std::shared_ptr<RpcClientApi> (*Getter)(Transport, const std::string&, const Message&, const std::chrono::seconds&);
   virtual std::tuple<std::string, Message> operator()() = 0;
};

class RpcClient {
   std::shared_ptr<RpcClientApi> pImpl;
public:
   RpcClient(Transport transport, const std::string& topic, const Message& message, const std::chrono::seconds& timeout);

   std::tuple<std::string, Message> operator()() { return (*pImpl)(); }
};

std::future<std::tuple<std::string, Message>> queryCall(Transport transport, std::string expr, const Message& message, const std::chrono::seconds& timeout);


struct RpcServerApi {
   typedef std::shared_ptr<RpcServerApi> (*Getter)(Transport, const std::string&, RpcServerCallback);
};

class RpcServer {
   std::shared_ptr<RpcServerApi> pImpl;
public:
   RpcServer(Transport transport, const std::string& topic, RpcServerCallback callback);
};

}; // PluggableTransport