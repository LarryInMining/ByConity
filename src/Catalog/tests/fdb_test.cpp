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

#include <Catalog/MetastoreFDBImpl.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <thread>
#include <atomic>

using String = std::string;
using Test = std::function<bool()>;


bool testPutAndGet(DB::Catalog::MetastoreFDBImpl & metastore)
{
    for (int i = 0; i < 2000; ++i)
    {
        std::string key = "xkey" + std::to_string(i);
        metastore.put(key.c_str(), "value1");
        String value;
        metastore.get(key.c_str(), value);
        if (value == "value1")
            continue;
        else
            return false;
    }
    return true;
}

int main(int , char ** argv)
{
    String cluster_path = argv[1];
    DB::Catalog::MetastoreFDBImpl metastore(cluster_path);

    //if (false == testPutAndGet(metastore))
    //    std::cout << "insert failed\n";
    auto it_ptr =  metastore.getByPrefix("xkey");
    while (it_ptr->next())
    {
        std::cout << "kkey: " << it_ptr->key() << '\n';
    }
    return 0;
}
