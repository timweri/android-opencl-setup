package project.timweri.openclfilter;

public class OpenCLFilter {
    static {
        System.loadLibrary("openclfilter");
    }
    public static native int[] vector_addition(int[] A, int[] B, int LIST_SIZE);

    // Test Driver
    public static void main() {
        System.out.println("Test");
        int[] A = new int[]{1,2,3};
        int[] B = new int[]{3,3,3};
        int[] C = new OpenCLFilter().vector_addition(A, B, 3);
        System.out.println(C);
    }
}
