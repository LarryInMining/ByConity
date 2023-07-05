pull:
	docker pull byconity/binary-builder

image:
	test -f nfpm.rpm || curl -L -o nfpm.deb https://github.com/goreleaser/nfpm/releases/download/v2.20.0/nfpm_amd64.rpm
	docker build -t byconity/binary-builder .
	docker tag byconity/binary-builder byconity/binary-builder:stable

image_push:
	docker tag byconity/binary-builder:latest byconity/binary-builder:stable
	docker push byconity/binary-builder
