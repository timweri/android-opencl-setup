#include <opencl_utils.h>

// Helper function to return the OpenCL context containing the phone's GPU
cl_context createContext() {
    cl_int ret;
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;

    // Take only the first platform and the first device
    // Phone normally has 1 GPU
    ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);

    cl_context_properties contextProperties[] = {
            CL_CONTEXT_PLATFORM,
            reinterpret_cast<cl_context_properties>(platform_id),
            0
    };

    cl_context context = clCreateContext(contextProperties, 1, &device_id, NULL, NULL, &ret);

    return context;
}

// Helper function to load the kernel source code from a suitable (text) file and setup an OpenCL program object
cl_program createProgram(cl_context context, cl_device_id device, const char* fileName) {
    FILE *fp;
    char *source_str;
    size_t source_size;
    cl_int ret;

    fp = fopen(fileName, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open kernel file.\n");
        exit(1);
    }

    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    cl_program program = clCreateProgramWithSource(context, 1,
                                                   (const char **)&source_str, (const size_t *)&source_size, &ret);

    ret = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    return program;
}


