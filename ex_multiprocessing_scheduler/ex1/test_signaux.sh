#!/bin/sh

echo "Recherche des PIDs de ex_comm_parent_child.o..."
PIDS=$(ps aux | grep './ex_comm_parent_child.o' | grep -v grep | awk '{print $2}')

echo "PIDs détectés : $PIDS"
SIGNALS="SIGHUP SIGINT SIGQUIT SIGABRT SIGTERM"

for PID in $PIDS; do
    echo "-----------------------------"
    echo "Test des signaux sur PID $PID"
    for SIG in $SIGNALS; do
        echo "Envoi de $SIG à $PID"
        kill -s $SIG $PID
        sleep 1
    done
done

echo "✅ Test terminé."
