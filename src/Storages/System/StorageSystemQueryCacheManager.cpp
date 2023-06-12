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

#include <Storages/System/StorageSystemQueryCacheManager.h>
#include <Core/NamesAndTypes.h>
#include <Columns/IColumn.h>
#include <Interpreters/Context.h>
#include <Storages/SelectQueryInfo.h>
#include <DataTypes/DataTypeUUID.h>
#include <CloudServices/CnchServerClient.h>
#include <Common/HostWithPorts.h>


namespace DB
{

namespace
{

String toString(const ServerAddress & server_address)
{
    return createHostPortString(server_address.host, server_address.tcp_port);
}

String toString(const std::vector<ServerAddress> & addresses)
{
    String res;
    char separator = ' ';
    std::for_each(addresses.begin(), addresses.end(),
        [&res, &separator] (const ServerAddress & s)
        {
            res += separator + toString(s);
            separator = ',';
        }
    );
    return res;
}

}

NamesAndTypesList StorageSystemQueryCacheManager::getNamesAndTypes()
{
    return
    {
        {"uuid", std::make_shared<DataTypeUUID>()},
        {"host", std::make_shared<DataTypeString>()},
        {"tcp_port", std::make_shared<DataTypeUInt16>()},
        {"last_update_ts", std::make_shared<DataTypeUInt64>()},
        {"readable_last_update_ts", std::make_shared<DataTypeDateTime>()},
        {"source_of_last_update_ts", std::make_shared<DataTypeString>()}
    };
}

void StorageSystemQueryCacheManager::fillData(MutableColumns & res_columns, ContextPtr context, const SelectQueryInfo &) const
{
    if (context->getServerType() != ServerType::cnch_server)
        return;

    DaemonManagerClientPtr client = context->getDaemonManagerClient();
    QueryCacheManagerInfos cache_info = client->getQueryCacheInfos();
    LOG_INFO(log, "alive servers: {}", toString(cache_info.alive_servers));
    std::for_each(cache_infos.begin(), cache_infos.end(),
    [& res_columns] (const std::pair<UUID, CacheInfo> & entry)
    {
        const CacheInfo & cache_info = entry.second;
        res_columns[0]->insert(entry.first);
        res_columns[1]->insert(entry.cache_info.server_address.host);
        res_columns[2]->insert(entry.cache_info.server_address.tcp_port);
        res_columns[3]->insert(entry.cache_info.last_update_ts);
        res_columns[4]->insert((entry.cache_info.last_update_ts >> 18)/ 1000);
        res_columns[5]->insert("DaemonManager");
    });
}

} // end namespace DB
