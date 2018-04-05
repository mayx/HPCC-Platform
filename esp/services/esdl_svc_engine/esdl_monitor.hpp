/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2018 HPCC Systems®.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
############################################################################## */
#ifndef ESDL_MONITOR_HPP

#include "esdl_def.hpp"
#include "esdl_transformer.hpp"

interface IEsdlMonitor : implements IInterface
{
    virtual void registerBinding(const char* bindingId, IEspRpcBinding* binding) = 0;
};

extern "C" esdl_decl void startEsdlMonitor();
extern "C" esdl_decl void stopEsdlMonitor();
esdl_decl IEsdlMonitor* queryEsdlMonitor();

#define ESDL_MONITOR_HPP

#endif //ESDL_MONITOR_HPP
