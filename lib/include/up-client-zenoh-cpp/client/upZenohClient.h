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

#ifndef _UP_ZENOH_CLIENT_H_
#define _UP_ZENOH_CLIENT_H_

#include <optional>
#include <up-client-zenoh-cpp/transport/zenohUTransport.h>
#include <up-client-zenoh-cpp/rpc/zenohRpcClient.h>
#include <up-core-api/uri.pb.h>

namespace uprotocol::client {

    class UpZenohClient : public uprotocol::utransport::ZenohUTransport, public uprotocol::rpc::ZenohRpcClient {

        private:
            struct ConstructToken {};

        public:
            UpZenohClient(const struct ConstructToken &) {}
            UpZenohClient(const UpZenohClient&) = delete;
            UpZenohClient& operator=(const UpZenohClient&) = delete;

            /**
            * The API provides an instance of the zenoh session
            * @return instance of UpZenohClient
            */
            static std::shared_ptr<UpZenohClient> instance() noexcept {
                return instance({}, {});
            }

            /**
            * The API provides an instance of the zenoh session
            * @return instance of UpZenohClient
            */
            static std::shared_ptr<UpZenohClient> instance(
                std::optional<uprotocol::v1::UAuthority>,
                std::optional<uprotocol::v1::UEntity>) noexcept;
    };
    
}


#endif /* _UP_ZENOH_CLIENT_H_ */

