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

#include <DaemonManager/DaemonManagerClient.h>
#include <Protos/RPCHelpers.h>
#include <Protos/daemon_manager_rpc.pb.h>
#include <brpc/channel.h>
#include <brpc/controller.h>


namespace DB::DaemonManager
{
DaemonManagerClient::DaemonManagerClient(String host_port)
    : RpcClientBase(getName(), std::move(host_port))
    , stub_ptr(std::make_unique<Protos::DaemonManagerService_Stub>(&getChannel()))
{
}

DaemonManagerClient::DaemonManagerClient(HostWithPorts host_ports_)
    : RpcClientBase(getName(), std::move(host_ports_))
    , stub_ptr(std::make_unique<Protos::DaemonManagerService_Stub>(&getChannel()))
{
}

DaemonManagerClient::~DaemonManagerClient() = default;

BGJobInfos DaemonManagerClient::getAllBGThreadServers(CnchBGThreadType type)
{
    brpc::Controller cntl;
    Protos::GetAllBGThreadServersReq req;
    Protos::GetAllBGThreadServersResp resp;

    req.set_job_type(type);

    stub_ptr->GetAllBGThreadServers(&cntl, &req, &resp, nullptr);
    assertController(cntl);
    RPCHelpers::checkResponse(resp);

    BGJobInfos res;
    for (auto i = 0; i < resp.dm_bg_job_infos_size(); ++i)
    {
        res.push_back(BGJobInfo{
            RPCHelpers::createStorageID(resp.dm_bg_job_infos(i).storage_id()),
            CnchBGThreadStatus{resp.dm_bg_job_infos(i).status()},
            CnchBGThreadStatus{resp.dm_bg_job_infos(i).expected_status()},
            resp.dm_bg_job_infos(i).host_port(),
            resp.dm_bg_job_infos(i).last_start_time(),
        });
    }
    return res;
}

std::optional<BGJobInfo> DaemonManagerClient::getDMBGJobInfo(const UUID & storage_uuid, CnchBGThreadType type)
{
    brpc::Controller cntl;
    Protos::GetDMBGJobInfoReq req;
    Protos::GetDMBGJobInfoResp resp;
    RPCHelpers::fillUUID(storage_uuid, *req.mutable_storage_uuid());
    req.set_job_type(type);
    stub_ptr->GetDMBGJobInfo(&cntl, &req, &resp, nullptr);
    assertController(cntl);
    RPCHelpers::checkResponse(resp);
    if (!resp.has_dm_bg_job_info())
        return {};

    return BGJobInfo{
        RPCHelpers::createStorageID(resp.dm_bg_job_info().storage_id()),
        CnchBGThreadStatus{resp.dm_bg_job_info().status()},
        CnchBGThreadStatus{resp.dm_bg_job_info().expected_status()},
        resp.dm_bg_job_info().host_port(),
        resp.dm_bg_job_info().last_start_time()
    };
}

void DaemonManagerClient::controlDaemonJob(const StorageID & storage_id, CnchBGThreadType job_type, CnchBGThreadAction action)
{
    brpc::Controller cntl;
    Protos::ControlDaemonJobReq req;
    Protos::ControlDaemonJobResp resp;

    RPCHelpers::fillStorageID(storage_id, *req.mutable_storage_id());
    req.set_job_type(job_type);
    req.set_action(action);

    stub_ptr->ControlDaemonJob(&cntl, &req, &resp, nullptr);

    assertController(cntl);
    RPCHelpers::checkResponse(resp);
}

void DaemonManagerClient::forwardOptimizeQuery(const StorageID & storage_id, const String & partition_id, bool enable_try, bool mutations_sync, UInt64 timeout_ms)
{
    brpc::Controller cntl;
    Protos::ForwardOptimizeQueryReq req;
    Protos::ForwardOptimizeQueryResp resp;

    RPCHelpers::fillStorageID(storage_id, *req.mutable_storage_id());
    req.set_partition_id(partition_id);
    req.set_enable_try(enable_try);
    req.set_mutations_sync(mutations_sync);

    if (mutations_sync && timeout_ms)
    {
        /// set timeout for sync mode.
        cntl.set_timeout_ms(timeout_ms);
        req.set_timeout_ms(timeout_ms);
    }

    stub_ptr->ForwardOptimizeQuery(&cntl, &req, &resp, nullptr);

    assertController(cntl);
    RPCHelpers::checkResponse(resp);
}

CacheInfo DaemonManagerClient::getOrInsertQueryCacheInfo(const ServerAddress & origin_server, const UUID uuid, const TxnTimestamp query_txn_ts)
{
    brpc::Controller cntl;
    Protos::GetOrInsertQueryCacheInfoReq req;
    Protos::GetOrInsertQueryCacheInfoResp resp;

    RPCHelpers::fillUUID(uuid, *req.mutable_uuid());
    RPCHelpers::fillCacheServerAddress(origin_server, *req.mutable_send_server_address());
    req.set_query_txn_ts(query_txn_ts);
    stub_ptr->GetOrInsertQueryCacheInfo(&cntl, &req, &resp, nullptr);

    assertController(cntl);
    RPCHelpers::checkResponse(resp);
    CacheInfo res{RPCHelpers::createCacheServerAddress(resp.cache_info_entry.server_address), resp.cache_info_entry.last_update_ts};
    return res;
}

void DaemonManagerClient::setQueryCacheLastUpdateTimestamp(const UUID, const TxnTimestamp update_ts)
{
    brpc::Controller cntl;
    Protos::SetQueryCacheLastUpdateTimestampReq req;
    Protos::SetQueryCacheLastUpdateTimestampResp resp;

    RPCHelpers::fillUUID(uuid, *req.mutable_uuid());
    req.set_update_ts(update_ts);
    stub_ptr->SetQueryCacheLastUpdateTimestamp(&cntl, &req, &resp, nullptr);

    assertController(cntl);
    RPCHelpers::checkResponse(resp);
}

QueryCacheManagerInfos DaemonManagerClient::getQueryCacheInfos()
{
    brpc::Controller cntl;
    Protos::GetQueryCacheInfosReq req;
    Protos::GetQueryCacheInfosResp resp;

    stub_ptr->GetQueryCacheInfos(&cntl, &req, &resp, nullptr);
    assertController(cntl);
    RPCHelpers::checkResponse(resp);

    QueryCacheManagerInfos res;
    std::transform(resp.server_addresses().begin(), resp.server_addresses().end(),
        std::back_inserter(res.alive_servers),
        [&res] (auto & server_address)
        {
            return RPCHelpers::createCacheServerAddress(server_address);
        }
    );
    std::transform(resp.cache_info_entries.begin(), resp.cache_info_entries.end(),
        std::back_inserter(res.cache_infos),
        [&res] (auto & cache_info_entry)
        {
            ServerAddress add{cache_info_entry.server_address.host(), };
            return std::make_pair(
                    RPCHelpers::createUUID(cache_info_entry.uuid()),
                    CacheInfo{
                        RPCHelpers::createCacheServerAddress(cache_info_entry.server_address()),
                        cache_info_entry.last_update_ts()
                             }
                   );
        }
    );

    return res;
}

}
