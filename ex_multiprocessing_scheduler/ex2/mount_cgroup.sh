#!/bin/sh

echo "🔧 Montage du système de fichiers cgroup"
mount -t tmpfs none /sys/fs/cgroup

echo "📁 Création du sous-répertoire 'memory'"
mkdir /sys/fs/cgroup/memory

echo "📦 Montage du sous-système memory"
mount -t cgroup -o memory memory /sys/fs/cgroup/memory

echo "📂 Création du groupe de contrôle mémoire 'mem'"
mkdir /sys/fs/cgroup/memory/mem

echo "📌 Ajout du shell courant (PID $$) au groupe de contrôle"
echo $$ > /sys/fs/cgroup/memory/mem/tasks

echo "💾 Définition de la limite mémoire à 20 MiB"
echo 20M > /sys/fs/cgroup/memory/mem/memory.limit_in_bytes

echo "✅ Configuration terminée. Lancement du programme."
exec ./mem_cgroup_ctrl.o
