#ifndef RDMA_COMMON_H_
#define RDMA_COMMON_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <unistd.h>

#include <infiniband/verbs.h>
/* rdma control path */

#define CQ_MAX_DEPTH 128
#define QP_MAX_DEPTH 128
#define WR_MAX_SGE 1
#define MAX_INLINE_SIZE 256
#define DEFAULT_GID_INDEX 2
struct RdmaResource
{
  struct ibv_context* dev_ctx;
  struct ibv_device_attr dev_attr;
  uint8_t port_id;
  struct ibv_port_attr port_attr;
  struct ibv_pd* pd;
  struct ibv_cq* send_cq;
  struct ibv_cq* recv_cq;
  bool is_preallocated;
  void* memory;
  size_t memory_size;
  struct ibv_mr* mr;
};

struct RouteInf
{
  uint32_t r_key;
  uint32_t qpn;
  uint16_t lid;
  uint8_t  gid[16];
}__attribute__((packed));
/**
* RdmaInit - resolve rdma device and create needed resource
* @param dev_name : rdma device name
* @param port_id  : rdma device port id  
* @param rdma_res : rdma resource should be created     
* @return : true on success, false on error
**/
bool RdmaInit(char* dev_name, uint8_t port_id, struct RdmaResource* rdma_res);

/**
* RdmaDestoryRes - Destroy all rdma resource
* @param rdma_res : rdma resource should be destroyed
* @return : true on success, false on error
**/
bool RdmaDestroyRes(struct RdmaResource* rdma_res);

/**
* RdmaCreateQueuePair - Create queue pair
* @param qp       : point of point to qp
* @param rdma_res : necessary rdma resource to create qp
* @return : true on success, false on error
**/
bool RdmaCreateQueuePair(struct ibv_qp** qp, struct RdmaResource* rdma_res);

/**
* RdmaDestroyQueuePair - Destroy queue pair
* @param qp : point to qp
* @return : true on success, false on error
**/
bool RdmaDestroyQueuePair(struct ibv_qp* qp);

/**
* StateTransitionToINIT - Queue Pair State Transition from RESET to INIT
* @param qp       : point to qp
* @param rdma_res : necessary rdma resource to modify qp  
* @return : true on success, false on error
**/
bool StateTransitionToINIT(struct ibv_qp* qp, struct RdmaResource* rdma_res);

/**
* StateTransitionToRTR - Queue Pair State Transition from INIT to RTR
* @param qp       : point to qp
* @param rdma_res : necessary rdma resource to modify qp
* @param peer_inf : necessary peer route information to modify qp  
* @return : true on success, false on error
**/
bool StateTransitionToRTR(struct ibv_qp* qp, struct RdmaResource* rdma_res, struct RouteInf* peer_inf);

/**
* StateTransitionToRTS - Queue Pair State Transition from RTR to RTS
* @param qp : point to qp
* @return : true on success, false on error
**/
bool StateTransitionToRTS(struct ibv_qp* qp);

/**
* TCPSocketSetNonBlocking - set socket non-blocking
* @param sock :
* @return : true on success, false on error
**/
bool TCPSocketSetNonBlocking(int sock);

/**
* TCPSocketCreate - create new socket for listen/connect
* @return : new socket >=0
**/

int TCPSocketCreate();
/**
* TCPSocketListen - create tcp socket to listen for peer connection
* @param sock : socket to listen
* @return : true on success, false on error 
**/
bool TCPSocketListen(int sock);


/**
* TCPSocketAccept - accept peer connection
* @param sock : socket to listen 
* @return : if ret < 0, please try again
**/
int TCPSocketAccept(int sock);

/**
* TCPSocketConnect - conncet to peer
* @param sock : source socket id  
* @param dest_ip : destination ip addr
* @return : if ret < 0, please try again
**/
int TCPSocketConnect(int sock, char* dest_ip);

/**
* TCPSocketWrite - write data by tcp socket
* @param sock   : socket id
* @param buffer : write data buffer
* @param size   : write data size
* @return : On success, the number of bytes written is returned. On error, -1 is returned,
**/
int TCPSocketWrite(int sock, char* buffer, int size);

/**
* TCPSocketRead - read data by tcp socket
* @param sock   : socket id
* @param buffer : read data buffer
* @param size   : read data size
* @return : On success, the number of bytes read is returned. On error, -1 is returned,
**/
int TCPSocketRead(int sock, char* buffer, int size);

/**
* SendRouteInf - create local route information to exchange
* @param sock     : socket id
* @param qp_num   : qp number
* @param rdma_res : 
* @return : real send size, -1 on error
*/
int SendRouteInf(int sock, uint32_t qp_num, struct RdmaResource* rdma_res);

/**
* @param sock : socket id
* @param peer_inf : peer route information
* @return : real read size, -1 on error
**/
int RecvRouteInf(int sock, char* peer_inf);
#endif // !RDMA_COMMON_H_

