#define RDMA_COMMON_H_
#ifndef RDMA_COMMON_H_

#include <stdio.h>
#include <stdbool.h>

#include <infiniband/verbs.h>
/* rdma control path */
struct RdmaResource
{

};
/**
* RdmaInit - resolve rdma device port and create needed resource
* @param DeviceName   : RDMA device name, default the first active one
* @param Memory       : pre-allocated memory
* @param MemorySize   :
* @return : true on success, false on error
**/
bool RdmaInit(char* DeviceName, void* Memory, size_t MemorySize);

bool RdmaCreateQueuePair();


#endif // !RDMA_COMMON_H_

