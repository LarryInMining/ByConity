FROM centos:centos7
RUN yum update -y && yum makecache
RUN yum install -y gcc gcc-c++ git  swig  unzip openssh-server  gzip bash  binutils  bzip2 coreutils  diffutils findutils grep hostname ncurses-base passwd perl procps psmisc python3 sed tar tzdata util-linux curl
WORKDIR /home

COPY foundationdb-clients-7.1.27-1.el7.x86_64.rpm ./
RUN yum localinstall -y foundationdb-clients-7.1.27-1.el7.x86_64.rpm

COPY cmake-3.25.3-linux-x86_64.sh ./
RUN ./cmake-3.25.3-linux-x86_64.sh --skip-license --prefix=/usr/local/ 

COPY ninja-linux.zip ./
RUN unzip ninja-linux.zip && mv ninja /usr/local/bin/

COPY clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz ./
RUN tar -xf clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz

COPY build.sh /
CMD ["/bin/bash", "/build.sh"]

