echo "Starting Controller"
./controller &

echo "Starting Node 4"
./node 4 &

echo "Starting Node 5 [Receiver]"
./node 5 receiver 1 &

echo "Starting Node 9 [Receiver]"
./node 9 receiver 0 &

echo "Starting Node 0 [Sender]"
./node 0 sender "this is node 0 multicast message" &

echo "Starting Node 3"
./node 3 &

sleep 40

echo "Starting Node 1 [Sender]"
./node 1 sender "this is node 1 multicast message" &
