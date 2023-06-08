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

#include <Common/tests/gtest_global_context.h>
#include <DaemonManager/QueryCacheManager/QueryCacheManager.h>
#include <string>
#include <gtest/gtest.h>

using namespace DB::DaemonManager;
using namespace DB;

namespace GtestQueryCacheManager
{

constexpr auto SERVER1 = "169.128.0.1:1223";
constexpr auto SERVER2 = "169.128.0.2:1223";
constexpr auto SERVER3 = "169.128.0.3:1223";
UUID uuid1 = UUID{UInt128{0, 1}};
UUID uuid2 = UUID{UInt128{0, 2}};
UUID uuid3 = UUID{UInt128{0, 3}};
UUID uuid4 = UUID{UInt128{0, 4}};
UUID uuid5 = UUID{UInt128{0, 5}};
UUID uuid6 = UUID{UInt128{0, 6}};

TEST(QueryCacheManager, normal_test)
{
    QueryCacheManager cache_manager;
    cache_manager.setAliveServers({SERVER1, SERVER2});
    {
        CacheInfo cache_info = cache_manager.getOrInsertCacheInfo(SERVER1, uuid1, 1);
        CacheInfo expected {SERVER1, 1};
        EXPECT_EQ(cache_info, expected);
    }

    {
        CacheInfo cache_info = cache_manager.getOrInsertCacheInfo(SERVER2, uuid1, 2);
        CacheInfo expected {SERVER1, 1};
        EXPECT_EQ(cache_info, expected);
    }

    cache_manager.setAliveServers({SERVER2, SERVER3});

    {
        CacheInfo cache_info = cache_manager.getOrInsertCacheInfo(SERVER2, uuid1, 3);
        CacheInfo expected {SERVER2, 3};
        EXPECT_EQ(cache_info, expected);
    }

    cache_manager.setLastUpdateTs(uuid1, 4);

    {
        CacheInfo cache_info = cache_manager.getOrInsertCacheInfo(SERVER3, uuid1, 4);
        CacheInfo expected {SERVER2, 4};
        EXPECT_EQ(cache_info, expected);
    }
}

} // end namespace

