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
#include <DaemonManager/DaemonJobGlobalGC.h>
#include <DaemonManager/DaemonHelper.h>
#include <DaemonManager/DaemonFactory.h>
#include <DaemonManager/DMDefines.h>
#include <Core/UUID.h>
#include <MergeTreeCommon/CnchTopologyMaster.h>
#include <CloudServices/CnchServerClientPool.h>
#include <CloudServices/CnchServerClient.h>
#include <iterator>

namespace DB::DaemonManager
{


DaemonJobQueryCacheManager(ContextMutablePtr global_context_);
    : DaemonJobQueryCacheManager{std::move(global_context_), CnchBGThreadType::QueryCacheManager}
{
    topology_master = global_context->getCnchTopologyMaster();
    if (!topology_master)
        throw Exception("Failed to get topology master", ErrorCodes::INVALID_CONFIG_PARAMETER);

    std::list<CnchServerTopology> server_topologies = topology_master->getCurrentTopology();
    if (server_topologies.empty())
        return;

    HostWithPortsVec host_ports = server_topologies.back().getServerList();

    std::vector<ServerAddress> server_addresses;
    std::transform(host_ports.begin(), host_ports.end(), std::back_inserter(ret),
        [] (const HostWithPorts & host_port)
        {
            return {host_port.getHost(), host_port.getTCPPort()};
        }
    );
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
        return;

    HostWithPortsVec host_ports = server_topologies.back().getServerList();

    std::vector<ServerAddress> server_addresses;
    std::transform(host_ports.begin(), host_ports.end(), std::back_inserter(ret),
        [] (const HostWithPorts & host_port)
        {
            return {host_port.getHost(), host_port.getTCPPort()};
        }
    );
    cache_manager.setAliveServers(server_addresses);
}

QueryCacheManager * lookforQueryCacheManager(std::vector<DaemonJobPtr> & local_daemon_jobs)
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


}
