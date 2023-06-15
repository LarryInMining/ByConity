build: image
	mkdir -p $(HOME)/.ccache
	docker run --network=host --rm --workdir /server --volume $(realpath ../../..):/server --cap-add=SYS_PTRACE --mount=type=bind,source=$(HOME)/.ccache,destination=/ccache -e CCACHE_DIR=/ccache -it byconity/byconity-builder-centos

pull:
	docker pull byconity/byconity-builder-centos

image:
	test -f foundationdb-clients-7.1.27-1.el7.x86_64.rpm || curl -L -o foundationdb-clients-7.1.27-1.el7.x86_64.rpm https://github.com/apple/foundationdb/releases/download/7.1.27/foundationdb-clients-7.1.27-1.el7.x86_64.rpm
	test -f cmake-3.25.3-linux-x86_64.sh || curl -L -o cmake-3.25.3-linux-x86_64.sh https://github.com/Kitware/CMake/releases/download/v3.25.3/cmake-3.25.3-linux-x86_64.sh

	docker build --network=host -t byconity/byconity-builder-centos .

image_push:
	docker push byconity/byconity-builder-centos
