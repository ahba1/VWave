
public class TestedDemo {
    public static void main(String[] args) throws InterruptedException {
        TestedDemo t = new TestedDemo();
        long first = System.currentTimeMillis();
        t.firstMethod();
        long firstEnd = System.currentTimeMillis();
        t.secondMethod();
        long secondEnd = System.currentTimeMillis();
        System.out.println("firstMethod Cost: " + (firstEnd - first) + "ms");
        System.out.println("secondMethod Cost: " + (secondEnd - firstEnd) + "ms");
    }

    public void firstMethod() throws InterruptedException {
        int a = 0;
        Thread.sleep(10);
        a = 1;
        endMethodPrint();
    }

    public void secondMethod() throws InterruptedException {
        float b = 1f;
        Thread.sleep(10);
        b = 1.2f;
        endMethodPrint();
    }

    public void endMethodPrint() {
        System.out.println("end print");
    }
}

