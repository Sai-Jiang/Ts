#!/bin/bash

tc qdisc show dev lo

tc qdisc add dev lo root netem delay 50ms
./UDP_Server_Ts > UDP_loss0_delay50 &
./UDP_Client_Ts &
wait && echo "UDP_loss0_delay50 done"
./TCP_Server_Ts > TCP_loss0_delay50 &
./TCP_Client_Ts &
wait && echo "TCP_loss0_delay50 done"
tc qdisc del dev lo root netem delay 50ms


tc qdisc add dev lo root netem loss 5% delay 50ms
./UDP_Server_Ts > UDP_loss5_delay50 &
./UDP_Client_Ts &
wait && echo "UDP_loss5_delay50 done"
./TCP_Server_Ts > TCP_loss5_delay50 &
./TCP_Client_Ts &
wait && echo "TCP_loss5_delay50 done"
tc qdisc del dev lo root netem loss 5% delay 50ms


tc qdisc add dev lo root netem loss 10% delay 50ms
./UDP_Server_Ts > UDP_loss10_delay50 &
./UDP_Client_Ts &
wait && echo "UDP_loss10_delay50 done"
./TCP_Server_Ts > TCP_loss10_delay50 &
./TCP_Client_Ts &
wait && echo "TCP_loss10_delay50 done"
tc qdisc del dev lo root netem loss 10% delay 50ms


tc qdisc add dev lo root netem loss 20% delay 50ms
./UDP_Server_Ts > UDP_loss20_delay50 &
./UDP_Client_Ts &
wait && echo "UDP_loss20_delay50 done"
./TCP_Server_Ts > TCP_loss20_delay50 &
./TCP_Client_Ts &
wait && echo "TCP_loss20_delay50 done"
tc qdisc del dev lo root netem loss 20% delay 50ms
