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
    if (argc < 2) {
        cerr << "Must provide path to impl library and topic to subscribe to." << endl;
        exit(-1);
    }
    
    auto transport = Transport("start_doc");

    auto callback = [](const string& sending_topic, const string& listening_topic, const Message& message) {
        cout << "subscriber callback with"
            << " from=" << sending_topic
            << " to=" << listening_topic
            << " payload=" << message.payload
            << " attributes=" << message.attributes << endl;
    };

    auto subscriber = Subscriber(transport, argv[1], callback);
    sleep(10000);
}

