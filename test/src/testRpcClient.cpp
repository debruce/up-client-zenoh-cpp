/*
 * Copyright (c) 2024 General Motors GTO LLC
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * SPDX-FileType: SOURCE
 * SPDX-FileCopyrightText: 2024 General Motors GTO LLC
 * SPDX-License-Identifier: Apache-2.0
 */

#include <csignal>
#include <spdlog/spdlog.h>
#include <up-client-zenoh-cpp/client/upZenohClient.h>
#include <up-cpp/transport/builder/UAttributesBuilder.h>
#include <up-cpp/uri/serializer/MicroUriSerializer.h>
#include <sys/types.h>
#include <signal.h>
#include <gtest/gtest.h>
#include <up-client-zenoh-cpp/trace_hook.hpp>
#include <sstream>


using namespace uprotocol::utransport;
using namespace uprotocol::v1;
using namespace uprotocol::uri;
using namespace uprotocol::rpc;
using namespace uprotocol::client;

IMPL_TRACEHOOK()

namespace {

UUri const& rpcUri() { 
    static auto uri = BuildUUri()
                      .setAutority(BuildUAuthority().build())
                      .setEntity(BuildUEntity()
                              .setName("test_rpc.app")
                              .setMajorVersion(1)
                              .setId(1)
                              .build())
                      .setResource(BuildUResource()
                              .setRpcRequest("handler", 1)
                              .build())
                      .build();
    return uri;
}

UUri const& rpcNoServerUri() {
    static auto uri = BuildUUri()
                      .setAutority(BuildUAuthority().build())
                      .setEntity(BuildUEntity()
                              .setName("test_rpc.app")
                              .setMajorVersion(1)
                              .setId(1)
                              .build())
                      .setResource(BuildUResource()
                              .setRpcRequest("noServer", 2)
                              .build())
                      .build();
    return uri;
}

} // anonymous namespace

static size_t cntr;

class RpcServer : public UListener {

     public:

        UStatus onReceive(UMessage &message) const override {
            TRACE();
            {
                using namespace std;
                cout << "onReceive called in " << getpid() << ' ' << get_type(message.payload()) << endl;
                // auto ptr = message.payload().data();
                // for (size_t i = 0; i < message.payload().size(); i++) cout << ptr[i] << ' ';
                // cout << endl;
            }
            // cout << "Request attributes #######################################" << endl;
            // cout << message.attributes().DebugString() << endl;
            UStatus status;

            status.set_code(UCode::OK);
            
            auto builder = UAttributesBuilder::response(
                message.attributes().source(),
                message.attributes().sink(),
                UPriority::UPRIORITY_CS0,
                message.attributes().id());
            builder.setReqid(message.attributes().id());

            UAttributes responseAttributes = builder.build();
            // cout << "Response attributes #######################################" << endl;
            // cout << responseAttributes.DebugString() << endl;

            // UPayload outPayload = message.payload();
            using namespace std;
            stringstream ss;
            ss << "Reply " << cntr++;
            UPayload outPayload((const uint8_t*)ss.str().data(), ss.str().size(), UPayloadType::VALUE);

            UMessage respMessage(outPayload, responseAttributes);
            TRACE();
            if (nullptr != message.payload().data()) {

                std::string cmd(message.payload().data(), message.payload().data() + message.payload().size());

                if ("No Response" != cmd) {
                    TRACE();
                    auto result =  UpZenohClient::instance(message.attributes().source().authority())->send(respMessage);
                    TRACE(); 
                    return result;
                }
            } else {
                TRACE();
                auto result = UpZenohClient::instance(message.attributes().source().authority())->send(respMessage);
                TRACE(); 
                return result;
            }
            TRACE();    
            return status;
        }
};

class ResponseListener : public UListener {

     public:

        UStatus onReceive(UMessage &message) const override {
            TRACE();
            (void) message;

            UStatus status;

            status.set_code(UCode::OK);
           
            return status;
        }
};

class TestRPcClient : public ::testing::Test {

    public:
        // SetUpTestSuite() is called before all tests in the test suite
        static void SetUpTestSuite() {

        }

        // TearDownTestSuite() is called after all tests in the test suite
        static void TearDownTestSuite() {

        }

        static RpcServer rpcListener;
        static ResponseListener responseListener;

};

RpcServer TestRPcClient::rpcListener;
ResponseListener TestRPcClient::responseListener;

// TEST_F(TestRPcClient, InvokeMethodWithoutServer) {
    
//     auto instance = UpZenohClient::instance(BuildUAuthority().setName("rpc_client").build());
    
//     EXPECT_NE(instance, nullptr);

//     UPayload payload(nullptr, 0, UPayloadType::REFERENCE);
    
//     CallOptions options;

//     options.set_priority(UPriority::UPRIORITY_CS4);

//     std::future<RpcResponse> future = instance->invokeMethod(rpcNoServerUri(), payload, options);

//     EXPECT_EQ(future.valid(), true);
    
//     auto response = future.get();
    
//     EXPECT_NE(response.status.code(), UCode::OK);
// }

// TEST_F(TestRPcClient, InvokeMethodWithLowPriority) {
    
//     auto instance = UpZenohClient::instance(BuildUAuthority().setName("rpc_client").build());

//     EXPECT_NE(instance, nullptr);

//     UPayload payload(nullptr, 0, UPayloadType::REFERENCE);
    
//     CallOptions options;

//     options.set_priority(UPriority::UPRIORITY_CS3);

//     std::future<RpcResponse> future = instance->invokeMethod(rpcNoServerUri(), payload, options);

//     EXPECT_EQ(future.valid(), false);
// }

// TEST_F(TestRPcClient, invokeMethodNoResponse) {
    
//     auto instance = UpZenohClient::instance(BuildUAuthority().setName("rpc_client").build());

//     EXPECT_NE(instance, nullptr);

//     std::string message = "No Response";
//     std::vector<uint8_t> data(message.begin(), message.end());

//     UPayload payload(data.data(), data.size(), UPayloadType::VALUE);
    
//     CallOptions options;

//     options.set_priority(UPriority::UPRIORITY_CS4);
//     options.set_ttl(1000);

//     std::future<RpcResponse> future = instance->invokeMethod(rpcUri(), payload, options);

//     EXPECT_EQ(future.valid(), true);
    
//     auto response = future.get();
    
//     EXPECT_NE(response.status.code(), UCode::OK);
// }

// TEST_F(TestRPcClient, maxSimultaneousRequests) {
    
//     auto instance = UpZenohClient::instance(BuildUAuthority().setName("rpc_client").build());

//     EXPECT_NE(instance, nullptr);

//     auto status = instance->registerListener(rpcUri(), TestRPcClient::rpcListener);

//     EXPECT_EQ(status.code(), UCode::OK);

//     std::string message = "No Response";
//     std::vector<uint8_t> data(message.begin(), message.end());

//     UPayload payload(data.data(), data.size(), UPayloadType::VALUE);
    
//     CallOptions options;

//     options.set_priority(UPriority::UPRIORITY_CS4);
//     options.set_ttl(5000);

//     size_t numRequestsUntilQueueIsFull = instance->getMaxConcurrentRequests() + instance->getQueueSize();

//     for (size_t i = 0; i < (numRequestsUntilQueueIsFull + 1) ; ++i) {
//         std::future<RpcResponse> future = instance->invokeMethod(rpcUri(), payload, options);

//         if (i < numRequestsUntilQueueIsFull) {
//             EXPECT_EQ(future.valid(), true);
//         } else {
//             EXPECT_EQ(future.valid(), false);
//         }
//     }

//     /* wait for al futures to return */
//     sleep(10);

//     std::future<RpcResponse> future = instance->invokeMethod(rpcUri(), payload, options);

//     EXPECT_EQ(future.valid(), true);

//     status = instance->unregisterListener(rpcUri(), TestRPcClient::rpcListener);

//     EXPECT_EQ(status.code(), UCode::OK);
// }

// TEST_F(TestRPcClient, invokeMethodWithNullResponse) {
    
//     auto instance = UpZenohClient::instance(BuildUAuthority().setName("rpc_client").build());

//     EXPECT_NE(instance, nullptr);

//     auto status = instance->registerListener(rpcUri(), TestRPcClient::rpcListener);

//     EXPECT_EQ(status.code(), UCode::OK);

//     UPayload payload(nullptr, 0, UPayloadType::REFERENCE);
    
//     CallOptions options;

//     options.set_priority(UPriority::UPRIORITY_CS4);
//     options.set_ttl(1000);

//     std::future<RpcResponse> future = instance->invokeMethod(rpcUri(), payload, options);

//     EXPECT_EQ(future.valid(), true);
    
//     auto response = future.get();
    
//     // EXPECT_EQ(response.status.code(), UCode::OK);

//     EXPECT_EQ(response.message.payload().size(), 0);

//     status = instance->unregisterListener(rpcUri(), TestRPcClient::rpcListener);

//     EXPECT_EQ(status.code(), UCode::OK);
// }

TEST_F(TestRPcClient, invokeMethodWithResponse) {
    using namespace std;

    TRACE();
    std::string message = "Response";
    std::vector<uint8_t> data(message.begin(), message.end());

    auto child_pid = fork();
    if (child_pid == 0) {
        cout << "after fork child=" << getpid() << " sees parent=" << getppid() << dec << endl;
        TRACE();
        auto instance = UpZenohClient::instance(BuildUAuthority().setName("rpc_server").build());
        if (instance == nullptr) {
            TRACE();
            cerr << "server instance allocation failed" << endl;
            exit(-1);
        }
        TRACE();
        auto status = instance->registerListener(rpcUri(), TestRPcClient::rpcListener);
        if (status.code() != UCode::OK) {
            TRACE();
            cerr << "server instance registerListener failed" << endl;
            exit(-1);
        }
        TRACE();
        sleep(100000);
    }
    else {
        sleep(2);
        TRACE();
        cout << "after fork parent=" << getpid() << " sees child=" << child_pid << dec << endl;
        TRACE();
        auto instance = UpZenohClient::instance(BuildUAuthority().setName("rpc_client").build());
        EXPECT_NE(instance, nullptr);
        TRACE();  

        CallOptions options;

        options.set_priority(UPriority::UPRIORITY_CS4);
        options.set_ttl(1000);

        for (size_t i = 0; i < 1; i++) {
            sleep(1);
            TRACE();
            using namespace std;
            stringstream ss;
            ss << "Hello world " << i;
            UPayload payload((const uint8_t*)ss.str().data(), ss.str().size(), UPayloadType::VALUE);  
            std::future<RpcResponse> future = instance->invokeMethod(rpcUri(), payload, options);

            EXPECT_EQ(future.valid(), true);
            TRACE();
            auto response = future.get();
            TRACE();
            cout << "response ###################################################################" << endl;
            // cout << response.message.attributes().DebugString() << endl;
            if (response.message.payload().size() > 0) {
                auto& data = response.message.payload(); 
                cout << "[ ";
                for (size_t i = 0; i < data.size(); i++) cout << data.data()[i];
                cout << " ]" << endl;
            }
            
            EXPECT_EQ(response.status.code(), UCode::OK);
            EXPECT_NE(response.message.payload().data(), nullptr);
            EXPECT_NE(response.message.payload().size(), 0);
        }

        kill(child_pid, SIGKILL);
    }

    TRACE();
}

// TEST_F(TestRPcClient, invokeMethodWithCbResponse) {
    
//     auto instance = UpZenohClient::instance(BuildUAuthority().setName("rpc_client").build());
    
//     EXPECT_NE(instance, nullptr);

//     std::string message = "Response";
//     std::vector<uint8_t> data(message.begin(), message.end());

//     UPayload payload(data.data(), data.size(), UPayloadType::VALUE);    

//     CallOptions options;

//     options.set_priority(UPriority::UPRIORITY_CS4);
//     options.set_ttl(1000);

//     auto status = instance->invokeMethod(rpcUri(), payload, options, responseListener);

//     // EXPECT_EQ(status.code(), UCode::OK);  
// }

// TEST_F(TestRPcClient, invokeMethodWithCbResponseFailure) {
    
//     auto instance = UpZenohClient::instance(BuildUAuthority().setName("rpc_client").build());
  
//     EXPECT_NE(instance, nullptr);

//     std::string message = "Response";
//     std::vector<uint8_t> data(message.begin(), message.end());

//     UPayload payload(data.data(), data.size(), UPayloadType::VALUE);    

//     CallOptions options;

//     options.set_priority(UPriority::UPRIORITY_CS0);
//     options.set_ttl(1000);

//     auto status = instance->invokeMethod(rpcUri(), payload, options, responseListener);

//     EXPECT_NE(status.code(), UCode::OK);  
// }

int main(int argc, char **argv) {
    TRACE();
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    TRACE();
    return result;
}
