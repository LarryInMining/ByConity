/*
 * Copyright (2022) Bytedance Ltd. and/or its affiliates
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <Core/Types.h>
#include <Transaction/TxnTimestamp.h>
#include <unordered_map>
#include <mutex>

namespace DB::DaemonManager
{

struct CacheInfo
{
    String server_address;
    TxnTimestamp last_update_ts;
};

/*when the query using query cache, firstly the server that process query reach DM, DM find the machine that store that cache, if there is no machine to store cache for the table yet, create one and return the CacheInfo with the last_update_ts is the timestamp of select query. Then the server forward the query to that server. It is better to also forward the info of last_update_ts, in the new server it process the query, depend on if the last_update_ts is forward along, it may need to send request to DM one more time*/

class QueryCacheManager
{
public:
    CacheInfo getOrInsertCacheInfo(const String & origin_server, const UUID, const TxnTimestamp query_txn_ts);
    void setLastUpdateTs(const UUID, const TxnTimestamp update_ts);
    void updateWithServerInfo(const std::vector<String> & alive_server);
private:
    struct UUIDToCacheInfoMapPart
    {
        std::unordered_map<UUID, CacheInfo> map;
        std::mutex mutex;
    };

    static constexpr UInt64 bits_for_first_level = 4;
    static inline size_t getFirstLevelIdx(const UUID & uuid)
    {
        return uuid.toUnderType().items[0] >> (64 - bits_for_first_level);
    }

    using UUIDToCacheInfoMap = std::array<UUIDToCacheInfoMapPart, 1ull << bits_for_first_level>;
    UUIDToCacheInfoMap uuid_map;

    class AliveServers
    {
    public:
        void set(std::vector<String> servers);
        bool contains(const String & s);
    private:
        std::vector<String> alive_servers;
        std::mutex alive_server_mutex;
    };

    AliveServers alive_servers;
};

} /// end namespace
