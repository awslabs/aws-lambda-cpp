#!/bin/bash
set -euo pipefail

OS=$1

case "$OS" in
  al2023|al2023-arm)
    dnf install -y cmake ninja-build gcc-c++ openssl-devel libcurl-devel zip tar gzip git jq
    ;;
  ubuntu)
    apt-get update && apt-get install -y git clang zlib1g-dev libssl-dev libcurl4-openssl-dev cmake ninja-build zip unzip curl jq
    update-alternatives --set cc /usr/bin/clang
    update-alternatives --set c++ /usr/bin/clang++
    ;;
  alpine)
    apk add --no-cache bash cmake curl-dev curl g++ git ninja zlib-dev zip unzip tar jq
    ;;
  arch)
    pacman -Sy --noconfirm cmake ninja clang curl zip unzip git jq
    ;;
  *)
    echo "Unknown OS: $OS" && exit 1
    ;;
esac
