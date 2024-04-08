set +e
# set -x

function gen_version_string {
    if [ -n "$TEST" ]; then
        VERSION_STRING="$VERSION_MAJOR.$VERSION_MINOR.$VERSION_PATCH.$VERSION_REVISION"
    else
        VERSION_STRING="$VERSION_MAJOR.$VERSION_MINOR.$VERSION_PATCH"
    fi
}

function get_version {
    if [ -z "$VERSION_MAJOR" ] && [ -z "$VERSION_MINOR" ] && [ -z "$VERSION_PATCH" ]; then
        BASEDIR=$(dirname "${BASH_SOURCE[0]}")/../../
        VERSION_REVISION=`grep "SET(VERSION_REVISION" ${BASEDIR}/cmake/autogenerated_versions.txt | sed 's/^.*VERSION_REVISION \(.*\)$/\1/' | sed 's/[) ].*//'`
        VERSION_MAJOR=`grep "SET(VERSION_MAJOR" ${BASEDIR}/cmake/autogenerated_versions.txt | sed 's/^.*VERSION_MAJOR \(.*\)/\1/' | sed 's/[) ].*//'`
        VERSION_MINOR=`grep "SET(VERSION_MINOR" ${BASEDIR}/cmake/autogenerated_versions.txt | sed 's/^.*VERSION_MINOR \(.*\)/\1/' | sed 's/[) ].*//'`
        VERSION_PATCH=`grep "SET(VERSION_PATCH" ${BASEDIR}/cmake/autogenerated_versions.txt | sed 's/^.*VERSION_PATCH \(.*\)/\1/' | sed 's/[) ].*//'`
    fi
    VERSION_PREFIX="${VERSION_PREFIX:-v}"
    VERSION_POSTFIX_TAG="${VERSION_POSTFIX:--testing}"

    gen_version_string
}

function get_author {
    AUTHOR=$(git config --get user.name || echo ${USER})
    echo $AUTHOR
}

# Generate revision number.
# set environment variables REVISION, AUTHOR
function gen_revision_author {
	TYPE=$1
	get_version

	set -x
	if [[ $STANDALONE != 'yes' ]]; then

		BUILD_VERSION=${BUILD_VERSION:-"$VERSION_MAJOR.$VERSION_MINOR.$VERSION_PATCH.$VERSION_REVISION"}

		BUILD_DATE="${BUILD_PUB_DATE:-$(date +'%Y-%m-%d')}"
		VERSION_STRING="$BUILD_VERSION"

		VERSION_MAJOR=$(grep -oP '\d+' <<<"$VERSION_MAJOR")
		# e.g.: BUILD_BRN = brn:cn:scm:cn:prod:repo|version:dp/cnch/debian_base_clang|test-2.0.4.210
		BUILD_BRN=${BUILD_BRN:-"version:unkown|"}
		BUILD_SCM=$(grep -oP 'version:\K[\w/]+(?= |)' <<<"$BUILD_BRN")

		VERSION_BRANCH=${BUILD_BRANCH:-$(git rev-parse --abbrev-ref HEAD)}
		git_hash=$(git rev-parse HEAD)
		VERSION_DATE=$(git show -s --format="%cd" --date=format:'%Y-%m-%d' $git_hash)
		git_describe="${VERSION_BRANCH}-${VERSION_STRING}.${VERSION_REVISION}-${git_hash:0:8}-${BUILD_DATE}-${BUILD_SCM}"

		sed -i -e "s/SET(VERSION_REVISION [^) ]*/SET(VERSION_REVISION $VERSION_REVISION/g;" \
			-e "s/SET(VERSION_DESCRIBE [^) ]*/SET(VERSION_DESCRIBE ${git_describe//\//\\\/}/g;" \
			-e "s/SET(VERSION_GITHASH [^) ]*/SET(VERSION_GITHASH $git_hash/g;" \
			-e "s/SET(VERSION_DATE [^) ]*/SET(VERSION_DATE $VERSION_DATE/g;" \
			-e "s/SET(BUILD_DATE [^) ]*/SET(BUILD_DATE $BUILD_DATE/g;" \
			-e "s/SET(BUILD_SCM [^) ]*/SET(BUILD_SCM ${BUILD_SCM//\//\\\/}/g;" \
			-e "s/SET(VERSION_BRANCH [^) ]*/SET(VERSION_BRANCH ${VERSION_BRANCH//\//\\\/}/g;" \
			-e "s/SET(VERSION_MAJOR [^) ]*/SET(VERSION_MAJOR $VERSION_MAJOR/g;" \
			-e "s/SET(VERSION_MINOR [^) ]*/SET(VERSION_MINOR $VERSION_MINOR/g;" \
			-e "s/SET(VERSION_PATCH [^) ]*/SET(VERSION_PATCH $VERSION_PATCH/g;" \
			-e "s/SET(VERSION_STRING [^) ]*/SET(VERSION_STRING $VERSION_STRING/g;" \
			cmake/autogenerated_versions.txt

		echo "Generated version: ${VERSION_STRING}, revision: ${VERSION_REVISION}."
	fi

	AUTHOR=$(git config --get user.name || echo ${USER})
	export AUTHOR
}

function get_revision_author {
    get_version
    AUTHOR=$(get_author)
    export AUTHOR
}

# Generate changelog from changelog.in.
function gen_changelog {
    VERSION_STRING="$1"
    CHDATE="$2"
    AUTHOR="$3"
    CHLOG="$4"
    if [ -z "$VERSION_STRING" ] ; then
        get_revision_author
    fi

    if [ -z "$CHLOG" ] ; then
        CHLOG=debian/changelog
    fi

    if [ -z "$CHDATE" ] ; then
        CHDATE=$(LC_ALL=C date -R | sed -e 's/,/\\,/g') # Replace comma to '\,'
    fi

    sed \
        -e "s/[@]VERSION_STRING[@]/$VERSION_STRING/g" \
        -e "s/[@]DATE[@]/$CHDATE/g" \
        -e "s/[@]AUTHOR[@]/$AUTHOR/g" \
        -e "s/[@]EMAIL[@]/$(whoami)@yandex-team.ru/g" \
        < $CHLOG.in > $CHLOG
}

# Change package versions that are installed for Docker images.
function gen_dockerfiles {
    VERSION_STRING="$1"
    ls -1 docker/*/Dockerfile | xargs sed -i -r -e 's/ARG version=.+$/ARG version='$VERSION_STRING'/'
}

function make_rpm {
    [ -z "$VERSION_STRING" ] && get_version && VERSION_STRING+=${VERSION_POSTFIX}
    VERSION_FULL="${VERSION_STRING}"
    PACKAGE_DIR=${PACKAGE_DIR=../}

    function deb_unpack {
        rm -rf $PACKAGE-$VERSION_FULL
        alien --verbose --generate --to-rpm --scripts ${PACKAGE_DIR}${PACKAGE}_${VERSION_FULL}_${ARCH}.deb
        cd $PACKAGE-$VERSION_FULL
        mv ${PACKAGE}-$VERSION_FULL-2.spec ${PACKAGE}-$VERSION_FULL-2.spec.tmp
        cat ${PACKAGE}-$VERSION_FULL-2.spec.tmp \
            | grep -vF '%dir "/"' \
            | grep -vF '%dir "/usr/"' \
            | grep -vF '%dir "/usr/bin/"' \
            | grep -vF '%dir "/usr/lib/"' \
            | grep -vF '%dir "/usr/lib/debug/"' \
            | grep -vF '%dir "/usr/lib/.build-id/"' \
            | grep -vF '%dir "/usr/share/"' \
            | grep -vF '%dir "/usr/share/doc/"' \
            | grep -vF '%dir "/lib/"' \
            | grep -vF '%dir "/lib/systemd/"' \
            | grep -vF '%dir "/lib/systemd/system/"' \
            | grep -vF '%dir "/etc/"' \
            | grep -vF '%dir "/etc/security/"' \
            | grep -vF '%dir "/etc/security/limits.d/"' \
            | grep -vF '%dir "/etc/init.d/"' \
            | grep -vF '%dir "/etc/cron.d/"' \
            | grep -vF '%dir "/etc/systemd/system/"' \
            | grep -vF '%dir "/etc/systemd/"' \
            | sed -e 's|%config |%config(noreplace) |' \
            > ${PACKAGE}-$VERSION_FULL-2.spec
    }

    function rpm_pack {
        rpmbuild --buildroot="$CUR_DIR/${PACKAGE}-$VERSION_FULL" -bb --target ${TARGET} "${PACKAGE}-$VERSION_FULL-2.spec"
        cd $CUR_DIR
    }

    function unpack_pack {
        deb_unpack
        rpm_pack
    }

    PACKAGE=clickhouse-server
    ARCH=all
    TARGET=noarch
    deb_unpack
    mv ${PACKAGE}-$VERSION_FULL-2.spec ${PACKAGE}-$VERSION_FULL-2.spec_tmp
    echo "Requires: clickhouse-common-static = $VERSION_FULL-2" >> ${PACKAGE}-$VERSION_FULL-2.spec
    echo "Requires: tzdata" >> ${PACKAGE}-$VERSION_FULL-2.spec
    echo "Requires: initscripts" >> ${PACKAGE}-$VERSION_FULL-2.spec
    echo "Obsoletes: clickhouse-server-common < $VERSION_FULL" >> ${PACKAGE}-$VERSION_FULL-2.spec

    cat ${PACKAGE}-$VERSION_FULL-2.spec_tmp >> ${PACKAGE}-$VERSION_FULL-2.spec
    rpm_pack

    PACKAGE=clickhouse-client
    ARCH=all
    TARGET=noarch
    deb_unpack
    mv ${PACKAGE}-$VERSION_FULL-2.spec ${PACKAGE}-$VERSION_FULL-2.spec_tmp
    echo "Requires: clickhouse-common-static = $VERSION_FULL-2" >> ${PACKAGE}-$VERSION_FULL-2.spec
    cat ${PACKAGE}-$VERSION_FULL-2.spec_tmp >> ${PACKAGE}-$VERSION_FULL-2.spec
    rpm_pack

    PACKAGE=clickhouse-test
    ARCH=all
    TARGET=noarch
    deb_unpack
    mv ${PACKAGE}-$VERSION_FULL-2.spec ${PACKAGE}-$VERSION_FULL-2.spec_tmp
    echo "Requires: python3" >> ${PACKAGE}-$VERSION_FULL-2.spec
    #echo "Requires: python3-termcolor" >> ${PACKAGE}-$VERSION-2.spec
    cat ${PACKAGE}-$VERSION_FULL-2.spec_tmp >> ${PACKAGE}-$VERSION_FULL-2.spec
    rpm_pack

    PACKAGE=clickhouse-common-static
    ARCH=amd64
    TARGET=x86_64
    unpack_pack

    PACKAGE=clickhouse-common-static-dbg
    ARCH=amd64
    TARGET=x86_64
    unpack_pack

    mv clickhouse-*-${VERSION_FULL}-2.*.rpm ${PACKAGE_DIR}
}

function make_tgz {
    [ -z "$VERSION_STRING" ] && get_version && VERSION_STRING+=${VERSION_POSTFIX}
    VERSION_FULL="${VERSION_STRING}"
    PACKAGE_DIR=${PACKAGE_DIR=../}

    for PACKAGE in clickhouse-server clickhouse-client clickhouse-test clickhouse-common-static clickhouse-common-static-dbg; do
        alien --verbose --scripts --generate --to-tgz ${PACKAGE_DIR}${PACKAGE}_${VERSION_FULL}_*.deb
        PKGDIR="./${PACKAGE}-${VERSION_FULL}"
        if [ ! -d "$PKGDIR/install" ]; then
            mkdir "$PKGDIR/install"
        fi

        if [ ! -f "$PKGDIR/install/doinst.sh" ]; then
            echo '#!/bin/sh' > "$PKGDIR/install/doinst.sh"
            echo 'set -e' >> "$PKGDIR/install/doinst.sh"
        fi

        SCRIPT_TEXT='
SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
for filepath in `find $SCRIPTPATH/.. -type f -or -type l | grep -v "\.\./install/"`; do
    destpath=${filepath##$SCRIPTPATH/..}
    mkdir -p $(dirname "$destpath")
    cp -r "$filepath" "$destpath"
done
'

        echo "$SCRIPT_TEXT" | sed -i "2r /dev/stdin" "$PKGDIR/install/doinst.sh"

        chmod +x "$PKGDIR/install/doinst.sh"

        if [ -f "/usr/bin/pigz" ]; then
            tar --use-compress-program=pigz -cf "${PACKAGE}-${VERSION_FULL}.tgz" "$PKGDIR"
        else
            tar -czf "${PACKAGE}-${VERSION_FULL}.tgz" "$PKGDIR"
        fi

        rm -r $PKGDIR
    done


    mv clickhouse-*-${VERSION_FULL}.tgz ${PACKAGE_DIR}
}
