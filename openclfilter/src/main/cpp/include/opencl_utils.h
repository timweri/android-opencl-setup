#ifndef ANDROID_OPENCL_SETUP_OPENCL_UTILS_H
#define ANDROID_OPENCL_SETUP_OPENCL_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <jni.h>

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
extern "C" {
    #include <CL/cl.h>
}

#define MAX_SOURCE_SIZE (0x100000)

cl_context createContext();
cl_program createProgram(cl_context context, cl_device_id device, const char* fileName);

#endif //ANDROID_OPENCL_SETUP_OPENCL_UTILS_H