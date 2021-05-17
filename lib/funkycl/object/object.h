#ifndef __OBJECT_H
#define __OBJECT_H

#include <CL/cl.h>

/**
 * object.h - define OpenCL/FunkyCL objects 
 *
 * OpenCL objects (_cl_***) and their pointer objects are defined in CL/cl.h,
 * but no actual implementation.
 * i.e., typedef struct _cl_platform_id *    cl_platform_id;
 *
 * Funky CL objects (funkycl::***) are their implementation specific to Funky platform,
 * which override original OpenCL objects. 
 * 
 * A static downcast is used to access Funky CL objects with OpenCL pointer objects.
 *
 * Some functions refer to Xilinx XRT implementation (TODO: check an OSS license? Apache 2.0?)
 */

namespace funkycl {

class platform;

template  <typename FUNKY_OBJ, typename CL_OBJ>
class cl_object_base
{
  public:
    typedef FUNKY_OBJ funky_obj_type;
    typedef CL_OBJ cl_obj_type;
};

template <typename CL_OBJ>
typename CL_OBJ::funky_obj_type* funky_obj(CL_OBJ* cl_obj)
{
  return static_cast<typename CL_OBJ::cl_obj_type*>(cl_obj);
}

}  // funkycl


struct _cl_platform_id : public funkycl::cl_object_base<funkycl::platform, _cl_platform_id> {};

// struct _cl_platform_id : public funkycl::platform {};
// using _cl_platform_id = funkycl::platform;

// using _cl_device_id     = funkycl::device;    
// using _cl_context       = funkycl::context;
// using _cl_command_queue = funkycl::cmd_queue;
// using _cl_mem           = funkycl::mem;
// using _cl_program       = funkycl::program;
// using _cl_kernel        = funkycl::kernel;
// using _cl_event         = funkycl::event;
// using _cl_sampler       = funkycl::sampler;
 


#endif // __OBJECT_H
