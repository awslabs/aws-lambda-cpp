FROM public.ecr.aws/amazonlinux/amazonlinux:2023 as sdkgit
RUN dnf install -y git
WORKDIR /
RUN git clone https://github.com/aws/aws-sdk-cpp --recurse-submodules

FROM public.ecr.aws/amazonlinux/amazonlinux:2 as amazon-linux-2
RUN yum install -y \
    cmake3 \
    ninja-build \
    git \
    gcc-c++ \
    openssl-devel \
    curl-devel \
    openssl-static \
    zip
COPY --from=sdkgit /aws-sdk-cpp /aws-sdk-cpp
RUN cmake3 -Saws-sdk-cpp -Baws-sdk-cpp/build -DBUILD_ONLY=lambda -DENABLE_TESTING=OFF -GNinja
RUN cd aws-sdk-cpp/build && ninja-build && ninja-build install
RUN ln -s /usr/bin/cmake3 /usr/local/bin/cmake
RUN ln -s /usr/bin/ctest3 /usr/local/bin/ctest
RUN ln -s /usr/bin/ninja-build /usr/local/bin/ninja

FROM public.ecr.aws/amazonlinux/amazonlinux:2018.03 as amazon-linux-2018.03
RUN yum install -y \
    gcc-c++ \
    git \
    ninja-build \
    curl-devel \
    openssl-devel \
    openssl-static \
    zlib-devel \
    gtest-devel \
    zip
RUN curl -fLo cmake-install https://github.com/Kitware/CMake/releases/download/v3.13.0/cmake-3.13.0-Linux-x86_64.sh && \
    sh cmake-install --skip-license --prefix=/usr --exclude-subdirectory;
COPY --from=sdkgit /aws-sdk-cpp /aws-sdk-cpp
RUN cmake -Saws-sdk-cpp -Baws-sdk-cpp/build -DBUILD_ONLY=lambda -DENABLE_TESTING=OFF -GNinja
RUN cd aws-sdk-cpp/build && ninja-build && ninja-build install
RUN ln -s /usr/bin/ninja-build /usr/local/bin/ninja

FROM public.ecr.aws/amazonlinux/amazonlinux:2023 as amazon-linux-2023
RUN dnf install -y \
    cmake \
    ninja-build \
    gcc-c++ \
    openssl-devel \
    curl-devel \
    zip \
    zlib-devel
COPY --from=sdkgit /aws-sdk-cpp /aws-sdk-cpp
RUN cmake -Saws-sdk-cpp -Baws-sdk-cpp/build -DBUILD_ONLY=lambda -DENABLE_TESTING=OFF -GNinja
RUN cd aws-sdk-cpp/build && ninja && ninja install

FROM public.ecr.aws/ubuntu/ubuntu:22.04 as ubuntu-linux-22.04
RUN apt-get update
RUN apt-get install -y \
    git \
    clang \
    cmake \
    zlib1g-dev \
    libssl-dev \
    libcurl4-openssl-dev \
    wget \
    ninja-build \
    zip
RUN update-alternatives --set cc /usr/bin/clang
RUN update-alternatives --set c++ /usr/bin/clang++
COPY --from=sdkgit /aws-sdk-cpp /aws-sdk-cpp
RUN cmake -Saws-sdk-cpp -Baws-sdk-cpp/build -DBUILD_ONLY=lambda -DENABLE_TESTING=OFF -GNinja
RUN cd aws-sdk-cpp/build && ninja && ninja install

FROM public.ecr.aws/ubuntu/ubuntu:18.04 as ubuntu-linux-18.04 
RUN apt-get update
RUN apt-get install -y \
    autoconf \
    clang \
    git \
    libssl-dev \
    libtool \
    make \
    ninja-build \
    wget \
    zip \
    zlib1g-dev
RUN wget -O cmake-install https://github.com/Kitware/CMake/releases/download/v3.13.0/cmake-3.13.0-Linux-x86_64.sh && \
    sh cmake-install --skip-license --prefix=/usr --exclude-subdirectory;
RUN update-alternatives --set cc /usr/bin/clang
RUN update-alternatives --set c++ /usr/bin/clang++
RUN git clone --branch curl-8_5_0 https://github.com/curl/curl.git
WORKDIR /curl
RUN autoreconf -fi
RUN ./configure --with-openssl
RUN make
RUN make install
WORKDIR /
COPY --from=sdkgit /aws-sdk-cpp /aws-sdk-cpp
RUN cmake -Saws-sdk-cpp -Baws-sdk-cpp/build -DBUILD_ONLY=lambda -DENABLE_TESTING=OFF -GNinja
RUN cd aws-sdk-cpp/build && ninja && ninja install

FROM public.ecr.aws/docker/library/archlinux:latest as arch-linux 
RUN pacman -Sy --noconfirm \
    cmake \
    ninja \
    clang \
    curl \
    zip
COPY --from=sdkgit /aws-sdk-cpp /aws-sdk-cpp
RUN CC=/usr/bin/clang CXX=/usr/bin/clang++ cmake -Saws-sdk-cpp -Baws-sdk-cpp/build -GNinja \
    -DBUILD_ONLY=lambda \
    -DENABLE_TESTING=OFF
RUN cmake --build aws-sdk-cpp/build -t install

FROM public.ecr.aws/docker/library/alpine:3.19 as alpine-linux-3.19
RUN apk add --no-cache \
    bash \
    cmake \
    curl-dev \
    g++ \
    git \
    ninja \
    openssl-libs-static \
    zlib-dev \
    zip
COPY --from=sdkgit /aws-sdk-cpp /aws-sdk-cpp
RUN cmake -Saws-sdk-cpp -Baws-sdk-cpp/build -GNinja \
    -DBUILD_ONLY=lambda \
    -DENABLE_TESTING=OFF
RUN cd aws-sdk-cpp/build && ninja && ninja install

FROM public.ecr.aws/docker/library/alpine:3.15 as alpine-linux-3.15
RUN apk add --no-cache \
    bash \
    cmake \
    curl-dev \
    g++ \
    git \
    libexecinfo-dev \
    ninja \
    openssl-libs-static \
    zlib-dev \
    zip
COPY --from=sdkgit /aws-sdk-cpp /aws-sdk-cpp
RUN cmake -Saws-sdk-cpp -Baws-sdk-cpp/build -GNinja \
    -DBUILD_ONLY=lambda \
    -DENABLE_TESTING=OFF
RUN cd aws-sdk-cpp/build && ninja && ninja install

FROM scratch as all
COPY --from=alpine-linux-3.19    /etc/os-release /
COPY --from=alpine-linux-3.15    /etc/os-release /
COPY --from=amazon-linux-2       /etc/os-release /
COPY --from=amazon-linux-2018.03 /etc/os-release /
COPY --from=amazon-linux-2023    /etc/os-release /
COPY --from=arch-linux           /etc/os-release /
COPY --from=ubuntu-linux-18.04   /etc/os-release /
COPY --from=ubuntu-linux-22.04   /etc/os-release /

FROM scratch
