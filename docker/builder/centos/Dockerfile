FROM centos:centos7
RUN yum update -y
RUN yum install -y clang gcc git cmake  swig  unzip openssh-server  gzip bash  binutils  bzip2 coreutils  diffutils findutils grep hostname ncurses-base passwd perl procps psmisc python3 sed tar tzdata util-linux curl
WORKDIR /home

COPY foundationdb-clients-7.1.27-1.el7.x86_64.rpm ./
RUN yum localinstall foundationdb-clients-7.1.27-1.el7.x86_64.rpm
COPY build.sh /
CMD ["/bin/bash", "/build.sh"]

