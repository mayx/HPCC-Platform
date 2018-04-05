/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2017 HPCC Systems®.

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

#include "jexcept.hpp"
#include "jsmartsock.hpp"
#include "dasds.hpp"
#include "daclient.hpp"
#include "dalienv.hpp"
#include "dadfs.hpp"
#include "dasess.hpp"
#include "dautils.hpp"
#include "wsexcept.hpp"
#include "httpclient.hpp"
#include "espcontext.hpp"
#include "esdl_store.hpp"
#include "esdl_binding.hpp"
#include <memory>

static const char* ESDL_DEFS_ROOT_PATH="/ESDL/Definitions/";
static const char* ESDL_DEF_PATH="/ESDL/Definitions/Definition";
static const char* ESDL_DEF_ENTRY="Definition";
static const char* ESDL_BINDINGS_ROOT_PATH="/ESDL/Bindings/";
static const char* ESDL_BINDING_PATH="/ESDL/Bindings/Binding";
static const char* ESDL_BINDING_ENTRY="Binding";
static const char* ESDL_CHANGE_PATH="/ESDL/Subscription/Change";

extern bool trimXPathToParentSDSElement(const char *element, const char * xpath, StringBuffer & parentNodeXPath);

class CEsdlSDSStore : implements IEsdlStore, public CInterface
{
public:
    IMPLEMENT_IINTERFACE;

    CEsdlSDSStore()
    {
        ensureSDSPath(ESDL_DEFS_ROOT_PATH);
        ensureSDSPath(ESDL_BINDINGS_ROOT_PATH);
    }
    virtual ~CEsdlSDSStore() { }

    virtual void fetchDefinition(const char* definitionId, StringBuffer& esxdl) override
    {
        if (!definitionId || !*definitionId)
            throw MakeStringException(-1, "Unable to fetch ESDL Service definition information, definition id is not available");

        DBGLOG("ESDL Binding: Fetching ESDL Definition from Dali: %s ", definitionId);

        VStringBuffer xpath("%s[@id='%s'][1]/esxdl", ESDL_DEF_PATH, definitionId);
        Owned<IRemoteConnection> conn = querySDS().connect(xpath.str(), myProcessSession(), RTM_LOCK_READ, SDS_LOCK_TIMEOUT_DESDL);
        if (!conn)
           throw MakeStringException(-1, "Unable to connect to ESDL Service definition information in dali '%s'", xpath.str());

        conn->close(false); //release lock right away

        IPropertyTree * deftree = conn->queryRoot();
        if (!deftree)
           throw MakeStringException(-1, "Unable to open ESDL Service definition information in dali '%s'", xpath.str());

        //There shouldn't be multiple entries here, but if so, we'll use the first one
        toXML(deftree, esxdl, 0, 0);
    }

    virtual void fetchLatestDefinition(const char* definitionName, StringBuffer& esxdl) override
    {
        if (!definitionName || !*definitionName)
            throw MakeStringException(-1, "Unable to fetch ESDL Service definition information, definition name is not available");

        DBGLOG("ESDL Binding: Fetching ESDL Definition from Dali based on name: %s ", definitionName);

        Owned<IRemoteConnection> conn = querySDS().connect(ESDL_DEFS_ROOT_PATH, myProcessSession(), RTM_LOCK_READ, SDS_LOCK_TIMEOUT_DESDL);
        if (!conn)
           throw MakeStringException(-1, "Unable to connect to ESDL Service definition information in dali '%s'", ESDL_DEFS_ROOT_PATH);

        IPropertyTree * esdlDefinitions = conn->queryRoot();

        VStringBuffer xpath("%s[@name='%s']", ESDL_DEF_ENTRY, definitionName);
        Owned<IPropertyTreeIterator> iter = esdlDefinitions->getElements(xpath.str());

        unsigned latestSeq = 1;
        ForEach(*iter)
        {
            IPropertyTree &item = iter->query();
            unsigned thisSeq = item.getPropInt("@seq");
            if (thisSeq > latestSeq)
                latestSeq = thisSeq;
        }

        xpath.setf("%s[@id='%s.%d'][1]/esxdl", ESDL_DEF_ENTRY, definitionName, latestSeq);
        IPropertyTree * deftree = esdlDefinitions->getPropTree(xpath);
        if (deftree)
            toXML(deftree, esxdl, 0,0);
        else
            throw MakeStringException(-1, "Unable to fetch ESDL Service definition from dali: '%s'", definitionName);
    }

    virtual IPropertyTree* fetchBinding(const char* espProcess, const char* espStaticBinding) override
    {
        //There shouldn't be multiple entries here, but if so, we'll use the first one
        VStringBuffer xpath("%s[@espprocess='%s'][@espbinding='%s'][1]", ESDL_BINDING_ENTRY, espProcess, espStaticBinding);
        try
        {
            Owned<IRemoteConnection> conn = querySDS().connect(ESDL_BINDINGS_ROOT_PATH, myProcessSession(), RTM_LOCK_READ, SDS_LOCK_TIMEOUT_DESDL);
            if (!conn)
            {
                ESPLOG(LogMin, "Unable to connect to ESDL Service binding information in dali %s", ESDL_BINDINGS_ROOT_PATH);
                return nullptr;
            }

            ESPLOG(LogNormal, "ESDL Binding: Fetching ESDL Binding from Dali %s", xpath.str());

            if (conn->queryRoot()->hasProp(xpath))
                return createPTreeFromIPT(conn->queryRoot()->queryPropTree(xpath));
            else
                return nullptr;
        }
        catch (IException *E)
        {
            VStringBuffer message("ESDL Binding: Error fetching ESDL Binding from Dali %s", xpath.str());
            EXCLOG(E, message);
            E->Release();
        }
        catch(...)
        {
            ESPLOG(LogMin, "ESDL Binding: Unknown error encountered while fetching ESDL Binding from Dali %s", xpath.str());
        }

        return nullptr;
    }

    virtual IPropertyTree* fetchBinding(const char* bindingId) override
    {
        try
        {
            //There shouldn't be multiple entries here, but if so, we'll use the first one
            VStringBuffer xpath("%s[@id='%s'][1]", ESDL_BINDING_ENTRY, bindingId);

            Owned<IRemoteConnection> conn = querySDS().connect(ESDL_BINDINGS_ROOT_PATH, myProcessSession(), RTM_LOCK_READ, SDS_LOCK_TIMEOUT_DESDL);
            if (!conn)
            {
                ESPLOG(LogMin, "Unable to connect to ESDL Service binding information in dali %s", ESDL_BINDINGS_ROOT_PATH);
                return nullptr;
            }

            ESPLOG(LogNormal, "ESDL Binding: Fetching ESDL Binding from Dali %s[@id='%s'][1]", ESDL_BINDING_ENTRY, bindingId);

            if (conn->queryRoot()->hasProp(xpath))
                return createPTreeFromIPT(conn->queryRoot()->queryPropTree(xpath));
            else
                return nullptr;
        }
        catch (IException *E)
        {
            VStringBuffer message("ESDL Binding: Error fetching ESDL Binding %s[@id='%s'][1] from Dali.", ESDL_BINDING_ENTRY, bindingId);
            EXCLOG(E, message);
            E->Release();
        }
        catch(...)
        {
            ESPLOG(LogMin, "ESDL Binding: Unknown error encountered while fetching ESDL Binding from Dali %s[@id='%s'][1]", ESDL_BINDING_ENTRY, bindingId);
        }

        return nullptr;
    }

    virtual bool addDefinition(IPropertyTree* definitionRegistry, const char* definitionName, IPropertyTree* definitionInfo, StringBuffer& newId, unsigned& newSeq, const char* userid, bool deleteprev, StringBuffer & message) override
    {
        StringBuffer lcName(definitionName);
        lcName.toLowerCase();
        StringBuffer xpath;
        xpath.append(ESDL_DEF_ENTRY).append("[@name=\"").append(lcName.str()).append("\"]");

        Owned<IPropertyTreeIterator> iter = definitionRegistry->getElements(xpath);

        newSeq = 1;
        ForEach(*iter)
        {
            IPropertyTree &item = iter->query();
            unsigned thisSeq = item.getPropInt("@seq");
            if (thisSeq >= newSeq)
                newSeq = thisSeq + 1;
        }

        StringBuffer origTimestamp;
        StringBuffer origOwner;

        if (deleteprev && newSeq > 1)
        {
            if (!isDefinitionBound(lcName, newSeq -1))
            {
                newSeq--;
                xpath.appendf("[@seq='%d']", newSeq);

                IPropertyTree * definition = definitionRegistry->queryPropTree(xpath);
                if (definition)
                {
                    origTimestamp.set(definition->queryProp("@created"));
                    origOwner.set(definition->queryProp("@publishedBy"));
                    definitionRegistry->removeTree(definition);
                }
                else
                {
                    message.setf("Could not overwrite Definition: '%s.%d'", definitionName, newSeq);
                    ESPLOG(LogMin, "%s", message.str());
                    return false;
                }
            }
            else
            {
                message.setf("Will not delete previous ESDL definition version because it is referenced in an ESDL binding.");
                ESPLOG(LogMin, "%s", message.str());
                return false;
            }
        }

        CDateTime dt;
        dt.setNow();
        StringBuffer str;

        newId.set(lcName).append(".").append(newSeq);
        definitionInfo->setProp("@name", lcName);
        definitionInfo->setProp("@id", newId);
        definitionInfo->setPropInt("@seq", newSeq);
        if (origOwner.length())
        {
            definitionInfo->setProp("@lastEditedBy", (userid && *userid) ? userid : "Anonymous") ;
            definitionInfo->setProp("@publishedBy", origOwner.str()) ;
        }
        else
            definitionInfo->setProp("@publishedBy", (userid && *userid) ? userid : "Anonymous") ;

        if (origTimestamp.length())
        {
            definitionInfo->setProp("@created", origTimestamp.str());
            definitionInfo->setProp("@lastEdit",dt.getString(str).str());
        }
        else
            definitionInfo->setProp("@created",dt.getString(str).str());

        definitionRegistry->addPropTree(ESDL_DEF_ENTRY, LINK(definitionInfo));
        return true;
    }

    virtual IPropertyTree* getDefinitionRegistry(bool readonly) override
    {
        Owned<IRemoteConnection> conn = querySDS().connect(ESDL_DEFS_ROOT_PATH, myProcessSession(), readonly ? RTM_LOCK_READ : RTM_LOCK_WRITE, SDS_LOCK_TIMEOUT_DESDL);
        if (!conn)
            throw MakeStringException(-1, "Unexpected error while attempting to access ESDL definition dali registry.");
        return (conn) ? conn->getRoot() : NULL;
    }

    virtual bool definitionExists(const char* definitionId) override
    {
        bool found = false;
        if (!definitionId)
            return found;

        StringBuffer lcid (definitionId);
        lcid.toLowerCase();
        VStringBuffer xpath("%s[@id='%s']", ESDL_DEF_PATH, lcid.str());
        Owned<IRemoteConnection> globalLock = querySDS().connect(xpath.str(), myProcessSession(), RTM_LOCK_READ, SDS_LOCK_TIMEOUT_DESDL);

        if (globalLock)
        {
            found = true;
            globalLock->close(false);
        }

        return found;
    }

    virtual bool isMethodDefined(const char* definitionId, StringBuffer& esdlServiceName, const char* methodName) override
    {
        bool found = false;
        if (!definitionId || !*definitionId)
            return found;

        StringBuffer lcdefid (definitionId);
        lcdefid.toLowerCase();
        VStringBuffer xpath("%s[@id='%s']/esxdl", ESDL_DEF_PATH, lcdefid.str());
        Owned<IRemoteConnection> globalLock = querySDS().connect(xpath.str(), myProcessSession(), RTM_LOCK_READ, SDS_LOCK_TIMEOUT_DESDL);

        if (globalLock)
        {
            IPropertyTree * esxdl = globalLock->queryRoot();
            if (esxdl)
            {
                Owned<IPropertyTreeIterator> it = esxdl->getElements("EsdlService");
                int servicesCount = esxdl->getCount("EsdlService");

                ForEach(*it)
                {
                    IPropertyTree* pCurrService = &it->query();
                    if ((servicesCount == 1 && !esdlServiceName.length()) || stricmp(pCurrService->queryProp("@name"), esdlServiceName.str())==0)
                    {
                        Owned<IPropertyTreeIterator> it2 = pCurrService->getElements("EsdlMethod");
                        ForEach(*it2)
                        {
                            IPropertyTree* pChildNode = &it2->query();
                            if (stricmp(pChildNode->queryProp("@name"), methodName)==0)
                            {
                                if (!esdlServiceName.length())
                                    esdlServiceName.set(pCurrService->queryProp("@name"));
                                found = true;
                                break;
                            }
                        }
                    }
                    if (found)
                        break;
                }
            }

            globalLock->close(false);
        }
        globalLock.clear();
        return found;
    }
    virtual int configureMethod(const char* bindingId, const char* methodName, IPropertyTree* configTree, bool overwrite, StringBuffer& message) override
    {
        if (!bindingId || !*bindingId)
        {
            message.set("Unable to configure method, binding id must be provided");
            return -1;
        }
        if (!configTree)
        {
            message.setf("Unable to configure method '%s', configuration attributes must be provided", methodName);
            return -1;
        }

        VStringBuffer rxpath("%sBinding[@id='%s']/Definition/Methods[1]", ESDL_BINDINGS_ROOT_PATH, bindingId);

        Owned<IRemoteConnection> conn;

        try
        {
            conn.setown(querySDS().connect(rxpath, myProcessSession(), RTM_LOCK_WRITE, SDS_LOCK_TIMEOUT_DESDL));
        }
        catch (ISDSException * e)
        {
            if (conn)
                conn->close(true);

            message.setf("Unable to operate on Dali path: %s", rxpath.str());
            e->Release();
            return -1;
        }

        //Only lock the branch for the target we're interested in.
        if (!conn)
            throw MakeStringException(-1, "Unable to connect to %s", rxpath.str());

        Owned<IPropertyTree> root = conn->getRoot();
        if (!root.get())
            throw MakeStringException(-1, "Unable to open %s", rxpath.str());

        VStringBuffer xpath("Method[@name='%s']", methodName);
        Owned<IPropertyTree> oldEnvironment = root->getPropTree(xpath.str());
        if (oldEnvironment.get())
        {
            if (overwrite)
            {
                message.set("Existing method configuration overwritten!");
                root->removeTree(oldEnvironment);
            }
            else
            {
                message.set("Method configuration exists will not overwrite!");
                return -1;
            }
        }

        root->addPropTree("Method", configTree);

        conn->commit();
        conn->close(false);

        VStringBuffer changestr("action=update;type=binding;targetId=%s", bindingId);
        triggerSubscription(changestr.str());

        message.appendf("\nSuccessfully configured Method '%s' for binding '%s'", methodName, bindingId);
        return 0;
    }

    //Yanrui TODO:
    // - GetEsdlBinding esp method doesn't fill in a lot of the response fields.
    virtual int bindService(const char* bindingName,
                                             IPropertyTree* methodsConfig,
                                             const char* espProcName,
                                             const char* espPort,
                                             const char* definitionId,
                                             const char* esdlServiceName,
                                             StringBuffer& message,
                                             bool overwrite,
                                             const char* user) override
    {
        if(!espProcExists(espProcName))
        {
            message.setf("Esp process %s not found in the environment, please double check the case-sensitive spelling", espProcName);
            return -1;
        }

        if ((!bindingName || !*bindingName) && (!espPort || !*espPort))
        {
            message.setf("Could not configure '%s' - need target binding name or port", esdlServiceName);
            return -1;
        }

        if (!definitionId || !*definitionId)
        {
            message.set("Could not configure DESDL service: Target Esdl definition id not available");
            return -1;
        }

        if(!definitionExists(definitionId))
        {
            message.setf("Esdl definition %s doesn't exist, please double check the spelling", definitionId);
            return -1;
        }

        if (!esdlServiceName || !*esdlServiceName)
        {
            message.set("Could not configure DESDL service: Target Esdl definition service name not available");
            return -1;
        }

        if(!isEsdlServiceDefined(definitionId, esdlServiceName))
        {
            message.setf("Esdl service %s is not defined in %s, please double check the case sensitive service name", esdlServiceName, definitionId);
            return -1;
        }

        //Get static binding if exists
        Owned<IPropertyTree> bindingcfg = getEspBindingConfig(espProcName, espPort, bindingName);
        StringBuffer duplicateBindingId;
        if(bindingName && *bindingName)
        {
            StringBuffer msg;
            Owned<IPropertyTree> esdlbindingtree = getBindingTree(espProcName, bindingName, msg);
            if(esdlbindingtree)
            {
                DBGLOG("Found esdl binding for name %s", bindingName);
                const char* existingPort = esdlbindingtree->queryProp("@port");
                if(!overwrite)
                {
                    message.setf("ESDL binding %s already exists", bindingName);
                    return -1;
                }
                else if(espPort && *espPort)
                {
                    if(existingPort && *existingPort && strcmp(esdlbindingtree->queryProp("@port"), espPort) != 0)
                    {
                        message.setf("Port provided %s doesn't match what's already defined in existing binding %s", espPort, existingPort);
                        return -1;
                    }
                }
                if(!espPort || !*espPort)
                    espPort = existingPort;
                duplicateBindingId.set(esdlbindingtree->queryProp("@id"));
            }
            else if(bindingcfg)
            {
                DBGLOG("Static esp binding configured for %s", bindingName);
                if(!espPort || !*espPort)
                    espPort = bindingcfg->queryProp("@port");
            }
            else if(!espPort || !*espPort)
            {
               message.set("Port must be provided");
               return -1;
            }
        }
        else //espPort provided
        {
            if(bindingcfg)
            {
                const char* bn = bindingcfg->queryProp("@name");
                DBGLOG("Static esp binding %s configured for port %s", bn, espPort);
                StringBuffer msg;
                Owned<IPropertyTree> esdlbindingtree = getBindingTree(espProcName, bn, msg);
                if(!esdlbindingtree)
                {
                    DBGLOG("There's currently no esdl binding for this esp binding, so bind to it.");
                    bindingName = bn;
                }
            }
        }
        Owned<IRemoteConnection> conn = querySDS().connect(ESDL_BINDINGS_ROOT_PATH, myProcessSession(), RTM_LOCK_WRITE | RTM_CREATE_QUERY, SDS_LOCK_TIMEOUT_DESDL);
        if (!conn)
           throw MakeStringException(-1, "Unexpected error while attempting to access ESDL definition dali registry.");

        IPropertyTree * bindings = conn->queryRoot();

        IPropertyTree* duplicateBinding = nullptr;
        if(duplicateBindingId.length() == 0)
        {
            VStringBuffer xpath("%s[@espprocess='%s'][@port='%s']", ESDL_BINDING_ENTRY, espProcName, espPort);
            Owned<IPropertyTreeIterator> bi = bindings->getElements(xpath.str());
            for (bi->first(); bi->isValid(); bi->next())
            {
                IPropertyTree& b = bi->query();
                VStringBuffer defpath("Definition[@esdlservice='%s']", esdlServiceName);
                if(b.hasProp(defpath.str()))
                    duplicateBinding = &b;
            }
        }
        else
        {
            VStringBuffer xpath("%s[@id='%s']", ESDL_BINDING_ENTRY, duplicateBindingId.str());
            duplicateBinding = bindings->queryPropTree(xpath.str());
        }

        StringBuffer origTimestamp;
        StringBuffer origOwner;

        if (duplicateBinding)
        {
            if (overwrite)
            {
                origTimestamp.set(duplicateBinding->queryProp("@created"));
                origOwner.set(duplicateBinding->queryProp("@publishedBy"));
                bindings->removeTree(duplicateBinding);
            }
            else
            {
               message.setf("Could not configure Service '%s' because this service has already been configured for ESP Process '%s' on port %s", esdlServiceName, espProcName, espPort);
               conn->close(false);
               return -1;
            }
        }

        VStringBuffer qbindingid("%s.%s.%s", espProcName, espPort, esdlServiceName);
        if(!bindingName || !*bindingName)
            bindingName = qbindingid.str();
        Owned<IPropertyTree> bindingtree  = createPTree();
        bindingtree->setProp("@espprocess", espProcName);
        bindingtree->setProp("@port", espPort);
        bindingtree->setProp("@espbinding", bindingName);
        bindingtree->setProp("@id", qbindingid.str());

        CDateTime dt;
        dt.setNow();
        StringBuffer str;

        if (origTimestamp.length())
        {
            bindingtree->setProp("@created", origTimestamp.str());
            bindingtree->setProp("@lastEdit", dt.getString(str).str());
        }
        else
            bindingtree->setProp("@created",  dt.getString(str).str());

        if (origOwner.length())
        {
            bindingtree->setProp("@publisheBy", origOwner.str()) ;
            bindingtree->setProp("@lastEditedBy", (user && *user) ? user : "Anonymous");
        }
        else
            bindingtree->setProp("@publishedBy", (user && *user) ? user : "Anonymous") ;

        StringBuffer lcId(definitionId);
        lcId.toLowerCase();
        StringBuffer esdlDefinitionName;
        int esdlver = 0;
        const char * esdlDefId = definitionId;
        while(esdlDefId && *esdlDefId && *esdlDefId != '.')
            esdlDefinitionName.append(*esdlDefId++);
        if(esdlDefId && *esdlDefId == '.')
            esdlDefId++;
        if (esdlDefId && *esdlDefId)
            esdlver = atoi(esdlDefId);
        if (esdlver <= 0)
            esdlver = 1;
        StringBuffer newId;
        newId.set(esdlDefinitionName).append(".").append(esdlver);

        Owned<IPropertyTree> esdldeftree  = createPTree();

        esdldeftree->setProp("@name", esdlDefinitionName.str());
        esdldeftree->setProp("@id", newId);
        esdldeftree->setProp("@esdlservice", esdlServiceName);


        if(methodsConfig != nullptr)
            esdldeftree->addPropTree("Methods", LINK(methodsConfig));
        else
            esdldeftree->addPropTree("Methods");

        bindingtree->addPropTree(ESDL_DEF_ENTRY, LINK(esdldeftree));
        bindings->addPropTree(ESDL_BINDING_ENTRY, LINK(bindingtree));

        conn->commit();
        conn->close(false);

        VStringBuffer changestr("action=add;type=binding;espProcess=%s;targetId=%s;targetName=%s;port=%s", espProcName, qbindingid.str(), bindingName, espPort);
        triggerSubscription(changestr.str());
        message.setf("Successfully configured Service '%s', associated with ESDL definition '%s', on ESP '%s' and port '%s'", esdlServiceName, newId.str(), espProcName, espPort);

        ESPLOG(LogMin, "ESDL Binding '%s' published by user='%s' overwrite flag: %s", newId.str(), (user && *user) ? user : "Anonymous", overwrite ? "TRUE" : "FALSE");
        return 0;
    }

    virtual IPropertyTree* getBindingTree(const char* espProcName, const char* espBindingName, StringBuffer& msg) override
    {
        if (!espProcName || !*espProcName)
        {
            msg.set("Could not get configuration: Target ESP proc name not available");
            return NULL;
        }

        if (!espBindingName || !*espBindingName)
        {
            msg.set("Could not get configuration: Target ESP Binding name not available");
            return NULL;
        }

        VStringBuffer xpath("%s[@espprocess='%s'][@espbinding='%s']", ESDL_BINDING_PATH, espProcName, espBindingName);
        Owned<IRemoteConnection> conn = querySDS().connect(xpath.str(), myProcessSession(), RTM_LOCK_READ , SDS_LOCK_TIMEOUT_DESDL);
        if (!conn)
        {
            msg.setf("Could not find binding for ESP proc: %s, and binding: %s", espProcName, espBindingName);
            return NULL;
        }
        else
        {
            conn->close(false);
            return conn->getRoot();
        }
    }

    virtual IPropertyTree* getBindingTree(const char* bindingId, StringBuffer& msg) override
    {
        if (!bindingId || !*bindingId)
        {
            msg.set("Could not get configuration: Target binding id not provided");
            return NULL;
        }

        VStringBuffer xpath("%s[@id='%s']", ESDL_BINDING_PATH, bindingId);
        Owned<IRemoteConnection> conn = querySDS().connect(xpath.str(), myProcessSession(), RTM_LOCK_READ , SDS_LOCK_TIMEOUT_DESDL);
        if (!conn)
        {
            msg.setf("Could not find binding for %s", bindingId);
            return NULL;
        }
        else
        {
            conn->close(false);
            return conn->getRoot();
        }
    }

    virtual bool deleteDefinition(const char* definitionId, StringBuffer& errmsg, StringBuffer* defxml) override
    {
        if (!definitionId || !*definitionId)
            return false;
        bool ret = false;
        Owned<IRemoteConnection> conn = querySDS().connect(ESDL_DEFS_ROOT_PATH, myProcessSession(), RTM_LOCK_WRITE, SDS_LOCK_TIMEOUT_DESDL);
        if (!conn)
            throw MakeStringException(-1, "Unable to connect to %s dali path", ESDL_DEFS_ROOT_PATH);

        Owned<IPropertyTree> root = conn->getRoot();

        if (!root)
            throw MakeStringException(-1, "Unable to open %s dali path", ESDL_DEFS_ROOT_PATH);

        if (isDefinitionBound(definitionId))
            throw MakeStringException(-1, "Unable to delete ESDL definition %s - It is currently bound", definitionId);

        VStringBuffer xpath("%s[@id='%s']", ESDL_DEF_ENTRY, definitionId);
        Owned<IPropertyTree> oldEnvironment = root->getPropTree(xpath.str());
        if (oldEnvironment.get())
        {
            if(defxml)
                toXML(oldEnvironment.get(), *defxml,0,0);
            root->removeTree(oldEnvironment);
            conn->commit();
            ret = true;
        }
        else
        {
            errmsg.append("Could not find ESDL definition. Verify Id (name.version).");
        }
        conn->close();
        return ret;
    }

    virtual bool deleteBinding(const char* bindingId, StringBuffer& errmsg, StringBuffer* bindingxml) override
    {
        if (!bindingId || !*bindingId)
            return false;
        bool ret = false;
        Owned<IRemoteConnection> conn = querySDS().connect(ESDL_BINDINGS_ROOT_PATH, myProcessSession(), 0, SDS_LOCK_TIMEOUT_DESDL);
        if (!conn)
            throw MakeStringException(-1, "Unable to connect to %s dali path", ESDL_BINDINGS_ROOT_PATH);

        Owned<IPropertyTree> root = conn->getRoot();
        if (!root)
            throw MakeStringException(-1, "Unable to open %s dali path", ESDL_BINDINGS_ROOT_PATH);

        VStringBuffer xpath("%s[@id='%s']", ESDL_BINDING_ENTRY, bindingId);
        Owned<IPropertyTree> bindingtree = root->getPropTree(xpath.str());
        StringBuffer bindingName;
        if (bindingtree.get())
        {
            if(bindingxml)
                toXML(bindingtree.get(), *bindingxml,0,0);
            bindingtree->getProp("@espbinding", bindingName);
            root->removeTree(bindingtree);
            conn->commit();
            ret = true;
        }
        else
        {
            errmsg.appendf("Could not find ESDL Binding %s", bindingId);
            return false;
        }
        conn->close();

        StringBuffer changestr;
        changestr.appendf("action=delete;type=binding;targetId=%s", bindingId);
        if(bindingName.length() > 0)
            changestr.appendf(";targetName=%s", bindingName.str());
        triggerSubscription(changestr.str());

        errmsg.appendf("Successfully unbound %s", bindingId);
        return ret;
    }

    virtual IPropertyTree* getDefinitions() override
    {
        Owned<IRemoteConnection> conn = querySDS().connect(ESDL_DEFS_ROOT_PATH, myProcessSession(), RTM_LOCK_READ, SDS_LOCK_TIMEOUT_DESDL);
        if (!conn)
           throw MakeStringException(-1, "Unable to connect to ESDL Service definition information in dali '%s'", ESDL_DEFS_ROOT_PATH);

        conn->close(false); //release lock right away

        Owned<IPropertyTree> esdlDefinitions = conn->getRoot();
        if (!esdlDefinitions.get())
           throw MakeStringException(-1, "Unable to open ESDL Service definition information in dali '%s'", ESDL_DEFS_ROOT_PATH);

        return esdlDefinitions.getLink();
    }

    virtual IPropertyTree* getBindings() override
    {
        Owned<IRemoteConnection> conn = querySDS().connect(ESDL_BINDINGS_ROOT_PATH, myProcessSession(), RTM_LOCK_READ, SDS_LOCK_TIMEOUT_DESDL);
        if (!conn)
           throw MakeStringException(-1, "Unable to connect to ESDL Service definition information in dali '%s'", ESDL_DEFS_ROOT_PATH);

        conn->close(false); //release lock right away

        Owned<IPropertyTree> esdlBindings = conn->getRoot();
        if (!esdlBindings.get())
           throw MakeStringException(-1, "Unable to open ESDL Service definition information in dali '%s'", ESDL_DEFS_ROOT_PATH);
        return esdlBindings.getLink();
    }

private:
    bool isDefinitionBound(const char* esdldefid)
    {
        if (!esdldefid || !*esdldefid)
               return false;

        Owned<IRemoteConnection> conn = querySDS().connect(ESDL_BINDINGS_ROOT_PATH, myProcessSession(), RTM_LOCK_READ, SDS_LOCK_TIMEOUT_DESDL);
        if (!conn)
           return false;

        conn->close(false);

        StringBuffer lcName(esdldefid);
        lcName.toLowerCase();

        IPropertyTree * bindings = conn->queryRoot();

        if (!bindings)
           return false;

        VStringBuffer xpath("%s/Definition[@id='%s']", ESDL_BINDING_ENTRY, lcName.str());

        bool has = bindings->hasProp(xpath);

        return has;
    }

    bool isDefinitionBound(const char* esdldefname, int version)
    {
        if (!esdldefname || !*esdldefname)
            return false;

        if (version <= 0)
            return false;

        StringBuffer id;
        id.appendf("%s.%d", esdldefname, version);

        return isDefinitionBound(id);
    }

    IPropertyTree * getEspBindingConfig(const char * espprocname, const char * espbindingport, const char * bindingname)
    {
        if (!espprocname || !*espprocname)
            return NULL;

        if ((!espbindingport || !*espbindingport) && (!bindingname || !*bindingname))
               return NULL;

        StringBuffer xpath("/Environment/Software");
        //This part of the environment is very constant so read lock is more than enough
        Owned<IRemoteConnection> globalLock = querySDS().connect(xpath.str(), myProcessSession(), RTM_LOCK_READ, SDS_LOCK_TIMEOUT_DESDL);

        if (!globalLock || !globalLock->queryRoot())
            throw MakeStringException(-1, "Unable to connect to ESP configuration information in dali %s", xpath.str());

        globalLock->close(false);

        if (espbindingport && *espbindingport)
            xpath.clear().appendf("EspProcess/[@name='%s']/EspBinding[@port='%s']", espprocname, espbindingport);
        else
            xpath.clear().appendf("EspProcess/[@name='%s']/EspBinding[@name='%s']", espprocname, bindingname);
        Owned<IPropertyTreeIterator> iter = globalLock->queryRoot()->getElements(xpath.str());
        IPropertyTree* result = nullptr;
        ForEach(*iter)
        {
            IPropertyTree &item = iter->query();
            const char* service = item.queryProp("@service");
            if(service && *service)
            {
                VStringBuffer servicepath("EspService[@name='%s']/Properties[@type='DynamicESDL'][1]", service);
                if(globalLock->queryRoot()->queryPropTree(servicepath.str()) != nullptr)
                {
                    result = LINK(&item);
                    break;
                }
            }
        }
        return result;
    }

    void triggerSubscription(const char* changeStr)
    {
        VStringBuffer exceptmsg("Can't access ESDL subscription dali registry, please check if %s exists", ESDL_CHANGE_PATH);
        Owned<IRemoteConnection> subsconn = querySDS().connect(ESDL_CHANGE_PATH, myProcessSession(), RTM_LOCK_WRITE | RTM_CREATE_QUERY, SDS_LOCK_TIMEOUT_DESDL);
        if (!subsconn)
            throw MakeStringException(-1, "%s", exceptmsg.str());
        IPropertyTree* substree = subsconn->queryRoot();
        if(!substree)
            throw MakeStringException(-1, "%s", exceptmsg.str());
        substree->setProp(".", changeStr);
        subsconn->commit();
        subsconn->close(false);
    }

    bool espProcExists(const char * espprocname)
    {
        if (!espprocname || !*espprocname)
            return false;
        StringBuffer xpath("/Environment/Software");
        Owned<IRemoteConnection> software = querySDS().connect(xpath.str(), myProcessSession(), RTM_LOCK_READ, SDS_LOCK_TIMEOUT_DESDL);
        if (!software || !software->queryRoot())
            return false;
        software->close(false);
        xpath.clear().appendf("EspProcess[@name='%s']", espprocname);
        return software->queryRoot()->hasProp(xpath.str());
    }

    bool isEsdlServiceDefined(const char* definitionId, const char* serviceName)
    {
        if (!definitionId || !*definitionId || !serviceName || !*serviceName)
            return false;
        StringBuffer lcdefid (definitionId);
        lcdefid.toLowerCase();
        VStringBuffer xpath("%s[@id='%s']/esxdl", ESDL_DEF_PATH, lcdefid.str());
        Owned<IRemoteConnection> definitiontree = querySDS().connect(xpath.str(), myProcessSession(), RTM_LOCK_READ, SDS_LOCK_TIMEOUT_DESDL);
        definitiontree->close(false);
        if (definitiontree)
        {
            IPropertyTree * esxdl = definitiontree->queryRoot();
            xpath.clear().appendf("EsdlService[@name='%s']", serviceName);
            return esxdl->hasProp(xpath.str());
        }
        return false;
    }
};

class CEsdlSDSSubscription : implements IEsdlSubscription, public CInterface, implements ISDSSubscription
{
private:
    IEsdlListener* mListener;
    CriticalSection daliSubscriptionCritSec;
    SubscriptionId sub_id;
public:
    IMPLEMENT_IINTERFACE;
    CEsdlSDSSubscription(IEsdlListener* listener)
    {
        mListener = listener;
        sub_id = 0;
        subscribe();
    }

    virtual ~CEsdlSDSSubscription()
    {
        unsubscribe();
    }

    virtual void unsubscribe()
    {
        CriticalBlock b(daliSubscriptionCritSec);
        try
        {
            if (sub_id)
            {
                querySDS().unsubscribe(sub_id);
                sub_id = 0;
            }
            DBGLOG("Esdl SDS subscription successfully unsubscribed.");
        }
        catch (IException *E)
        {
            StringBuffer msg;
            E->errorMessage(msg);
            DBGLOG("Error unsubscribing: %s", msg.str());
            E->Release();
        }
    }

    void notify(SubscriptionId id, const char *xpath, SDSNotifyFlags flags, unsigned valueLen, const void *valueData)
    {
        if (!mListener)
        {
            DBGLOG("Can't handle subscription notification because the listener is NULL");
            return;
        }
        if (id != sub_id)
        {
            DBGLOG("Dali subscription (%" I64F "d) received notification for unrecognized dali subscription id: (%" I64F "d)", (__int64) sub_id, (__int64) id);
            return;
        }

        DBGLOG("ESDL change reported to path %s", xpath);
        if(valueLen == 0)
        {
            DBGLOG("There's no data from notify, ignore.");
            return;
        }
        StringBuffer valuebuf;
        valuebuf.append(valueLen, (const char*)valueData);
        DBGLOG("Flags = %d, valueLen = %d, valueData=%s", flags, valueLen, valueLen>0?valuebuf.str():"");
        Owned<IProperties> props = createProperties(false);
        valuebuf.replace(';', '\n');
        props->loadProps(valuebuf.str());

        std::unique_ptr<EsdlNotifyData> dataptr(new EsdlNotifyData);

        //<Change>action=add;type=binding;espProcess=myesp;targetName=DESDLBinding1;targetId=myesp.DESDLBinding1;port=8003</Change>            StringBuffer changeType;
        StringBuffer changeType;
        StringBuffer changeAction;
        StringBuffer portStr;
        changeType.set(props->queryProp("type"));
        changeAction.set(props->queryProp("action"));
        dataptr->id.set(props->queryProp("targetId"));
        dataptr->espProcess.set(props->queryProp("espProcess"));
        dataptr->name.set(props->queryProp("targetName"));
        portStr.set(props->queryProp("port"));
        if (dataptr->id.length() == 0)
        {
            DBGLOG("targetId is empty, ignore this change notification.");
            return;
        }
        dataptr->type = str2type(changeType.str(), changeAction.str());
        if(portStr.length() > 0)
            dataptr->port = atoi(portStr.str());
        else
            dataptr->port = 0;
        mListener->onNotify(dataptr.get());
    }

private:
    void subscribe()
    {
        CriticalBlock b(daliSubscriptionCritSec);
        if(sub_id == 0)
        {
            try
            {
                //Attention: When set both sub and sendValue to true it stops working
                sub_id = querySDS().subscribe(ESDL_CHANGE_PATH, *this, false, true);
                DBGLOG("Esdl subscription to DALI (%s) succeeded.", ESDL_CHANGE_PATH);
            }
            catch (IException *E)
            {
                DBGLOG("ESDL failed to subscribe to DALI (%s)", ESDL_CHANGE_PATH);
                E->Release();
            }
        }
        else
            DBGLOG("Esdl subscription already exists.");
    }

    EsdlNotifyType str2type(const char* type, const char* action)
    {
        EsdlNotifyType result = EsdlNotifyType::BindingAdd;
        if(type && *type)
        {
            if(stricmp(type, "binding") == 0)
            {
                if(action && *action)
                {
                    if(stricmp(action, "add") == 0)
                        result = EsdlNotifyType::BindingAdd;
                    else if(stricmp(action, "update") == 0)
                        result = EsdlNotifyType::BindingUpdate;
                    else if(stricmp(action, "delete") == 0)
                        result = EsdlNotifyType::BindingDelete;
                }
            }
            else if(stricmp(type, "definition") == 0)
            {
                if(action && *action)
                {
                    if(action && stricmp(action, "add") == 0)
                        result = EsdlNotifyType::DefinitionAdd;
                    else if(action && stricmp(action, "update") == 0)
                        result = EsdlNotifyType::DefinitionUpdate;
                    else if(action && stricmp(action, "delete") == 0)
                        result = EsdlNotifyType::DefinitionDelete;
                }
                else
                    result = EsdlNotifyType::DefinitionAdd;
            }
        }
        return result;
    }
};

Owned<IEsdlStore> gEsdlCentralStore;

esdl_engine_decl IEsdlStore* createEsdlCentralStore()
{
    if(gEsdlCentralStore.get() == nullptr)
        gEsdlCentralStore.setown(new CEsdlSDSStore);
    return gEsdlCentralStore.getLink();
}

esdl_engine_decl IEsdlSubscription* createEsdlSubscription(IEsdlListener* listener)
{
    return new CEsdlSDSSubscription(listener);
}
