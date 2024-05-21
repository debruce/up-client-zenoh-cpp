#include <iostream>
#include <sstream>
#include <future>
#include <string_view>
#include <chrono>
#include <map>
#include <unistd.h>

#include "PluginApi.hpp"

using namespace std;
using namespace PluggableTransport;

int main(int argc, char* argv[])
{
    auto transport = Transport("start_doc");

    auto rpc_server_callback = [&](const string& sending_topic, const string& listening_topic, const Message& message) {
        cout << "rpc callback with"
            << " from=" << sending_topic
            << " to=" << listening_topic
            << " payload=" << message.payload
            << " attributes=" << message.attributes << endl;
        return Message{"hello", "world"};
    };
    auto rpc_server = RpcServer(transport, "demo/rpc/*", rpc_server_callback);
    sleep(10000);
}

