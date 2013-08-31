echo "Starting Controller"
./controller &

echo "Starting Node 4"
./node 4 &

echo "Starting Node 5"
./node 5 &

echo "Starting Node 9 [Receiver]"
./node 9 receiver 0 &

echo "Starting Node 0 [Sender]"
./node 0 sender "this is node 0 multicast message" &

echo "Starting Node 8 [Receiver]"
./node 8 receiver 0 &