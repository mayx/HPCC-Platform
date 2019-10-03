
/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2019 HPCC Systems.

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

#pragma warning (disable : 4786)

#ifndef WS_CODESIGN_API
#define WS_CODESIGN_API DECL_EXPORT
#endif //WS_CODESIGN_API

#include "ws_codesign_esp.ipp"

//ESP Bindings
#include "httpprot.hpp"

//ESP Service
#include "ws_codesignService.hpp"

#include "espplugin.hpp"

extern "C"
{

ESP_FACTORY IEspService * esp_service_factory(const char *name, const char* type, IPropertyTree *cfg, const char *process)
{
    if (strcmp(type, "ws_codesign")==0)
    {
        Cws_codesignEx* service = new Cws_codesignEx;
        service->init(cfg, process, name);
        return service;
    }
    return nullptr;
}

ESP_FACTORY IEspRpcBinding * esp_binding_factory(const char *name, const char* type, IPropertyTree *cfg, const char *process)
{
    if (strcmp(type, "ws_codesignSoapBinding")==0)
    {
        return new Cws_codesignSoapBinding(cfg, name, process);
    }

    return nullptr;
}

ESP_FACTORY IEspProtocol * esp_protocol_factory(const char *name, const char* type, IPropertyTree *cfg, const char *process)
{
    return http_protocol_factory(name, type, cfg, process);
}

} // extern "C"
