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
struct RdmaResource
{
  struct ibv_context* dev_ctx;
  struct ibv_device_attr dev_attr;
  struct ibv_port_attr port_attr;
  struct ibv_pd* pd;
  struct ibv_cq* send_cq;
  struct ibv_cq* recv_cq;
  bool is_preallocated;
  void* memory;
  size_t memory_size;
  struct ibv_mr* mr;
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
* @param qp       : point of point to qp
* @param rdma_res : necessary rdma resource to create qp
* @return : true on success, false on error
**/
bool RdmaCreateQueuePair(struct ibv_qp** qp, struct RdmaResource* rdma_res);

/**
* @param qp : point to qp
* @return : true on success, false on error
**/
bool RdmaDestroyQueuePair(struct ibv_qp* qp);
#endif // !RDMA_COMMON_H_

