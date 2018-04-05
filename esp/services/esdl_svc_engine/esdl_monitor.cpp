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

#include "esdl_monitor.hpp"
#include "jmd5.hpp"
#include "esdl_binding.hpp"
#include "esdl_svc_engine.hpp"
#include "esdl_store.hpp"
#include <memory>

static CriticalSection gEsdlMonitorCritSection;

class CEsdlMonitor : implements IEsdlMonitor, public CInterface, implements IEsdlListener
{
private:
    MapStringTo<EsdlBindingImpl*> m_esdlBindingMap;
    CriticalSection m_CritSect;
    StringBuffer mEnvptHeader;
    Owned<IEsdlStore> m_pCentralStore;
    Owned<IEsdlSubscription> m_pSubscription;
public:
    IMPLEMENT_IINTERFACE;

    CEsdlMonitor()
    {
        constructEnvptHeader();
        m_pCentralStore.setown(createEsdlCentralStore());
        DBGLOG("EsdlMonitor started.");
    }

    virtual ~CEsdlMonitor()
    {
        m_pSubscription->unsubscribe();
    }

    void setupSubscription()
    {
        m_pSubscription.setown(createEsdlSubscription(this));
    }

    //Reference count increment is done by the function
    virtual void registerBinding(const char* bindingId, IEspRpcBinding* binding) override
    {
        if(!bindingId || !binding)
            return;
        EsdlBindingImpl* esdlbinding = dynamic_cast<EsdlBindingImpl*>(binding);
        if(esdlbinding)
        {
            CriticalBlock cb(m_CritSect);
            m_esdlBindingMap.setValue(bindingId, esdlbinding);
        }
    }

    void loadDynamicBindings()
    {
        IPropertyTree * esdlBindings = m_pCentralStore->getBindings();
        if (!esdlBindings)
           throw MakeStringException(-1, "Unable to retrieve ESDL bindings information");

        Owned<IPropertyTreeIterator> iter = esdlBindings->getElements("Binding");
        ForEach(*iter)
        {
            std::unique_ptr<EsdlNotifyData> data(new EsdlNotifyData);
            IPropertyTree & cur = iter->query();
            StringBuffer portStr;
            cur.getProp("@id", data->id);
            cur.getProp("@espprocess", data->espProcess);
            if(!espProcessMatch(data->espProcess.str()))
            {
                DBGLOG("ESDL binding %s is not for this esp process, skip.", data->id.str());
                continue;
            }
            cur.getProp("@espbinding", data->name);
            if(data->name.length() > 0 && existsStaticBinding(data->name.str()))
            {
                DBGLOG("ESDL binding %s has esp binding configured, no need to create it dynamically, skip.", data->id.str());
                continue;
            }
            else
                DBGLOG("ESDL binding %s doesn't have esp binding configured, creating the binding dynamically...", data->id.str());

            cur.getProp("@port", portStr);
            if(portStr.length() == 0)
            {
                DBGLOG("ESDL binding %s doesn't have port specified, skip", data->id.str());
                continue;
            }
            data->port = atoi(portStr.str());
            addBinding(data.get());
        }
    }

    //IEsdlListener
    virtual void onNotify(EsdlNotifyData* data) override
    {
        if(!data)
            return;
        EsdlNotifyType ntype = data->type;
        if(ntype == EsdlNotifyType::DefinitionUpdate)
        {
            CriticalBlock cb(m_CritSect);
            for (auto sb:m_esdlBindingMap)
            {
                EsdlBindingImpl* binding = sb.getValue();
                if (binding->usesESDLDefinition(data->id.str()))
                {
                    DBGLOG("Requesting reload of ESDL definitions for binding %s...", (const char*)sb.getKey());
                    StringBuffer loaded;
                    binding->reloadDefinitionsFromCentralStore(nullptr, loaded);
                }
            }
        }
        else if(ntype == EsdlNotifyType::BindingDelete)
        {
            CriticalBlock cb(m_CritSect);
            EsdlBindingImpl* theBinding = findBinding(data->id.str());
            if(theBinding)
            {
                DBGLOG("Requesting clearing of binding %s", data->id.str());
                if(data->name.length() > 0 && existsStaticBinding(data->name.str()))
                {
                    DBGLOG("Static esp binding exists for this esdl binding.");
                    theBinding->reloadBindingFromCentralStore(data->id.str()); //clear the binding by reloading
                }
                else
                {
                    theBinding->clearBindingState();
                    if(data->port <= 0)
                        data->port = theBinding->getPort();
                    DBGLOG("Removing binding from port %d", data->port);
                    queryEspServer()->removeBinding(data->port, *theBinding);
                }
                removeBindingFromMap(data->id.str());
            }
            else
                DBGLOG("Can't delete binding %s, it doesn't exist", data->id.str());
        }
        else if(ntype == EsdlNotifyType::BindingUpdate)
        {
            CriticalBlock cb(m_CritSect);
            EsdlBindingImpl* theBinding = findBinding(data->id.str());
            if(!theBinding)
            {
                DBGLOG("Binding %s not found, can't update", data->id.str());
                return;
            }
            DBGLOG("Reloading ESDL binding %s", data->id.str());
            theBinding->reloadBindingFromCentralStore(data->id.str());
        }
        else if(ntype == EsdlNotifyType::BindingAdd)
        {
            CriticalBlock cb(m_CritSect);
            EsdlBindingImpl* theBinding = findBinding(data->id.str());
            if(theBinding)
            {
                DBGLOG("Binding %s already exists, reload...", data->id.str());
                theBinding->reloadBindingFromCentralStore(data->id.str());
                return;
            }

            if(!espProcessMatch(data->espProcess.str()))
            {
                DBGLOG("ESDL binding %s is not for this esp process, ignore.", data->id.str());
                return;
            }
            bool existsStatic = false;
            if(data->name.length() > 0)
                existsStatic = existsStaticBinding(data->name.str());
            else
                data->name.set(data->id);
            if(!existsStatic)
            {
                DBGLOG("ESDL binding %s doesn't have esp binding configured, creating the binding dynamically...", data->id.str());
                if (data->port == 0)
                {
                    DBGLOG("Port is not provided for binding, can't create binding.");
                    return;
                }
                addBinding(data);
            }
            else
            {
                DBGLOG("ESDL binding %s has esp binding configured, reloading the esp binding...", data->id.str());
                //Reload static binding
                IEspServer* server = queryEspServer();
                IEspRpcBinding* espBinding = server->queryBinding(data->name.str());
                if(espBinding != nullptr)
                {
                    EsdlBindingImpl* esdlBinding = dynamic_cast<EsdlBindingImpl*>(espBinding);
                    if(esdlBinding != nullptr)
                    {
                        esdlBinding->reloadBindingFromCentralStore(data->id.str());
                        registerBinding(data->id.str(), esdlBinding);
                    }
                    else
                        DBGLOG("The esp binding failed to be cast to esdl binding.");
                }
                else
                    DBGLOG("Esp binding not found.");
            }
        }
        else
        {
            //DefintionAdd and DefinitionDelete shouldn't happen
        }
    }

private:
    EsdlBindingImpl* findBinding(const char* bindingId)
    {
        if(!bindingId)
            return nullptr;
        EsdlBindingImpl** ptrptr = m_esdlBindingMap.getValue(bindingId);
        if(ptrptr)
            return *ptrptr;
        else
            return nullptr;
    }

    void removeBindingFromMap(const char* bindingId)
    {
        if(!bindingId)
            return;
        CriticalBlock cb(m_CritSect);
        m_esdlBindingMap.remove(bindingId);
    }

    void addBinding(EsdlNotifyData* data)
    {
        DBGLOG("Creating new binding %s", data->id.str());
        StringBuffer protocol, serviceName;
        Owned<IPropertyTree> envpt = getEnvpt(data, protocol, serviceName);
        if(protocol.length() == 0)
            protocol.set("http");
        StringBuffer envptxml;
        toXML(envpt, envptxml);
        DBGLOG("Use the following config tree to create the binding and service:\n%s\n", envptxml.str());
        IEspServer* server = queryEspServer();
        IEspProtocol* espProtocol = server->queryProtocol(protocol.str());
        Owned<EsdlBindingImpl> esdlbinding = new CEsdlSvcEngineSoapBindingEx(envpt,  data->name.str(), data->espProcess.str());
        Owned<EsdlServiceImpl> esdlservice = new CEsdlSvcEngine();
        esdlservice->init(envpt, data->espProcess.str(), serviceName.str());
        esdlbinding->addService(esdlservice->getServiceType(), nullptr, data->port, *esdlservice.get());
        esdlbinding->addProtocol(protocol.str(), *espProtocol);
        server->addBinding(data->name.str(), nullptr, data->port, *espProtocol, *esdlbinding.get(), false, envpt);
        DBGLOG("Successfully instantiated new DESDL binding %s and service %s", data->id.str(), serviceName.str());
    }

    bool existsStaticBinding(const char* espBinding)
    {
        if(!espBinding)
            return false;
        DBGLOG("Checking if there is esp binding %s configured...", espBinding);
        IPropertyTree* procpt = queryEspServer()->queryProcPT();
        if(procpt)
        {
            VStringBuffer xpath("EspBinding[@name='%s']", espBinding);
            if(procpt->queryPropTree(xpath.str()) != nullptr)
                return true;
        }
        return false;
    }

    bool espProcessMatch(const char* espProcess)
    {
        if(!espProcess)
            return false;
        return (strcmp(espProcess, queryEspServer()->getProcName()) == 0);
    }

    void constructEnvptHeader()
    {
        mEnvptHeader.clear().append("<Environment><Software><EspProcess ");
        IPropertyTree* procpt = queryEspServer()->queryProcPT();
        if(procpt)
        {
            Owned<IAttributeIterator> attrs = procpt->getAttributes(false);
            for(attrs->first(); attrs->isValid(); attrs->next())
            {
                StringBuffer name(attrs->queryName());
                StringBuffer value;
                encodeXML(attrs->queryValue(), value);
                if(name.length() > 1)
                    mEnvptHeader.appendf("%s=\"%s\" ", name.str()+1, value.str());
            }
        }
        mEnvptHeader.append(">");
    }

    IPropertyTree* getEnvpt(EsdlNotifyData* notifyData, StringBuffer& protocol, StringBuffer& serviceName)
    {
        VStringBuffer portStr("%d", notifyData->port);
        return getEnvpt(notifyData->espProcess.str(), notifyData->name.str(),
                portStr.str(), protocol, serviceName);
    }

    IPropertyTree* getEnvpt(const char* espProcess, const char* bindingName, const char* port, StringBuffer& protocol, StringBuffer& serviceName)
    {
        const static int DEFAULT_PORT = 8043;
        const static int DEFAULT_HTTPS_PORT = 18043;
        serviceName.set(bindingName);
        StringBuffer envxmlbuf;
        IPropertyTree* procpt = queryEspServer()->queryProcPT();
        if(procpt)
        {
            //If esp's original config has one configured for this binding, use it
            VStringBuffer xpath("EspBinding[@name='%s']", bindingName);
            IPropertyTree* bindingtree = procpt->queryPropTree(xpath.str());
            if(bindingtree)
            {
                protocol.set(bindingtree->queryProp("@protocol"));
                return LINK(procpt);
            }
            //Otherwise check if there's binding configured on the same port
            xpath.clear().appendf("EspBinding[@type='EsdlBinding'][@port='%s']", port);
            bindingtree = procpt->queryPropTree(xpath.str());
            if(!bindingtree)
            {
                //Otherwise check if there's binding configured on the default esdl port
                xpath.clear().appendf("EspBinding[@type='EsdlBinding'][@port='%d']", DEFAULT_PORT);
                bindingtree = procpt->queryPropTree(xpath.str());
                if(!bindingtree)
                {
                    xpath.clear().appendf("EspBinding[@type='EsdlBinding'][@port='%d']", DEFAULT_HTTPS_PORT);
                    bindingtree = procpt->queryPropTree(xpath.str());
                }
            }
            if(bindingtree)
            {
                bindingtree->getProp("@protocol", protocol);
                const char* service = bindingtree->queryProp("@service");
                xpath.clear().appendf("EspService[@name='%s']", service);
                IPropertyTree* servicetree = procpt->queryPropTree(xpath.str());
                if(servicetree)
                {
                    bindingtree = createPTreeFromIPT(bindingtree);
                    servicetree = createPTreeFromIPT(servicetree);
                    bindingtree->setProp("@name", bindingName);
                    bindingtree->setProp("@service", serviceName.str());
                    bindingtree->setProp("@port", port);
                    servicetree->setProp("@name", serviceName.str());
                    servicetree->setProp("@type", "DynamicESDL");
                    envxmlbuf.appendf("%s</EspProcess></Software></Environment>", mEnvptHeader.str());
                    Owned<IPropertyTree> envpttree = createPTreeFromXMLString(envxmlbuf.str());
                    envpttree->addPropTree("Software/EspProcess/EspBinding", bindingtree);
                    envpttree->addPropTree("Software/EspProcess/EspService", servicetree);
                    return envpttree.getClear();
                }
            }
        }
        //Otherwise construct a config, mostly for testing and demo purposes
        WARNLOG("There's no esp binding configured on port %s, default port %d or default HTTPS port %d, constructing one on the fly", port, DEFAULT_PORT, DEFAULT_HTTPS_PORT);
        protocol.set("http");
        envxmlbuf.appendf("%s\n<EspService name=\"%s\" type=\"DynamicESDL\" plugin=\"esdl_svc_engine\" namespaceBase=\"urn:hpccsystems:ws\"/>"
                               "<EspBinding name=\"%s\" service=\"%s\" protocol=\"http\" type=\"EsdlBinding\" plugin=\"esdl_svc_engine\" netAddress=\"0.0.0.0\" port=\"%s\"/>\n"
                           "</EspProcess></Software></Environment>",
                mEnvptHeader.str(), serviceName.str(), bindingName, serviceName.str(), port);
        return createPTreeFromXMLString(envxmlbuf.str());
    }
};

static Owned<IEsdlMonitor> gEsdlMonitor;
static bool isEsdlMonitorStarted = false;

extern "C" esdl_decl void startEsdlMonitor()
{
    CriticalBlock cb(gEsdlMonitorCritSection);
    if(gEsdlMonitor.get() == nullptr)
    {
        CEsdlMonitor* monitor = new CEsdlMonitor();
        gEsdlMonitor.setown(monitor);
        isEsdlMonitorStarted = true;
        monitor->loadDynamicBindings();
        monitor->setupSubscription();
    }
}

extern "C"  void stopEsdlMonitor()
{
    DBGLOG("stopping esdl monitor...");
    if(gEsdlMonitor.get() != nullptr)
        gEsdlMonitor.clear();
}

esdl_decl IEsdlMonitor* queryEsdlMonitor()
{
    if(!isEsdlMonitorStarted)
        startEsdlMonitor();
    return gEsdlMonitor.get();
}
