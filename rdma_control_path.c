#include "rdma_common.h"

bool RdmaInit(char* dev_name, uint8_t port_id, struct RdmaResource* rdma_res)
{
  struct ibv_device** dev_list = NULL;
  struct ibv_device* dev = NULL;
  struct ibv_context* dev_ctx = NULL;
  int dev_num = 0;

  bool is_resolved = false;
  struct ibv_device_attr dev_attr;
  struct ibv_port_attr port_attr;

  struct ibv_cq* cq = NULL;

  struct ibv_pd* pd = NULL;

  int mr_flag;
  struct ibv_mr* mr = NULL;

  /* get device list*/
  dev_list = ibv_get_device_list(&dev_num);
  if (!dev_list) {
    printf("Failed to get device list!!!\n");
    return false;
  }
  if (!dev_num) {
    printf("Get %d devices\n");
    return false;
  }

  /* resolve device and port*/
  for (int dev_i = 0; dev_i < dev_num; dev_i++) {
    /* reslove device*/
    if (!dev_name) {
      dev = dev_list[dev_i];
    } 
    else {
      if (!strcmp(ibv_get_device_name(dev_list[dev_i]), dev_name)) {
        dev = dev_list[dev_i];
      }
    }

    /* resolve port*/
    if (dev) {
      dev_ctx = ibv_open_device(dev);
      if (!dev_ctx) {
        printf("Failed to open device");
        break;
      }

      memset(&dev_attr, 0, sizeof(dev_attr));
      if (ibv_query_device(dev_ctx, &dev_attr)) {
        printf("Failed to query device");
        break;
      }

      memset(&port_attr, 0, sizeof(port_attr));
      if (port_id > 0) {
        if (ibv_query_port(dev_ctx, port_id, &port_attr)) {
          printf("Failed to query port");
          break;
        }
        if (port_attr.state == IBV_PORT_ACTIVE) {
          is_resolved = true;
          break;
        }
      }
    
      for (uint8_t port_i = 1; port_i <= dev_attr.phys_port_cnt; port_i++) {
        memset(&port_attr, 0, sizeof(port_attr));
        if (ibv_query_port(dev_ctx, port_i, &port_attr)) {
          printf("Failed to query port\n");
          break;
        }
        if (port_attr.state == IBV_PORT_ACTIVE) {
          is_resolved = true;
          break;
        }
      }

      if (is_resolved) {
        break;
      }
    }
  }

  /* dev_list is useless, free it*/
  ibv_free_device_list(dev_list);

  if (!is_resolved) {
    printf("Failed to resolve device and port\n");
    ibv_close_device(dev_ctx);
    return false;
  }

  /* create completion queue */
  send_cq = ibv_create_cq(dev_ctx, CQ_MAX_DEPTH, NULL, NULL, 0);
  recv_cq = ibv_create_cq(dev_ctx, CQ_MAX_DEPTH, NULL, NULL, 0);
  if (!send_cq || !recv_cq) {
    printf("Failed to create cq\n");
    ibv_close_device(dev_ctx);
    return false;
  }

  /* create protection domain */
  pd = ibv_alloc_pd(dev_ctx);
  if (!pd) {
    printf("Failed to malloc pd\n");
    ibv_destroy_cq(send_cq);
    ibv_destroy_cq(recv_cq);
    ibv_close_device(dev_ctx);
    return false;
  }

  /* register memeory */
  if (!rdma_res->is_preallocated) {
    rdma_res->memory_size = 64 * 1024 * 1024;
    rdma_res->memory = (void* )malloc(rdma_res->memory_size);
    printf("There is no pre-allocated memory, malloc %d memory for rdma\n", rdma_res->memory_size);
  }
  mr_flag = IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_ATOMIC;
  mr = ibv_reg_mr(pd, rdma_res->memory, rdma_res->memory_size,  mr_flag);
  if (!mr) {
    printf("Failed to register memory\n");
    if (!rdma_res->is_preallocated) {
      free(rdma_res->memory);
    }
    ibv_dealloc_pd(pd);
    ibv_destroy_cq(send_cq);
    ibv_destroy_cq(recv_cq);
    ibv_close_device(dev_ctx);
    return false;
  }

  /* have a long copy at rdma_res*/
  rdma_res->dev_ctx = dev_ctx;
  rdma_res->send_cq = send_cq;
  rdma_res->recv_cq = recv_cq;
  rdma_res->pd = pd;
  rdma_res->mr = mr;
  memcpy(&(rdma_res->dev_attr), &dev_attr, sizeof(dev_attr));
  memcpy(&(rdma_res->port_attr), &port_attr, sizeof(port_attr));
  return true;

}

bool RdmaDestroyRes(struct RdmaResource* rdma_res)
{
  if (rdma_res->send_cq) {
    if (ibv_destroy_cq(rdma_res->send_cq)) {
      printf("Failed to destory send_cq\n");
      return false;
    }
  }

  if (rdma_res->recv_cq) {
    if (ibv_destroy_cq(rdma_res->recv_cq)) {
      printf("Failed to destory recv_cq\n");
      return false;
    }
  }

  if (rdma_res->mr) {
    if (ibv_dereg_mr(rdma_res->mr)) {
      printf("Failed to deregister memory\n");
      return false;
    }
  }

  if (rdma_res->is_preallocated) {
    free(rdma_res->memory);
  }

  if (rdma_res->pd) {
    if (ibv_dealloc_pd(rdma_res->pd)) {
      printf("Failed to dealloc pd\n");
      return false;
    }
  }

  if (rdma_res->dev_ctx) {
    if (ibv_close_device(rdma_res->dev_ctx)) {
      printf("Failed to close device\n");
      return false;
    }
  }

  return true;
}

bool RdmaCreateQueuePair(struct ibv_qp** qp, struct RdmaResource* rdma_res)
{
  struct ibv_qp_init_attr qp_attr;
  memset(&qp_attr, 0, sizeof(qp_attr));

  /* qp type select RC for write/read */
  qp_attr.qp_type = IBV_QPT_RC;

  qp_attr.send_cq = rdma_res->send_cq;
  qp_attr.recv_cq = rdma_res->recv_cq;

  qp_attr.cap.max_send_wr = QP_MAX_DEPTH;
  qp_attr.cap.max_recv_wr = QP_MAX_DEPTH;

  qp_attr.cap.max_send_sge = WR_MAX_SGE;
  qp_attr.cap.max_recv_sge = WR_MAX_SGE;

  qp_attr.cap.max_inline_data = MAX_INLINE_SIZE;
  
  *qp = ibv_create_qp(rdma_res->pd, &qp_attr);
  if (!(*qp)) {
    printf("Failed to create qp, whose qp_num is %d\n", *qp->qp_num);
    return false;
  }

  return true;
}

bool RdmaDestroyQueuePair(struct ibv_qp* qp)
{
  if (ibv_destroy_qp(qp)) {
    printf("Failed to destroy qp, whose qp_num is %d\n", qp->qp_num);
    return false;
  }
  return true;
}

void main()
{
  struct RdmaResource rdma_res;
  rdma_res.is_preallocated = false;
  RdmaInit((char*)NULL, 0, &rdma_res);
  RdmaDestroyRes(&rdma_res);
  struct ibv_qp* qp;
  RdmaCreateQueuePair(&qp, &rdma_res);
  RdmaDestroyQueuePair(qp);
}