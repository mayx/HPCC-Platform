    1  ls
    2  history
    3  apt-get install wget
    4  wget https://github.com/Kitware/CMake/releases/download/v3.15.7/cmake-3.15.7-Linux-x86_64.tar.gz
    5  ls
    6  tar zxvf cmake-3.15.7-Linux-x86_64.tar.gz
    7  ls
    8  cd cmake-3.15.7-Linux-x86_64
    9  make configure
   10  ./configure
   11  ls
   12  ls bin/
   13  pwd
   14  ls
   15  cd ..
   16  cp cmake-3.15.7-Linux-x86_64/bin/cmake bin/
   17  which cmake
   18  cmake -v
   19  cmake -version
   20  which cmake
   21  cp cmake-3.15.7-Linux-x86_64/bin/cmake /usr/bin/
   22  cmake -version
   23  mkdir
   24  ls
   25  pwd
   26  mkdir hpcc
   27  cd hpcc/
   28  ls
   29  apt-get install git
   30  git clone https://github.com/hpcc-systems/HPCC-Platform
   31  cd HPCC-Platform/
   32  git submodule update --recursive --init
   33  ls
   34  cd ../buil
   35  cd ..
   36  mkdir build
   37  cd build/
   38  ls
   39  scp mayx@192.168.56.102:/home/mayx/git/build/runcmake.sh .
   40  vim runcmake.sh
   41  apt-get install vim
   42  vim runcmake.sh
   43  ./runcmake.sh
   44  ls /cmake-3.15.7-Linux-x86_64
   45  cp -r /cmake-3.15.7-Linux-x86_64/share/cmake-3.15 /usr/share
   46  ./runcmake.sh
   47  vim runcmake.sh$
   48  gpg
   49  ls
   50  vim runcmake.sh$
   51  echo $USER
   52* gpg --gen-ke
   53  ./runcmake.sh$
   54  apt-get install npm
   55  apt-get install node-gyp
   56  apt-get install nodejs-dev
   57  ls
   58  cp runcmake.sh ..
   59  rm -rf *
   60  ls
   61  vim ../runcmake.sh$
   62  cp ../runcmake.sh$
   63  cp ../runcmake.sh .
   64  ./runcmake.sh$
   65  curl -sL https://deb.nodesource.com/setup_13.x | sudo -E bash -
   66  sudo apt-get install -y nodejs
   67  curl -sL https://deb.nodesource.com/setup_13.x
   68  curl -sL https://deb.nodesource.com/setup_13.x | bash -
   69  apt-get install -y nodejs
   70  ./runcmake.sh$
   71  make -j8
   72  make install
   73  ls
   74  ls /
   75  find / -name opt
   76  ls
   77  cd ../run
   78  ls
   79  ./opt/HPCCSystems/etc/init.d/hpcc-init start
   80  useradd hpcc
   81  ./opt/HPCCSystems/etc/init.d/hpcc-init start
   82  history
   83  history > setup.sh
