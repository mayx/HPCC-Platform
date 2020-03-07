FROM ubuntu:18.04
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
apt-get -y install binutils binutils-common binutils-x86-64-linux-gnu cpp cpp-7 curl expect fontconfig-config fonts-dejavu-core g++ \
  g++-7 gcc gcc-7 gcc-7-base libapr1 libaprutil1 libasan4 libasn1-8-heimdal libatlas3-base libatomic1 libbinutils \
  libboost-regex1.65.1 libc-dev-bin libc6-dev libcc1-0 libcilkrts5 libcurl4 libdrm-amdgpu1 libdrm-common libdrm-intel1 \
  libdrm-nouveau2 libdrm-radeon1 libdrm2 libelf1 libfontconfig1 libfontenc1 libfreetype6 libgcc-7-dev libgdbm5 \
  libgfortran4 libgl1 libgl1-mesa-dri libgl1-mesa-glx libglapi-mesa libglvnd0 libglx-mesa0 libglx0 libgomp1 \
  libgssapi3-heimdal libhcrypto4-heimdal libheimbase1-heimdal libheimntlm0-heimdal libhx509-5-heimdal libice6 libisl19 \
  libitm1 libkrb5-26-heimdal libldap-2.4-2 libldap-common libllvm9 liblsan0 libmemcached11 libmemcachedutil2 libmpc3 \
  libmpfr6 libmpx2 libnghttp2-14 libnuma1 libpciaccess0 libpng16-16 libpopt0 libpython-stdlib libpython2.7 \
  libpython2.7-minimal libpython2.7-stdlib libpython3.6 libquadmath0 libroken18-heimdal librtmp1 libsasl2-2 \
  libsasl2-modules libsasl2-modules-db libsensors4 libsm6 libstdc++-7-dev libtcl8.6 libtsan0 libubsan0 libutempter0 \
  libwind0-heimdal libx11-xcb1 libxaw7 libxcb-dri2-0 libxcb-dri3-0 libxcb-glx0 libxcb-present0 libxcb-shape0 \
  libxcb-sync1 libxcomposite1 libxdamage1 libxfixes3 libxft2 libxi6 libxinerama1 libxmu6 libxpm4 libxrandr2 libxrender1 \
  libxshmfence1 libxslt1.1 libxt6 libxtst6 libxv1 libxxf86dga1 libxxf86vm1 linux-libc-dev manpages manpages-dev psmisc \
  python python-minimal python2.7 python2.7-minimal rsync tcl-expect tcl8.6 tzdata x11-common x11-utils xbitmaps xterm
RUN apt-get -y install  dbus dmsetup gir1.2-glib-2.0 libapparmor1 libargon2-0 libcap2 libcryptsetup12 libdbus-1-3 libdevmapper1.02.1 \
  libgirepository-1.0-1 libglib2.0-0 libglib2.0-data libidn11 libip4tc0 libjson-c3 libkmod2 libnss-systemd \
  libpam-systemd libpython3-stdlib libssl1.0.0 libwrap0 ncurses-term networkd-dispatcher openssh-client openssh-server \
  openssh-sftp-server python3 python3-certifi python3-chardet python3-dbus python3-gi python3-idna python3-minimal \
  python3-pkg-resources python3-requests python3-six python3-urllib3 python3.6 python3.6-minimal shared-mime-info \
  ssh-import-id systemd systemd-sysv wget xauth xdg-user-dirs
RUN apt-get clean
COPY hpccsystems-platform-community_7.6.30-1bionic_amd64.deb .
RUN dpkg -i hpccsystems-platform-community_7.6.30-1bionic_amd64.deb
USER hpcc
ENTRYPOINT ["/bin/sh", "-c", "/opt/HPCCSystems/etc/init.d/hpcc-init start && tail -f /var/log/HPCCSystems/myesp/esp.log"]
