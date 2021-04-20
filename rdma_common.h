#ifndef RDMA_COMMON_H_
#define RDMA_COMMON_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

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

struct RemoteInformation
{
  uint32_t remote_qpn;
  uint16_t remote_lid;
  uint8_t* remote_gid;
};
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
bool StateTransitionToRTR(struct ibv_qp* qp, struct RdmaResource* rdma_res, struct RemoteInformation* peer_inf);

/**
* StateTransitionToRTS - Queue Pair State Transition from RTR to RTS
* @param qp : point to qp
* @return : true on success, false on error
**/
bool StateTransitionToRTS(struct ibv_qp* qp);
#endif // !RDMA_COMMON_H_

