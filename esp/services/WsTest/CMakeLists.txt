################################################################################
#    HPCC SYSTEMS software Copyright (C) 2012 HPCC Systems®.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
################################################################################


# Component: WsTest
#####################################################
# Description:
# ------------
#    Cmake Input File for WsTest
#####################################################

project( WsTest )

include(${HPCC_SOURCE_DIR}/esp/scm/espscm.cmake)

set (    SRCS
         ${ESPSCM_GENERATED_DIR}/WsTest_esp.cpp
         WsTestPlugin.cpp
         WsTestService.cpp
    )

include_directories (
         ./../..
         ./../../../system/mp
         ./../../../system/jlib
         ./../../services
         ./../../platform
         ./../../../system/xmllib
         ./../../../system/security/shared
         ./../../../system/security/securesocket
         ./../../../system/include
         ./../../../common/environment
         ./../../../common/workunit
         ./../../../common/wuwebview
         ./../../clients
         ./../../../common/fileview2
         ./../../../dali/base
         ./../../bindings
         ./../../bindings/SOAP/xpp
    )

ADD_DEFINITIONS( -D_USRDLL )

HPCC_ADD_LIBRARY( WsTest SHARED ${SRCS} )
add_dependencies( WsTest espscm )
install ( TARGETS WsTest RUNTIME DESTINATION ${EXEC_DIR} LIBRARY DESTINATION ${LIB_DIR} COMPONENT Runtime)
target_link_libraries ( WsTest
         jlib
         xmllib
         esphttp
         mp
         hrpc
         remote
         dalibase
         environment
         dllserver
         nbcd
         eclrtl
         deftype
         workunit
         wuwebview
         jhtree
         hql
         fileview2
    )

IF (USE_OPENSSL)
    target_link_libraries ( WsTest
        securesocket
    )
ENDIF()
IF (USE_OPENLDAP)
  target_link_libraries ( WsTest LdapSecurity )
ENDIF(USE_OPENLDAP)
