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

#ifndef DEFAULTSECURITY_INCL
#define DEFAULTSECURITY_INCL

#include "basesecurity.hpp"
#include "secloader.hpp"
#include "SecurityResourceList.hpp"

class CDefaultSecurityManager : public CBaseSecurityManager
{
protected:

public:
    CDefaultSecurityManager(const char *serviceName, const char *config);
    CDefaultSecurityManager(const char *serviceName, IPropertyTree *config);
    virtual ~CDefaultSecurityManager(){};
    virtual bool dbauthenticate(ISecUser& User, StringBuffer& SQLQuery){return true;}
    virtual bool dbValidateResource(ISecResource& res,int usernum,const char* realm)
    {
        CSecurityResource * tmpResource =  (CSecurityResource*)(&res);
        if(tmpResource)
            tmpResource->setAccessFlags(SecAccess_Full);
        return true;
    }

    bool authorizeEx(SecResourceType rtype, ISecUser & user, ISecResourceList * resources, IEspSecureContext* secureContext)
    {
        CSecurityResourceList* rlist = static_cast<CSecurityResourceList*>(resources);
        IArrayOf<ISecResource>& list = rlist->getResourceList();
        ForEachItemIn(x, list)
        {
            ISecResource* r = &list.item(x);
            r->setAccessFlags(SecAccess_Full);
        }
        return true;
    }

    virtual SecAccessFlags getAccessFlagsEx(SecResourceType rtype, ISecUser& sec_user, const char* resourcename)
    {
        return SecAccess_Full;
    }
    virtual SecAccessFlags authorizeFileScope(ISecUser & user, const char * filescope)
    {
        return SecAccess_Full;
    }
    virtual bool authorizeFileScope(ISecUser & user, ISecResourceList * resources)
    {
        if(resources)
        {
            int cnt = resources->count();
            for(int i = 0; i < cnt; i++)
            {
                ISecResource* r = resources->queryResource(i);
                if(r)
                    r->setAccessFlags(SecAccess_Full);
            }
        }

        return true;
    }
    virtual SecAccessFlags authorizeWorkunitScope(ISecUser & user, const char * filescope)
    {
        return SecAccess_Full;
    }
    virtual bool authorizeWorkunitScope(ISecUser & user, ISecResourceList * resources)
    {
        if(resources)
        {
            int cnt = resources->count();
            for(int i = 0; i < cnt; i++)
            {
                ISecResource* r = resources->queryResource(i);
                if(r)
                    r->setAccessFlags(SecAccess_Full);
            }
        }

        return true;
    }
    IAuthMap * createAuthMap(IPropertyTree * authconfig)
    {
        return SecLoader::loadTheDefaultAuthMap(authconfig);
    }
    const char * getDescription()
    {
        return "DefaultSecurityManager";
    }
    virtual bool createUserScopes() { return false; }
    virtual aindex_t getManagedFileScopes(IArrayOf<ISecResource>& scopes) { return 0; }
    virtual SecAccessFlags queryDefaultPermission(ISecUser& user) { return SecAccess_Full; }
    virtual secManagerType querySecMgrType() { return SMT_Default; }
    inline virtual const char* querySecMgrTypeName() { return "Default"; }
};

#endif // DEFAULTSECURITY_INCL
//end
