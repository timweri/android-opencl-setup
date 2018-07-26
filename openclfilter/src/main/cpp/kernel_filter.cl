__kernel void grayscale_RGB(__global const int w,
                            __global const int h,
                            __global const int *red_inp,
                            __global const int *green_inp,
                            __global const int *blue_inp,
                            __global int *red_out,
                            __global int *green_out,
                            __global int *blue_out) {
    int pid = get_global_id(0);

    int gray = (red_inp[pid] + green_inp[pid] + blue_inp[pid]) / 3;

    red_out[pid] = gray;
    green_out[pid] = gray;
    blue_out[pid] = gray;
}