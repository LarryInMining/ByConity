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

#include <netdb.h>
#include <MergeTreeCommon/CnchTopologyMaster.h>
#include <DaemonManager/DaemonHelper.h>
#include <DaemonManager/DaemonFactory.h>
#include <DaemonManager/DMDefines.h>
#include <DaemonManager/DaemonJobQueryCacheManager.h>
#include <Core/UUID.h>
#include <iterator>

namespace DB
{

namespace ErrorCodes
{
    extern const int FILE_DOESNT_EXIST;
    extern const int CANNOT_OPEN_FILE;
    extern const int SYSTEM_ERROR;
    extern const int NOT_ENOUGH_SPACE;
    extern const int CANNOT_KILL;
}

namespace DaemonManager
{

namespace
{

std::vector<ServerAddress> getServerAddressesFromHostPortVec(const HostWithPortsVec & host_ports)
{
    std::vector<ServerAddress> res;
    std::transform(host_ports.begin(), host_ports.end(), std::back_inserter(res),
        [] (const HostWithPorts & host_port)
        {
            return ServerAddress{host_port.getHost(), host_port.getTCPPort()};
        }
    );

    return res;
}

}

DaemonJobQueryCacheManager::DaemonJobQueryCacheManager(ContextMutablePtr global_context_)
    : DaemonJob{global_context_, CnchBGThreadType::QueryCacheManager}
{
    topology_master = global_context_->getCnchTopologyMaster();
    if (!topology_master)
        throw Exception("Failed to get topology master", ErrorCodes::SYSTEM_ERROR);

    std::list<CnchServerTopology> server_topologies = topology_master->getCurrentTopology();
    if (server_topologies.empty())
        return;

    HostWithPortsVec host_ports = server_topologies.back().getServerList();

    std::vector<ServerAddress> server_addresses = getServerAddressesFromHostPortVec(host_ports);
    cache_manager.setAliveServers(server_addresses);
}

void registerQueryCacheManagerDaemon(DaemonFactory & factory)
{
    factory.registerLocalDaemonJob<DaemonJobQueryCacheManager>("QUERY_CACHE_MANAGER");
}

bool DaemonJobQueryCacheManager::executeImpl()
{
    std::list<CnchServerTopology> server_topologies = topology_master->getCurrentTopology();
    if (server_topologies.empty())
    {
        LOG_WARNING(log, "Server topology is empty");
        return false;
    }

    HostWithPortsVec host_ports = server_topologies.back().getServerList();
    std::vector<ServerAddress> server_addresses = getServerAddressesFromHostPortVec(host_ports);
    cache_manager.setAliveServers(server_addresses);
}

QueryCacheManager * lookForQueryCacheManager(std::vector<DaemonJobPtr> & local_daemon_jobs)
{
    QueryCacheManager * res{nullptr};
    for (DaemonJobPtr & daemon_job : local_daemon_jobs)
    {
        if (daemon_job->getType() == CnchBGThreadType::QueryCacheManager)
        {
            DaemonJobQueryCacheManager * j = reinterpret_cast<DaemonJobQueryCacheManager *>(daemon_job.get());
            res = j->getQueryCacheManager();
        }
    }
    return res;
}

} /// end namespace DaemonManager
} /// end namespace DB

