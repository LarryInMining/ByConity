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

#include <DaemonManager/DaemonJob.h>
#include <Catalog/Catalog.h>

namespace DB::DaemonManager
{

class DaemonJobQueryCacheManager : public DaemonJob
{
public:
    DaemonJobQueryCacheManager(ContextMutablePtr global_context_);
        : DaemonJob{std::move(global_context_), CnchBGThreadType::QueryCacheManager}
    {}

    QueryCacheManager * getQueryCacheManager() { return &cache_manager; }
protected:
    bool executeImpl() override;
private:
    QueryCacheManager cache_manager;
    std::shared_ptr<CnchTopologyMaster> topology_master;
};

QueryCacheManager * lookforQueryCacheManager(std::vector<DaemonJobPtr> & local_daemon_jobs);

}
