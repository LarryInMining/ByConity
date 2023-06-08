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

#include <DaemonManager/QueryCacheManager/QueryCacheManager.h>

namespace DB::DaemonManager
{

CacheInfo QueryCacheManager::getOrInsertCacheInfo(const ServerAddress & origin_server, const UUID uuid, const TxnTimestamp query_txn_ts)
{
    UUIDToCacheInfoMapPart & map_part = uuid_map[getFirstLevelIdx(uuid)];
    std::lock_guard lock{map_part.mutex};
    auto it = map_part.map.find(uuid);
    if (it != map_part.map.end() && (!alive_servers.contains(it->second.server_address)))
        it->second.server_address = origin_server;

    if (it == map_part.map.end())
    {
        /// create cache at server where the original query come from
        auto res = map_part.map.insert(std::make_pair(uuid, CacheInfo{origin_server, query_txn_ts}));
        it = res.first;
    }

    return it->second;
}

void QueryCacheManager::setLastUpdateTs(const UUID uuid, const TxnTimestamp update_ts)
{
    UUIDToCacheInfoMapPart & map_part = uuid_map[getFirstLevelIdx(uuid)];
    std::lock_guard lock{map_part.mutex};

    auto it = map_part.map.find(uuid);
    if (it == map_part.map.end())
        return;
    it->second.last_update_ts = update_ts;
}

void QueryCacheManager::AliveServers::set(std::vector<ServerAddress> servers)
{
    std::lock_guard lock{alive_server_mutex};
    alive_servers = std::move(servers);
}

bool QueryCacheManager::AliveServers::contains(const ServerAddress & s)
{
    std::lock_guard lock{alive_server_mutex};
    auto it = std::find(alive_servers.begin(), alive_servers.end(), s);
    return (it != alive_servers.end());
}

void QueryCacheManager::setAliveServers(std::vector<String> servers)
{
    alive_servers.set(std::move(servers));
}

}

