#!/bin/sh

echo "🔧 Montage du système de fichiers cgroup"
mount -t tmpfs none /sys/fs/cgroup

echo "📁 Création du sous-répertoire 'cpuset'"
mkdir /sys/fs/cgroup/cpuset

echo "📦 Montage du sous-système cpuset (cpu + cpuset)"
mount -t cgroup -o cpu,cpuset cpuset /sys/fs/cgroup/cpuset

echo "📂 Création du groupe 'high'"
mkdir /sys/fs/cgroup/cpuset/high

echo "📂 Création du groupe 'low'"
mkdir /sys/fs/cgroup/cpuset/low

echo "⚙️  Configuration de 'high' : CPU 3, mémoire 0"
echo 3 > /sys/fs/cgroup/cpuset/high/cpuset.cpus
echo 0 > /sys/fs/cgroup/cpuset/high/cpuset.mems

echo "⚙️  Configuration de 'low' : CPU 2, mémoire 0"
echo 2 > /sys/fs/cgroup/cpuset/low/cpuset.cpus
echo 0 > /sys/fs/cgroup/cpuset/low/cpuset.mems