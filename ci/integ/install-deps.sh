#!/bin/bash
set -euo pipefail

OS=$1

install_system_deps() {
  case "$OS" in
    al2023|al2023-arm)
      dnf install -y tar gzip git jq
      ;;
    ubuntu)
      apt-get update && apt-get install -y git unzip curl jq
      ;;
    alpine)
      apk add --no-cache bash tar git jq
      ;;
    arch)
      pacman -Syu --noconfirm tar git jq
      ;;
    *)
      echo "Unknown OS: $OS" && exit 1
      ;;
  esac
}

install_build_deps() {
  case "$OS" in
    al2023|al2023-arm)
      dnf install -y cmake ninja-build gcc-c++ openssl-devel libcurl-devel zip
      ;;
    ubuntu)
      apt-get install -y clang zlib1g-dev libssl-dev libcurl4-openssl-dev cmake ninja-build zip
      update-alternatives --set cc /usr/bin/clang
      update-alternatives --set c++ /usr/bin/clang++
      ;;
    alpine)
      apk add --no-cache cmake curl-dev g++ ninja openssl-dev zlib-dev zip unzip
      ;;
    arch)
      pacman -S --noconfirm cmake ninja clang curl zip unzip
      ;;
    *)
      echo "Unknown OS: $OS" && exit 1
      ;;
  esac
}

install_system_deps
install_build_deps
