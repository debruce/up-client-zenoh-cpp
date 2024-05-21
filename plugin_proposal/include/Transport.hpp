#pragma once

#include <memory>
#include <any>

namespace PluggableTransport {

struct Transport {
   Transport(const std::string& start_doc);
    std::any get_factory(const std::string&);

    struct Impl;
    std::shared_ptr<Impl> pImpl;
};

};