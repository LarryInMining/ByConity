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

#include <DaemonManager/DaemonJobServerBGThread.h>
#include <Protos/daemon_manager_rpc.pb.h>

namespace DB::DaemonManager
{

class QueryCacheManager;
class DaemonManagerServiceImpl : public DB::Protos::DaemonManagerService
{
public:
    DaemonManagerServiceImpl(std::unordered_map<CnchBGThreadType, DaemonJobServerBGThreadPtr> daemon_jobs_, QueryCacheManager * query_cache_manager_)
        : daemon_jobs(std::move(daemon_jobs_)), query_cache_manager{query_cache_manager_}
    {}

    void GetAllBGThreadServers(
        ::google::protobuf::RpcController * controller,
        const ::DB::Protos::GetAllBGThreadServersReq * request,
        ::DB::Protos::GetAllBGThreadServersResp * response,
        ::google::protobuf::Closure * done) override;

    void GetDMBGJobInfo(
        ::google::protobuf::RpcController * controller,
        const ::DB::Protos::GetDMBGJobInfoReq * request,
        ::DB::Protos::GetDMBGJobInfoResp * response,
        ::google::protobuf::Closure * done) override;

    void ControlDaemonJob(
        ::google::protobuf::RpcController * controller,
        const ::DB::Protos::ControlDaemonJobReq * request,
        ::DB::Protos::ControlDaemonJobResp * response,
        ::google::protobuf::Closure * done) override;

    void ForwardOptimizeQuery(
        ::google::protobuf::RpcController * controller,
        const ::DB::Protos::ForwardOptimizeQueryReq * request,
        ::DB::Protos::ForwardOptimizeQueryResp * response,
        ::google::protobuf::Closure * done) override;

#if 0
    void GetOrInsertCacheInfo(
        ::google::protobuf::RpcController * controller,
        const ::DB::Protos::GetOrInsertCacheInfoReq * request,
        ::DB::Protos::GetOrInsertCacheInfoResp * response,
        ::google::protobuf::Closure * done) override;

    void SetLastUpdateTimestamp(
        ::google::protobuf::RpcController * controller,
        const ::DB::Protos::SetLastUpdateTimestampReq * request,
        ::DB::Protos::SetLastUpdateTimestampResp * response,
        ::google::protobuf::Closure * done) override;
#endif
private:
    std::unordered_map<CnchBGThreadType, DaemonJobServerBGThreadPtr> daemon_jobs;
    QueryCacheManager * query_cache_manager;
    Poco::Logger * log = &Poco::Logger::get("DaemonManagerRPCService");
};

using DaemonManagerServicePtr = std::shared_ptr<DaemonManagerServiceImpl>;

}
