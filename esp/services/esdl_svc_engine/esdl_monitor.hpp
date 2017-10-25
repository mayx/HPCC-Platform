/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2013 HPCC Systems®.

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

interface IEsdlCache : implements IInterface
{
    virtual void add(const char* defId, const char* defsig, const char* defText, const char* loadedServiceName, IEsdlDefinition* defobj, IEsdlTransformer* transformer) = 0;
    virtual IEsdlDefinition* queryDef(const char* defId, StringBuffer& sigbuf, StringBuffer& loadedServiceName) = 0;
    virtual IEsdlTransformer* queryTransformer(const char* defId) = 0;
};

interface IEsdlMonitor : implements IInterface
{
    virtual IEsdlCache* queryEsdlCache() = 0;
    virtual void registerBinding(const char* bindingId, IEspRpcBinding* binding) = 0;
};

interface IEsdlCacheHook : implements IInterface
{
    virtual void esdlDefChanged(const char* defId, IEsdlDefinition* def, IEsdlTransformer* transformer) = 0;
};

esdl_decl void initEsdlMonitor();
esdl_decl IEsdlMonitor* queryEsdlMonitor();
esdl_decl IEsdlCache* queryEsdlCache();

#define ESDL_MONITOR_HPP

#endif //ESDL_MONITOR_HPP
