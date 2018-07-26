#define __CL_ENABLE_EXCEPTIONS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS // Use old API to support OpenCL 1.1 devices

#include <opencl_utils.h>
#include "filter.h"

extern "C"
JNIEXPORT void Java_project_timweri_openclfilter_OpenCLFilter_grayscale_1RGB(
    JNIEnv *env,
    jobject javaThis,
    jobject bmp
) {
    jint ret;

    cl_context context = createContext();

    // Query the device from the context - should be the GPU since we requested this before
    size_t deviceBufferSize;
    cl_int errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, 0, &deviceBufferSize);
    cl_device_id* contextDevices = static_cast<cl_device_id*>(malloc(deviceBufferSize));
    errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize, contextDevices, 0);
    cl_device_id device = contextDevices[0];

    cl_program program = createProgram(context, device, "kernel_filter.cl");
    cl_command_queue command_queue = clCreateCommandQueue(context, device, 0, &ret);

    // Get Bitmap_RGB class
    jclass bitmap_rgb = env->GetObjectClass(bmp);

    // Get Java field references
    jfieldID widthField = env->GetFieldID(bitmap_rgb, "w", "I");
    jfieldID heightField = env->GetFieldID(bitmap_rgb, "h", "I");

    jfieldID redField = env->GetFieldID(bitmap_rgb, "red", "[I");
    jfieldID greenField = env->GetFieldID(bitmap_rgb, "green", "[I");
    jfieldID blueField = env->GetFieldID(bitmap_rgb, "blue", "[I");

    // Get Java fields
    jint width = env->GetIntField(bmp, widthField);
    jint height = env->GetIntField(bmp, heightField);

    jintArray redArray = reinterpret_cast<jintArray>(env->GetObjectField(bitmap_rgb, redField));
    jintArray greenArray = reinterpret_cast<jintArray>(env->GetObjectField(bitmap_rgb, greenField));
    jintArray blueArray = reinterpret_cast<jintArray>(env->GetObjectField(bitmap_rgb, blueField));

    jsize length = env->GetArrayLength(redArray);

    // Extract RGB elements
    jint *red= env->GetIntArrayElements(redArray, 0);
    jint *green = env->GetIntArrayElements(greenArray, 0);
    jint *blue = env->GetIntArrayElements(blueArray, 0);

    // Create memory objects
    cl_mem w_inp = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(jint), NULL, &ret);
    cl_mem h_inp = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(jint), NULL, &ret);
    cl_mem red_inp = clCreateBuffer(context, CL_MEM_READ_ONLY, length * sizeof(jint), NULL, &ret);
    cl_mem green_inp = clCreateBuffer(context, CL_MEM_READ_ONLY, length * sizeof(jint), NULL, &ret);
    cl_mem blue_inp = clCreateBuffer(context, CL_MEM_READ_ONLY, length * sizeof(jint), NULL, &ret);
    cl_mem red_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY, length * sizeof(jint), NULL, &ret);
    cl_mem green_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY, length * sizeof(jint), NULL, &ret);
    cl_mem blue_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY, length * sizeof(jint), NULL, &ret);

    // Queue command to copy data to memory buffers
    ret = clEnqueueWriteBuffer(command_queue, w_inp, CL_TRUE, 0, sizeof(jint), &w_inp, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, h_inp, CL_TRUE, 0, length * sizeof(jint), &h_inp, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, red_inp, CL_TRUE, 0, length * sizeof(jint), red, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, green_inp, CL_TRUE, 0, length * sizeof(jint), green, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, blue_inp, CL_TRUE, 0, length * sizeof(jint), blue, 0, NULL, NULL);

    // Build program
    ret = clBuildProgram(program, 1, &device, NULL, NULL, NULL);

    // Create OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, "grayscale_RGB", &ret);

    // Set kernel arguments
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&w_inp);
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&h_inp);
    ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&red_inp);
    ret = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&green_inp);
    ret = clSetKernelArg(kernel, 5, sizeof(cl_mem), (void *)&blue_inp);
    ret = clSetKernelArg(kernel, 6, sizeof(cl_mem), (void *)&red_out);
    ret = clSetKernelArg(kernel, 7, sizeof(cl_mem), (void *)&green_out);
    ret = clSetKernelArg(kernel, 8, sizeof(cl_mem), (void *)&blue_out);

    // Execute the OpenCL kernel on the list
    size_t global_item_size = length; // Process the entire lists
    size_t local_item_size = 64; // Divide work items into groups of 64
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
                                 &global_item_size, &local_item_size, 0, NULL, NULL);

    // Read the result memory buffers on the device to the local variables
    ret = clEnqueueReadBuffer(command_queue, red_out, CL_TRUE, 0,
                              length * sizeof(jint), red, 0, NULL, NULL);
    ret = clEnqueueReadBuffer(command_queue, green_out, CL_TRUE, 0,
                              length * sizeof(jint), green, 0, NULL, NULL);
    ret = clEnqueueReadBuffer(command_queue, blue_out, CL_TRUE, 0,
                              length * sizeof(jint), blue, 0, NULL, NULL);

    // Release RGB elements and copy results back to Java arrays
    env->ReleaseIntArrayElements(redArray, red, 0);
    env->ReleaseIntArrayElements(greenArray, green, 0);
    env->ReleaseIntArrayElements(blueArray, blue, 0);

    // Pass result back to bitmap object
    env->SetObjectField(bmp, redField, redArray);
    env->SetObjectField(bmp, greenField, greenArray);
    env->SetObjectField(bmp, blueField, blueArray);

    // Cleanup OpenCL
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(w_inp);
    ret = clReleaseMemObject(h_inp);
    ret = clReleaseMemObject(red_inp);
    ret = clReleaseMemObject(blue_inp);
    ret = clReleaseMemObject(green_inp);
    ret = clReleaseMemObject(red_out);
    ret = clReleaseMemObject(green_out);
    ret = clReleaseMemObject(blue_out);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);
}
