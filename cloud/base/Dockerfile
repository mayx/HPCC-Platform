FROM ubuntu:18.04
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
apt-get -y install cmake bison flex build-essential binutils-dev libldap2-dev libcppunit-dev libicu-dev libxslt1-dev zlib1g-dev libboost-regex-dev libarchive-dev python-dev libv8-dev default-jdk libapr1-dev libaprutil1-dev libiberty-dev libhiredis-dev libtbb-dev libxalan-c-dev libnuma-dev nodejs libevent-dev libatlas-base-dev libblas-dev python3-dev default-libmysqlclient-dev libsqlite3-dev r-base-dev r-cran-rcpp r-cran-rinside r-cran-inline libmemcached-dev libcurl4-openssl-dev pkg-config libtool autotools-dev automake libssl-dev  && \
apt-get clean
