#!/usr/bin/env bash

set -o errexit
set -o pipefail
set -o nounset
# set -o xtrace

git clone https://gitlab.com/buildroot.org/buildroot.git /buildroot

cd /buildroot
git checkout -b csel 2022.08.3

rsync -a /workspace/config/board/ /buildroot/board/
rsync -a /workspace/config/configs/ /buildroot/configs/

make csel_defconfig
