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

#include "esdl_monitor.hpp"
#include "jmd5.hpp"
#include "dasds.hpp"
#include "esdl_binding.hpp"
#include "esdl_svc_engine.hpp"

class CEsdlInstance : implements IInterface, public CInterface
{
public:
    IMPLEMENT_IINTERFACE;

    StringBuffer m_defId;
    StringBuffer m_defText;
    StringBuffer m_defSig;
    StringBuffer m_loadedServiceName;
    Owned<IEsdlDefinition> m_def;
    Owned<IEsdlTransformer> m_defTran;
};

class CEsdlCache : implements IEsdlCache, public CInterface
{
private:
    CEsdlInstance* queryInstance(const char* defId)
    {
        return m_esdlInstanceMap.getValue(defId);
    }

public:
    IMPLEMENT_IINTERFACE;
    CEsdlCache()
    {
    }
    virtual ~CEsdlCache() {}

    virtual void add(const char* defId, const char* defsig, const char* defText, const char* loadedServiceName, IEsdlDefinition* def, IEsdlTransformer* tran)
    {
        CriticalBlock cb(m_critSect);
        if(m_esdlInstanceMap.find(defId) == nullptr)
        {
            Owned<CEsdlInstance> instance = new CEsdlInstance();
            instance->m_defId.set(defId);
            if(defText)
                instance->m_defText.set(defText);
            if(defsig)
                instance->m_defSig.set(defsig);
            instance->m_loadedServiceName.set(loadedServiceName);
            instance->m_def.set(def);
            if(tran)
                instance->m_defTran.set(tran);
            m_esdlInstanceMap.setValue(defId, *instance.get());
        }
    }

    virtual IEsdlDefinition* queryDef(const char* defId, StringBuffer& defsig, StringBuffer& loadedServiceName)
    {
        CriticalBlock cb(m_critSect);
        CEsdlInstance* instance = queryInstance(defId);
        if(instance)
        {
            if(instance->m_loadedServiceName.length() > 0)
                loadedServiceName.set(instance->m_loadedServiceName);
            defsig.set(instance->m_defSig);
            return instance->m_def.get();
        }
        return nullptr;

    }

    virtual IEsdlTransformer* queryTransformer(const char* defId)
    {
        CriticalBlock cb(m_critSect);
        CEsdlInstance* instance = queryInstance(defId);
        if(instance)
            return instance->m_defTran.get();
        return nullptr;
    }

    private:
        CriticalSection m_critSect;
        MapStringTo<CEsdlInstance> m_esdlInstanceMap;

};

static CriticalSection gEsdlMonitorCritSection;

extern bool trimXPathToParentSDSElement(const char *element, const char * xpath, StringBuffer & parentNodeXPath);

class CEsdlMonitor : implements IEsdlMonitor, public CInterface
{
private:
    class CESDLBindingSubscription;
    class CESDLDefinitionSubscription;
    class CESDLChangeSubscription;
    Owned<CEsdlCache> m_esdlCache;
    //Owned<CESDLBindingSubscription>         m_pBindingSubscription;
    //Owned<CESDLDefinitionSubscription>      m_pDefinitionSubscription;
    Owned<CESDLChangeSubscription>      m_pChangeSubscription;
    MapStringTo<EsdlBindingImpl*> m_esdlBindingMap;
    CriticalSection m_CritSect;
    StringBuffer mEnvptHeader;

public:
    IMPLEMENT_IINTERFACE;

    CEsdlMonitor()
    {
        constructEnvptHeader();
        m_esdlCache.setown(new CEsdlCache());
        //m_pBindingSubscription.setown(new CESDLBindingSubscription(this));
        //m_pDefinitionSubscription.setown(new CESDLDefinitionSubscription(this));
        DBGLOG("EsdlMonitor started.");
    }

    virtual ~CEsdlMonitor()
    {
        m_pChangeSubscription->unsubscribe();
    }

    void setupSubscription()
    {
        m_pChangeSubscription.setown(new CESDLChangeSubscription(this));
    }

    virtual IEsdlCache* queryEsdlCache() { return m_esdlCache.get(); }

    //Reference count increment is done by the function
    virtual void registerBinding(const char* bindingId, IEspRpcBinding* binding)
    {
        if(!bindingId || !binding)
            return;
        EsdlBindingImpl* esdlbinding = dynamic_cast<EsdlBindingImpl*>(binding);
        if(esdlbinding)
        {
            //TODO: is critcal section needed here?
            CriticalBlock cb(m_CritSect);
            m_esdlBindingMap.setValue(bindingId, esdlbinding);
        }
    }

    EsdlBindingImpl* queryBinding(const char* bindingId)
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

    MapStringTo<EsdlBindingImpl*>& queryBindingMap()
    {
        return m_esdlBindingMap;
    }

    CriticalSection& queryCritSect()
    {
        return m_CritSect;
    }

    void loadDynamicBindings()
    {
        Owned<IRemoteConnection> conn = querySDS().connect(ESDL_BINDINGS_ROOT_PATH, myProcessSession(), RTM_LOCK_READ, SDS_LOCK_TIMEOUT_DESDL);
        if (!conn)
           throw MakeStringException(-1, "Unable to connect to ESDL bindings information in dali '%s'", ESDL_BINDINGS_ROOT_PATH);

        conn->close(false); //release lock right away

        IPropertyTree * esdlBindings = conn->queryRoot();
        if (!esdlBindings)
           throw MakeStringException(-1, "Unable to open ESDL bindings information in dali '%s'", ESDL_BINDINGS_ROOT_PATH);

        Owned<IPropertyTreeIterator> iter = conn->queryRoot()->getElements("Binding");
        ForEach(*iter)
        {
            IPropertyTree & cur = iter->query();
            bool hasEspBinding = false;
            StringBuffer bindingId, bindingName, procName, portStr;
            cur.getProp("@id", bindingId);
            cur.getProp("@espbinding", bindingName);
            cur.getProp("@espprocess", procName);
            cur.getProp("@port", portStr);
            if(portStr.length() == 0)
            {
                DBGLOG("ESDL binding %s doesn't have port specified, skip", bindingId.str());
                continue;
            }
            if(bindingName.length() > 0 && procName.length() > 0)
            {
                DBGLOG("ESDL Binding %s has properties espprocess=%s espbinding=%s", bindingId.str(), procName.str(), bindingName.str());
                StringBuffer xpath;
                xpath.appendf("/Environment/Software/EspProcess[@name=\"%s\"]/EspBinding[@name=\"%s\"]", procName.str(), bindingName.str());
                Owned<IRemoteConnection> conn1 = querySDS().connect(xpath.str(), myProcessSession(), RTM_LOCK_READ, SDS_LOCK_TIMEOUT_DESDL);
                if (conn1)
                    hasEspBinding = true;
            }

            if(hasEspBinding)
            {
                DBGLOG("ESDL binding %s has Esp Binding configured, so don't need to create it dynamically, skip.", bindingId.str());
                continue;
            }

            StringBuffer serviceName;
            Owned<IPropertyTree> envpt = getEnvpt(&cur, serviceName);
            IEspServer* server = queryEspServer();
            const char* protocol = cur.queryProp("@protocol");
            if(!protocol || !*protocol)
                protocol = "http";
            IEspProtocol* espProtocol = server->queryProtocol(protocol);
            Owned<EsdlBindingImpl> esdlbinding = new CEsdlSvcEngineSoapBindingEx(envpt,  bindingName.str(), procName.str());
            registerBinding(bindingId.str(), esdlbinding.get());
            Owned<EsdlServiceImpl> esdlservice = new CEsdlSvcEngine();
            esdlservice->init(envpt, procName.str(), serviceName.str());
            int port = atoi(portStr.str());
            esdlbinding->addService(esdlservice->getServiceType(), nullptr, port, *esdlservice.get());
            esdlbinding->addProtocol(protocol, *espProtocol);
            //Yanrui TODO: check if getLink is needed. Also investigate shutdown core
            server->addBinding(bindingName.str(), nullptr, port, *espProtocol, *esdlbinding, false, envpt);
            DBGLOG("Successfully instantiated new DESDL binding %s and service %s", bindingName.str(), serviceName.str());
        }
    }

private:
    class CESDLChangeSubscription : public CInterface, implements ISDSSubscription
    {
    private:
        SubscriptionId sub_id;
        CEsdlMonitor* m_theMonitor;
    public:
        IMPLEMENT_IINTERFACE;

        //TODO:
        // - When set subscribe's sendData to true, won't get notifications
        // - When add a binding, still reports as "SDSNotify_Deleted"? Acutally seems to be getting "changed" update
        CESDLChangeSubscription(CEsdlMonitor* theMonitor) : m_theMonitor(theMonitor)
        {
            CriticalBlock b(m_theMonitor->queryCritSect());
            try
            {
                //<Change type="Binding" action="add" targetId="DESDLBinding2"/>
                sub_id = querySDS().subscribe("/ESDL/Subscription/Change", *this, false, true);
                //Questions: 1 - What does parameters sub and "sendValue" mean? When set both sub and sendValue to true it stops working
                //           2 - Will multiple subscribers all get notified for the same change?
                //           3 - notification comes back as /ESDL[1]/Changes[1]/Change[1]. Then have to query this path from dali to find the new value.
                //               Then the problem arises, how do you guarantee it won't change again at the same time? It would be much easier to
                //               get the changed value, is it possible, is it what "sendValue" supposed to do?

            }
            catch (IException *E)
            {
                DBGLOG("Failed to subscribe to DALI (%s)", ESDL_BINDING_PATH);
                // failure to subscribe implies dali is down... is this ok??
                // Is this bad enough to halt the esp load process??
                E->Release();
            }
        }

        virtual ~CESDLChangeSubscription()
        {
            //unsubscribe();
        }

        void unsubscribe()
        {
            CriticalBlock b(m_theMonitor->queryCritSect());
            try
            {
                if (sub_id)
                {
                    querySDS().unsubscribe(sub_id);
                    sub_id = 0;
                }
            }
            catch (IException *E)
            {
                E->Release();
            }
            sub_id = 0;
        }

        void notify(SubscriptionId id, const char *xpath, SDSNotifyFlags flags, unsigned valueLen=0, const void *valueData=NULL)
        {
            if (id != sub_id)
            {
                DBGLOG("Dali subscription (%" I64F "d) received notification for unrecognized dali subscription id: (%" I64F "d)", (__int64) sub_id, (__int64) id);
                return;
            }

            StringBuffer parentElementXPath;
            DBGLOG("ESDL change reported to path %s - flags = %d, valueLen = %d valueData=%s", xpath, flags, valueLen, valueData?(const char*)valueData:"");

            if(valueLen == 0)
            {
                DBGLOG("There's no data from notify, ignore.");
                return;
            }

            Owned<IProperties> props = createProperties(false);
            parseChangeString(valueLen, (const char*)valueData, *props.get());

            //<Change action="add" type="binding" espProcess="myesp" targetName="DESDLBinding2" targetId="myesp.DESDLBinding2" protocol="http" port="8004"/>
            StringBuffer changeType;
            StringBuffer changeAction;
            StringBuffer targetId;
            StringBuffer espProcess;
            StringBuffer targetName;
            StringBuffer protocol;
            StringBuffer portStr;
            changeType.set(props->queryProp("type"));
            changeAction.set(props->queryProp("action"));
            targetId.set(props->queryProp("targetId"));
            espProcess.set(props->queryProp("espProcess"));
            targetName.set(props->queryProp("targetName"));
            protocol.set(props->queryProp("protocol"));
            portStr.set(props->queryProp("port"));
            if (targetId.length() == 0)
            {
                DBGLOG("targetId is empty, ignore this change notification.");
                return;
            }
            if (portStr.length() == 0)
            {
                DBGLOG("port is not provided, ignore this change notification.");
                return;
            }

            DBGLOG("Port = %s", portStr.str());
            int port = atoi(portStr.str());
            EsdlBindingImpl* theBinding = NULL;
            if(changeType.length() > 0 && strcmp(changeType.str(), "definition") == 0)
            {
                //Deal with defintion changes
            }
            else if(changeType.length() > 0 && strcmp(changeType.str(), "binding") == 0)
            {
                if(changeAction.length() > 0 && strcmp(changeAction.str(), "delete") == 0)
                {
                    CriticalBlock cb(m_theMonitor->queryCritSect());
                    theBinding = m_theMonitor->queryBinding(targetId.str());
                    if(theBinding)
                    {
                        DBGLOG("Requesting clearing of binding %s", targetId.str());
                        queryEspServer()->removeBinding(port, *theBinding);
                        m_theMonitor->removeBindingFromMap(targetId.str());
                    }
                    else
                        DBGLOG("Can't delete binding %s, it doesn't exist", targetId.str());
                }
                else
                {
                    bool isAdd = false;
                    if(changeAction.length() > 0 && strcmp(changeAction.str(), "add") == 0)
                        isAdd = true;

                    CriticalBlock cb(m_theMonitor->queryCritSect());
                    theBinding = m_theMonitor->queryBinding(targetId.str());
                    if(theBinding != NULL && isAdd)
                    {
                        DBGLOG("Binding %s already exists, can't add", targetId.str());
                        return;
                    }
                    if(!theBinding && !isAdd)
                    {
                        DBGLOG("Binding %s not found, can't update", targetId.str());
                        return;
                    }

                    //Yanrui TODO:
                    // - [X] Better way to instantiate binding and service. Is PropertyTree config required? If yes, what's best way to create this tree,
                    //   in an efficient and re-usable way?
                    // - [X] Change reloading bindings to re-creating and replacing, to guarantee completion of requests in flight.
                    // - [X] Best way to implement the ability to hold binding and service for requests in flight. Holding a reference count in
                    //   the threads might be sufficient? --- Reference count increments in request threads, plus delayed release.
                    // - [X] Need to add removeBinding method to IEspServer/CEspServer
                    // - [X] Dali notification issue
                    // - Load pure dynamic bindings at start up (where is the best place to do this?)
                    // - Need to think about how to identify the binding, bindingId, port or both.
                    // - [X] Make sure binding and service destructors are called, make sure sockets and ports are re-collected when there's no more bindings on the port
                    // - [X?] Examine critical sections usage
                    // - [X] Should not allow adding 2 bindings with the same id.
                    // - ESDL Monitor really should be ESP monitor. There's no reason why non-DESDL services and bindings can not be loaded dynamically. (feature for 8.0 maybe?)
                    // - Need to change ws_esdlconfig to publish binding when there's no static binding. Add attributes like protocol
                    // - [X?] Shutdown memory usage and core
                    DBGLOG("Creating new binding %s", targetId.str());
                    StringBuffer serviceName;
                    Owned<IPropertyTree> envpt = m_theMonitor->getEnvpt(props.get(), serviceName);
                    IEspServer* server = queryEspServer();
                    IEspProtocol* espProtocol = server->queryProtocol(protocol);
                    Owned<EsdlBindingImpl> esdlbinding = new CEsdlSvcEngineSoapBindingEx(envpt,  targetName.str(), espProcess.str());
                    if(isAdd)
                        m_theMonitor->registerBinding(targetId.str(), esdlbinding.get());
                    Owned<EsdlServiceImpl> esdlservice = new CEsdlSvcEngine();
                    esdlservice->init(envpt, espProcess.str(), serviceName.str());
                    esdlbinding->addService(esdlservice->getServiceType(), nullptr, port, *esdlservice.get());
                    esdlbinding->addProtocol(protocol, *espProtocol);
                    server->addBinding(targetName.str(), nullptr, port, *espProtocol, *esdlbinding.get(), false, envpt);

                    if(theBinding)
                    {
                        DBGLOG("Removing previous binding...");
                        //theBinding->reloadBindingFromDali(targetId.str());
                        server->removeBinding(port, *theBinding);
                    }
                }
            }
        }
private:
        void parseChangeString(int len, const char* changeStr, IProperties& props)
        {
            if(len == 0 || !changeStr || !*changeStr)
                return;
            StringBuffer buf(len, changeStr);
            buf.replace(';', '\n');
            props.loadProps(buf.str());
        }
    };

    class CESDLBindingSubscription : public CInterface, implements ISDSSubscription
    {
    private:
        CriticalSection daliSubscriptionCritSec;
        SubscriptionId sub_id;
        CEsdlMonitor* m_theMonitor;
    public:
        IMPLEMENT_IINTERFACE;

        //TODO:
        // - When set subscribe's sendData to true, won't get notifications
        // - When add a binding, still reports as "SDSNotify_Deleted"? Acutally seems to be getting "changed" update
        CESDLBindingSubscription(CEsdlMonitor* theMonitor) : m_theMonitor(theMonitor)
        {
            CriticalBlock b(daliSubscriptionCritSec);
            try
            {
                sub_id = querySDS().subscribe(ESDL_BINDING_PATH, *this, true);
            }
            catch (IException *E)
            {
                DBGLOG("Failed to subscribe to DALI (%s)", ESDL_BINDING_PATH);
                // failure to subscribe implies dali is down... is this ok??
                // Is this bad enough to halt the esp load process??
                E->Release();
            }
        }

        virtual ~CESDLBindingSubscription()
        {
            unsubscribe();
        }

        void unsubscribe()
        {
            CriticalBlock b(daliSubscriptionCritSec);
            try
            {
                if (sub_id)
                {
                    querySDS().unsubscribe(sub_id);
                    sub_id = 0;
                }
            }
            catch (IException *E)
            {
                E->Release();
            }
            sub_id = 0;
        }

        void notify(SubscriptionId id, const char *xpath, SDSNotifyFlags flags, unsigned valueLen=0, const void *valueData=NULL)
        {
            if (id != sub_id)
            {
                DBGLOG("Dali subscription (%" I64F "d) received notification for unrecognized dali subscription id: (%" I64F "d)", (__int64) sub_id, (__int64) id);
                return;
            }

            DBGLOG("ESDL binding change reported to binding %s - flags = %d", xpath, flags);

            StringBuffer parentElementXPath;
            //path is reported with sibbling number annotation ie /ESDL/Bindings/Binding[2]/...
            if(!trimXPathToParentSDSElement("Binding[", xpath, parentElementXPath))
                return;

            StringBuffer bindingName;
            StringBuffer processName;
            Owned<IRemoteConnection> conn = querySDS().connect(parentElementXPath.str(), myProcessSession(), RTM_LOCK_READ, SDS_LOCK_TIMEOUT_DESDL);
            EsdlBindingImpl* theBinding = NULL;
            if (!conn)
            {
                DBGLOG("Can't find path %s on dali", parentElementXPath.str());
                //Can't find this path, is this a delete?
                if (flags == SDSNotify_Deleted)
                {
                    DBGLOG("Received notification that ESDL binding is deleted. Checking the deleted bindings...");
                    StringArray bindingIds;
                    m_theMonitor->findDeletedBindings(bindingIds);
                    ForEachItemIn(x, bindingIds)
                    {
                        const char* bindingId = bindingIds.item(x);
                        DBGLOG("ESDL binding %s deleted.", bindingId);
                        theBinding = m_theMonitor->queryBinding(bindingId);
                        if(theBinding)
                        {
                            DBGLOG("Requesting clearing of %s.%s binding...", processName.str(), bindingName.str() );
                            CriticalBlock b(daliSubscriptionCritSec);
                            theBinding->clearBindingState();
                        }
                    }
                    return;
                    //TODO: need to deal with delete
                }
                else
                    return;
            }
            else
            {
                IPropertyTree * bindingSubscription = conn->queryRoot();
                if (!bindingSubscription)
                    return;

                conn->close(false); //release lock right away

                bindingName = bindingSubscription->queryProp("@espbinding");
                if (bindingName.length() == 0)
                    return;

                processName = bindingSubscription->queryProp("@espprocess");
                if (processName.length() == 0)
                    return;

                const char* esdlBindingId = bindingSubscription->queryProp("@id");
                if(esdlBindingId)
                {
                    theBinding = m_theMonitor->queryBinding(esdlBindingId);
                }
            }

            if(theBinding)
            {
                DBGLOG("Requesting reload of %s.%s binding...", processName.str(), bindingName.str() );
                CriticalBlock b(daliSubscriptionCritSec);
                theBinding->reloadBindingFromDali(bindingName.str(), processName.str());
            }
        }
    };

    class CESDLDefinitionSubscription : public CInterface, implements ISDSSubscription
    {
    private :
        CriticalSection daliSubscriptionCritSec;
        SubscriptionId sub_id;
        CEsdlMonitor* m_theMonitor;
    public:
        IMPLEMENT_IINTERFACE;

        CESDLDefinitionSubscription(CEsdlMonitor* theMonitor) : m_theMonitor(theMonitor)
        {
            //for some reason subscriptions based on xpaths with attributes don't seem to work correctly
            //fullBindingPath.set("/ESDL/Bindings/Binding[@EspBinding=\'WsAccurint\'][@EspProcess=\'myesp\']");
            CriticalBlock b(daliSubscriptionCritSec);
            try
            {
                sub_id = querySDS().subscribe(ESDL_DEF_PATH, *this, true);
            }
            catch (IException *E)
            {
                // failure to subscribe implies dali is down... is this ok??
                // Is this bad enough to halt the esp load process??
                E->Release();
            }
        }

        virtual ~CESDLDefinitionSubscription()
        {
            unsubscribe();
        }

        void unsubscribe()
        {
            CriticalBlock b(daliSubscriptionCritSec);
            try
            {
                if (sub_id)
                {
                    querySDS().unsubscribe(sub_id);
                    sub_id = 0;
                }
            }
            catch (IException *E)
            {
                E->Release();
            }
            sub_id = 0;
        }

        void notify(SubscriptionId id, const char *xpath, SDSNotifyFlags flags, unsigned valueLen=0, const void *valueData=NULL)
        {
            if (id != sub_id)
            {
                DBGLOG("Dali subscription (%" I64F "d) received notification for unrecognized dali subscription id: (%" I64F "d)", (__int64) sub_id, (__int64) id);
                return;
            }

            DBGLOG("ESDL definition change reported for %s - flags = %d", xpath, flags);


            StringBuffer parentElementXPath;
            //path is reported with sibbling number annotation ie /ESDL/Definitions/Definition[2]/...
            if(!trimXPathToParentSDSElement("Definition[", xpath, parentElementXPath))
                return;

            Owned<IRemoteConnection> conn = querySDS().connect(parentElementXPath.str(), myProcessSession(), RTM_LOCK_READ, SDS_LOCK_TIMEOUT_DESDL);
            if (!conn)
                return;

            conn->close(false); //release lock right away

            IPropertyTree * definitionSubscription = conn->queryRoot();
            if (!definitionSubscription)
                return;

            const char * definitionId = definitionSubscription->queryProp("@id");
            if (!definitionId || !*definitionId)
                return;

            CriticalBlock b(daliSubscriptionCritSec);
            HashIterator it(m_theMonitor->queryBindingMap()); //package bases can be across parts
            ForEach (it)
            {
                const char* bindingId = (const char *)it.query().getKey();
                EsdlBindingImpl* theBinding =  m_theMonitor->queryBinding(bindingId);
                //we reload all definitions used by this binding because the
                //ESDL object intermingles all nodes of all definitions used.
                if (theBinding && theBinding->usesESDLDefinition(definitionId))
                {
                    DBGLOG("Requesting reload of ESDL definitions for binding %s...", bindingId);
                    StringBuffer loaded;
                    theBinding->reloadBindingFromDali(bindingId);
                }
            }
        }

    };

    void findDeletedBindings(StringArray& bindingIds)
    {
        Owned<IRemoteConnection> conn = querySDS().connect(ESDL_BINDINGS_ROOT_PATH, myProcessSession(), RTM_LOCK_READ, SDS_LOCK_TIMEOUT_DESDL);
        if (!conn)
           throw MakeStringException(-1, "Unable to connect to ESDL bindings information in dali '%s'", ESDL_BINDINGS_ROOT_PATH);

        conn->close(false); //release lock right away

        IPropertyTree * esdlBindings = conn->queryRoot();
        if (!esdlBindings)
           throw MakeStringException(-1, "Unable to open ESDL bindings information in dali '%s'", ESDL_BINDINGS_ROOT_PATH);

        //There shouldn't be multiple entries here, but if so, we'll use the first one
        HashIterator it(m_esdlBindingMap); //package bases can be across parts
        ForEach (it)
        {
            const char* id = (const char *)it.query().getKey();
            VStringBuffer xpath("%s[@id='%s'][1]", ESDL_BINDING_ENTRY, id);
            if(esdlBindings->queryPropTree(xpath.str()) == NULL)
                bindingIds.append(id);
        }
    }
    //Yanrui TODO: need to take care of all available attributes!
    void constructEnvptHeader()
    {
        mEnvptHeader.clear().append("<Environment><Software><EspProcess ");
        IPropertyTree* envpt = queryEspServer()->queryEnvpt();
        if(envpt)
        {
            //Yanrui TODO: don't think there could be more than 1 Esp Process within 1 esp config file, right?
            IPropertyTree* espprocpt = envpt->queryPropTree("Software/EspProcess");
            if(espprocpt)
            {
                Owned<IAttributeIterator> attrs = espprocpt->getAttributes(false);
                for(attrs->first(); attrs->isValid(); attrs->next())
                {
                    StringBuffer name(attrs->queryName());
                    StringBuffer value;
                    encodeXML(attrs->queryValue(), value);
                    if(name.length() > 1)
                        mEnvptHeader.appendf("%s=\"%s\" ", name.str()+1, value.str());
                }
            }
        }
        mEnvptHeader.append(">");
    }

    IPropertyTree* getEnvpt(IProperties* changeEntry, StringBuffer& serviceName)
    {
        return getEnvpt(changeEntry->queryProp("espProcess"), changeEntry->queryProp("targetName"), changeEntry->queryProp("protocol"),
                changeEntry->queryProp("port"), serviceName);
    }

    /* Sample binding:
    <Binding created="2017-09-29T19:05:50"
              espbinding="DESDLBinding2"
              espprocess="myesp"
              id="myesp.DESDLBinding2"
              port="8004"
              publishedBy="Anonymous">
      <Definition esdlservice="EsdlExample" id="esdlexample.1" name="esdlexample">
       <Methods>
        <Method javamethod="EsdlExample.EsdlExampleService.JavaEchoPersonInfo" name="JavaEchoPersonInfo"
     querytype="java"/>
        <Method name="RoxieEchoPersonInfo"
                queryname="RoxieEchoPersonInfo"
                querytype="roxie"
                url="http://localhost:9876/roxie"/>
       </Methods>
      </Definition>
     </Binding>
    */
    IPropertyTree* getEnvpt(IPropertyTree* changeEntry, StringBuffer& serviceName)
    {
        return getEnvpt(changeEntry->queryProp("@espprocess"), changeEntry->queryProp("@espbinding"), changeEntry->queryProp("@protocol"),
                changeEntry->queryProp("@port"), serviceName);
    }

    IPropertyTree* getEnvpt(const char* espProcess, const char* bindingName, const char* protocol, const char* port, StringBuffer& serviceName)
    {
        if(!protocol || !*protocol)
        {
            DBGLOG("Protocol not specified for binding %s, use http as default.", bindingName);
            protocol = "http";
        }
        IPropertyTree* envpt = queryEspServer()->queryEnvpt();
        if(envpt)
        {
            VStringBuffer xpath("Software/EspProcess[name='%s']/EspBinding[name='%s']",
                    espProcess, bindingName);
            if(envpt->queryProp(xpath.str()))
                return LINK(envpt);
        }
        serviceName.appendf("ServiceFor%s", bindingName);
        StringBuffer envxmlbuf;
        envxmlbuf.appendf("%s<EspBinding defaultForPort=\"true\""
                " defaultServiceVersion=\"\""
                " name=\"%s\""
                " port=\"%s\""
                " protocol=\"%s\""
                " resourcesBasedn=\"ou=EspServices,ou=ecl\""
                " service=\"%s\"/>\n"
                " <EspService build=\"_\""
                " buildSet=\"DynamicESDL\""
                " description=\"My ESDL Based Web Service Interface\""
                " LoggingManager=\"\""
                " name=\"%s\""
                " type=\"%s\""
                " namespaceBase=\"urn:hpccsystems:ws\">"
                " <Properties bindingType=\"EsdlBinding\""
                " defaultPort=\"8043\""
                "  defaultResourcesBasedn=\"ou=EspServices,ou=ecl\""
                "  defaultSecurePort=\"18043\""
                "  plugin=\"esdl_svc_engine\""
                "  type=\"DynamicESDL\"/>"
                " </EspService>\n"
                "</EspProcess></Software></Environment>",
                mEnvptHeader.str(), bindingName, port, protocol, bindingName, serviceName.str(), serviceName.str());
        //envxmlbuf.loadFile("/home/mayx/git/ecrun/opt/HPCCSystems/examples/EsdlExampleYanrui/esp.xml", false);
        DBGLOG("Constructed configuration: \n------\n%s\n-------\n", envxmlbuf.str());
        return createPTreeFromXMLString(envxmlbuf.str());
    }
};

static Owned<IEsdlMonitor> gEsdlMonitor;
static bool isEsdlMonitorInitted = false;

extern "C" esdl_decl void initEsdlMonitor()
{
    CriticalBlock cb(gEsdlMonitorCritSection);
    if(gEsdlMonitor.get() == nullptr)
    {
        CEsdlMonitor* monitor = new CEsdlMonitor();
        gEsdlMonitor.setown(monitor);
        isEsdlMonitorInitted = true;
        monitor->loadDynamicBindings();
        monitor->setupSubscription();
    }
}

esdl_decl IEsdlMonitor* queryEsdlMonitor()
{
    if(!isEsdlMonitorInitted)
        initEsdlMonitor();
    return gEsdlMonitor.get();
}

esdl_decl IEsdlCache* queryEsdlCache()
{
    if(!isEsdlMonitorInitted)
        initEsdlMonitor();
    return gEsdlMonitor->queryEsdlCache();
}
