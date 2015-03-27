HPCC SYSTEMS software Copyright (C) 2012 HPCC Systems.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 

http://hpccsystems.com
 

To build for Linux:
-------------------

1. Check out sources (for example, to directory ~/hpcc)
2. Create a build directory - either as a child of hpcc or elsewhere
3. cd to the build directory
4a.To create makefiles to build native release version for local machine, run
   cmake ~/hpcc
4b.To create makefiles to build native debug version, run
   cmake -DCMAKE_BUILD_TYPE=Debug ~/hpcc
4c.To create makefiles to build 32-bit version from 64-bit host, run
   cmake -DCMAKE_C_FLAGS:STRING="-m32 -march=i386" -DCMAKE_CXX_FLAGS:STRING="-m32 -march=i386" ~/hpcc
5. To build the makefiles just created above, run
   make
6. Executables will be created in ./<releasemode>/bin and ./<releasemode>/libs
7. To create a .deb / ,rpm to install, run
   make package

 
To build for Windows:
---------------------

1. Check out sources (for example, to directory c:\hpcc)
2. Create a build directory - either as a child of hpcc or elsewhere
3. cd to the build directory
4. To create a Visual Studio project, run
   cmake c:\hpcc -G "Visual Studio 9 2008"
   The sln file hpccsystems-platform.sln will be created in the current directory, and will support debug and release targets
5. To build the project, load the solution into the visual studio IDE and build in the usual way.
6. Executables will be created in (for example) c:\hpcc\bin\<release_mode>

To build client tools for Macintosh OSX:
----------------------------------------

1. Check out sources (for example, to directory ~/hpcc)
2. Fetch all sub-modules with:
   git submodule update --init --recursive
   
3. You many need to install some 3rd-party dev packages using macports or brew. (brew installs shown below)
   
   brew install icu4c
   brew install boost
   brew install libarchive
   brew install bison27 
   brew install openldap 

   n.b. Also make sure that bison is ahead of the system bison on your path.
   bison --version
   * The result should be > 2.4.1 
  
   n.b. OS X has LDAP installed, but when compiling against it (/System/Library/Frameworks/LDAP.framework/Headers/ldap.h) you will get a "#include nested too deeply", which is why you should install openldap
   
4. Create a build directory - either as a child of hpcc or elsewhere
5. cd to the build directory
6. Use clang to build the clienttools (gcc4.2 cores when compiling some of the sources):
   export CC=/usr/bin/clang 
   export CXX=/usr/bin/clang++ 
   cmake ../ -DICU_LIBRARIES=/usr/local/opt/icu4c/lib/libicuuc.dylib -DICU_INCLUDE_DIR=/usr/local/opt/icu4c/include -DLIBARCHIVE_INCLUDE_DIR=/usr/local/opt/libarchive/include -DLIBARCHIVE_LIBRARIES=/usr/local/opt/libarchive/lib/libarchive.dylib -DBOOST_REGEX_LIBRARIES=/usr/local/opt/boost/lib -DBOOST_REGEX_INCLUDE_DIR=/usr/local/opt/boost/include -DCLIENTTOOLS_ONLY=true -DUSE_OPENLDAP=true -DOPENLDAP_INCLUDE_DIR=/usr/local/opt/openldap/include -DOPENLDAP_LIBRARIES=/usr/local/opt/openldap/lib/libldap_r.dylib
7. To build the makefiles just created above, run
   make
8. Executables will be created in ./<releasemode>/bin and ./<releasemode>/libs
9. To create a .dmg to install, run
   make package
 
 
