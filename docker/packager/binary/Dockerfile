# rebuild in #33610
# docker build -t byconity/binary-builder .
FROM byconity/byconity-builder-centos
RUN yum install -y curl
ENV CC=clang-11
ENV CXX=clang++-11

# Architecture of the image when BuildKit/buildx is used
ARG TARGETARCH
ARG NFPM_VERSION=2.20.0

COPY nfpm.rpm /tmp/
RUN yum localinstall -y /tmp/nfpm.rpm \
  && rm /tmp/nfpm.rpm

RUN mkdir /workdir && chmod 777 /workdir
WORKDIR /workdir

COPY build.sh /
CMD ["bash", "-c", "/build.sh 2>&1"]
