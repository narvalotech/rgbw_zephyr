FROM ubuntu:22.04

ARG UID=1000
ARG GID=1000

SHELL ["/bin/bash", "-c"]

ENV DEBIAN_FRONTEND=noninteractive
ARG ZSDK_VERSION=0.16.8
ENV ZSDK_VERSION=$ZSDK_VERSION
ARG BSIM_VERSION=v2.3
ENV BSIM_VERSION=$BSIM_VERSION
ARG WGET_ARGS="-q --show-progress --progress=bar:force:noscroll"

RUN apt-get -y update && \
	apt-get -y upgrade && \
	apt-get install -y --no-install-recommends \
	git cmake ninja-build gperf \
	ccache dfu-util device-tree-compiler wget \
	python3-dev python3-pip python3-setuptools python-is-python3 python3-tk python3-wheel \
	xz-utils file make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1 libfftw3-3

# Install gdbserver for enabling GUI debugging in VSCode
# It's ok to make another layer for this IMO
RUN apt-get install --no-install-recommends -y \
	gdb gdbserver

# Initialize system locale
RUN apt-get install -y --no-install-recommends \
	liblocale-gettext-perl locales && \
	locale-gen en_US.UTF-8
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US:en
ENV LC_ALL=en_US.UTF-8

# Install python deps
RUN pip3 install -U pip && \
	pip3 install west && \
	pip3 install -r https://raw.githubusercontent.com/zephyrproject-rtos/zephyr/main/scripts/requirements.txt

# But what about second python deps?
# See .github/workflows/compliance.yml *cries in requirements.txt*
RUN pip3 install python-magic lxml junitparser gitlint pylint pykwalify yamllint clang-format unidiff

# Install (minimal) Zephyr SDK
RUN mkdir -p /opt/toolchains && \
	cd /opt/toolchains && \
	wget ${WGET_ARGS} https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZSDK_VERSION}/zephyr-sdk-${ZSDK_VERSION}_linux-${HOSTTYPE}_minimal.tar.xz && \
	tar xf zephyr-sdk-${ZSDK_VERSION}_linux-${HOSTTYPE}_minimal.tar.xz && \
	zephyr-sdk-${ZSDK_VERSION}/setup.sh -t x86_64-zephyr-elf -t arm-zephyr-eabi -h -c && \
	rm zephyr-sdk-${ZSDK_VERSION}_linux-${HOSTTYPE}_minimal.tar.xz

# Install BSIM
# Note: west needs an extra folder level, so we create a link to the old location to be backwards compatible
RUN mkdir -p /opt/bsim_west && \
	cd /opt/ && \
	west init -m https://github.com/zephyrproject-rtos/babblesim-manifest.git --mr ${BSIM_VERSION} bsim_west && \
	cd bsim_west/bsim && \
	west update && \
	make everything -j 8 && \
	echo ${BSIM_VERSION} > ./version && \
	chmod ag+w . -R && \
	ln -s /opt/bsim_west/bsim /opt/bsim

# Create 'user' account
RUN groupadd -g $GID -o user

RUN apt-get install -y --no-install-recommends \
	sudo && \
	useradd -u $UID -m -g user -G plugdev user \
	&& echo 'user ALL = NOPASSWD: ALL' > /etc/sudoers.d/user \
	&& chmod 0440 /etc/sudoers.d/user

# TODO: is this necessary?
RUN usermod -s /usr/bin/bash user

USER user

# Run the Zephyr SDK setup script as 'user' in order to ensure that the
# `Zephyr-sdk` CMake package is located in the package registry under the
# user's home directory.

RUN sudo -E -- bash -c ' \
	/opt/toolchains/zephyr-sdk-${ZSDK_VERSION}/setup.sh -c && \
	chown -R user:user /home/user/.cmake \
	'

# Add a cache of the west workspace to speed-up checkout
COPY west.yml /west-cache.yml
COPY scripts/west-cache.sh /west-cache.sh
RUN /west-cache.sh && sudo rm -rf /west-cache.*

RUN sudo DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y \
	tshark

ENV ZEPHYR_TOOLCHAIN_VARIANT=zephyr

# Link to GH repo
# LABEL org.opencontainers.image.source=https://github.com/jori-nordic/bsim-demo

CMD /usr/bin/bash
