import java.nio.ByteBuffer;

class JniDemo {
	static {
		System.loadLibrary("jnidemo");
	}

	public native void runExpressionEval();

	public native void runLimitTest();

	public static void main(String[] args) throws Exception {
		JniDemo demo = new JniDemo();
		demo.runExpressionEval();
		demo.runLimitTest();

		int memorySize = 10 * 1024 * 1024; // 10MB
		// 这里申请的堆外内存是受JVM堆外内存大小的限制的
		ByteBuffer buf = ByteBuffer.allocateDirect(memorySize);
        // JVM 启动参数 -XX:MaxDirectMemorySize=10M
        // 这里再申请内存会报错：java.lang.OutOfMemoryError: Direct buffer memory
		// ByteBuffer.allocateDirect(1);
	}
}
