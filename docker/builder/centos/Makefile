build: image
	mkdir -p $(HOME)/.ccache
	docker run --network=host --rm --workdir /server --volume $(realpath ../..):/server --cap-add=SYS_PTRACE --mount=type=bind,source=$(HOME)/.ccache,destination=/ccache -e CCACHE_DIR=/ccache -it byconity/byconity-builder-centos

pull:
	docker pull byconity/byconity-builder-centos

image:
	test -f foundationdb-clients_7.1.27-1_amd64.deb || curl -L -o foundationdb-clients_7.1.27-1_amd64.deb https://github.com/apple/foundationdb/releases/download/7.1.27/foundationdb-clients_7.1.27-1_amd64.deb
	docker build --network=host -t byconity/byconity-builder-centos .

image_push:
	docker push byconity/byconity-builder-centos
