# MiniRPC
RPC based on RDMA network

rdma_control_path.c is finished

Compile rdma_control_path.c:

gcc -o test rdma_control_path.c -std=gnu99 -libverbs

In Server:
./test -s

In Client:
./test -i ${server_ip}  
