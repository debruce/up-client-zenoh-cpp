#include <iostream>
#include <sstream>
#include <future>
#include <string_view>
#include <chrono>
#include <map>
#include <unistd.h>

#include "PluginApi.hpp"
#include <unistd.h>

using namespace std;
using namespace PluggableTransport;

int main(int argc, char* argv[])
{
    if (argc < 3) {
        cerr << "Must provide path to impl library and topic to subscribe to." << endl;
        exit(-1);
    }
    
    PluginApi::WhiteList white_list{"6f4e764446ae6c636363448bcfe3e32d"};
    auto plugin =  PluginApi(argv[1]); //, white_list);
    auto transport = Transport(plugin, "start_doc");

    auto callback = [](const string& sending_topic, const string& listening_topic, const Message& message) {
        cout << "subscriber callback with"
            << " from=" << sending_topic
            << " to=" << listening_topic
            << " payload=" << message.payload
            << " attributes=" << message.attributes << endl;
    };

    auto subscriber = Subscriber(transport, argv[2], callback, 4, "sub");
    sleep(10000);
}

