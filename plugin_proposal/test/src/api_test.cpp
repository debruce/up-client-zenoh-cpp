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

template <typename KEY>
class Histogram {
    std::mutex mtx;
    std::map<KEY, size_t>   counts;
public:
    void operator()(const KEY& key)
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = counts.find(key);
        if (it != counts.end()) it->second++;
        else counts[key] = 1;
    }

    std::string density()
    {
        std::lock_guard<std::mutex> lock(mtx);
        stringstream ss;
        ss << "( ";
        for (const auto& [k,v] : counts) {
            ss << '(' << k << ", " << v << ' ';
        }
        ss << ')';
        return ss.str();
    }
};

template <class... Args>
string genString(const char* fmt, Args... args)
{
    char buf[128];
    snprintf(buf, sizeof(buf), fmt, args...);
    return string(buf);
}

void test(const string& dll_path)
{
    auto transport = Transport("start_doc");

    {
        auto callback = [](const string& sending_topic, const string& listening_topic, const Message& message) {
            cout << "subscriber callback with"
                << " from=" << sending_topic
                << " to=" << listening_topic
                << " payload=" << message.payload
                << " attributes=" << message.attributes << endl;
        };
        auto subscriber = Subscriber(transport, "upl/*", callback);
        auto p1 = Publisher(transport, "upl/p1");
        auto p2 = Publisher(transport, "upl/p2");

        for (auto i = 0; i < 5; i++) {
            cout << endl << "client code pubishing " << i << endl;
            p1(Message{genString("pay_A_%d", i), genString("attr_A_%d", i)});
            p2(Message{genString("pay_B_%d", i), genString("attr_B_%d", i)});
            usleep(100000);
        }
    }
    cout << "################### rpc test" << endl;
    {
        auto rpc_server_callback = [&](const string& sending_topic, const string& listening_topic, const Message& message) {
            cout << "rpc callback with"
                << " from=" << sending_topic
                << " to=" << listening_topic
                << " payload=" << message.payload
                << " attributes=" << message.attributes << endl;
            return Message{"hello", "world"};
        };
        auto rpc_server = RpcServer(transport, "demo/rpc/*", rpc_server_callback);

        for (auto i = 0; i < 5; i++) {
            using namespace std::chrono_literals;
            auto f1 = queryCall(transport, "demo/rpc/action1", Message{genString("pay_A_%d", i), genString("attr_A_%d", i)}, 1s);
            auto f2 = queryCall(transport, "demo/rpc/action2", Message{genString("pay_B_%d", i), genString("attr_B_%d", i)}, 1s);
            auto results1 = f1.get();
            auto results2 = f2.get();
            cout << "rpc results " << get<0>(results1) << ' ' << get<1>(results1).payload << ' ' << get<1>(results1).attributes << endl;
            cout << "rpc results " << get<0>(results2) << ' ' << get<1>(results2).payload << ' ' << get<1>(results2).attributes << endl;
            usleep(100000);
        }     
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        cerr << "Must provide path to impl library." << endl;
        exit(-1);
    }

    test(argv[1]);
    // Oh, by the way, dlclose() will not unload or invoke dtors if there are 'UNIQUE' tagged objects in plugin.
    // Palm to forehead.
    sleep(5);
    test(argv[1]);
}

