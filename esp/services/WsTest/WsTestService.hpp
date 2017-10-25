/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2012 HPCC Systems®.

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

#ifndef _WsTestService_HPP__
#define _WsTestService_HPP__

#include "WsTest_esp.ipp"

class CWsTestEx : public CWsTest
{
public:
   IMPLEMENT_IINTERFACE;

    CWsTestEx() {}

    virtual void init(IPropertyTree *cfg, const char *process, const char *service);
    bool onEcho(IEspContext &context, IEspEchoRequest &req, IEspEchoResponse &resp);
};

#endif //_EclDirectService_HPP__
