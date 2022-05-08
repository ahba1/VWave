
public class TestedDemo {
    private int a = 0;
    private float b = 1f;
    public static void main(String[] args) {
        TestedDemo t = new TestedDemo();
        t.firstMethod();
        t.secondMethod();
    }

    public void firstMethod() {
        int a = 0;
        System.out.println("in first method");
        a = 1;
    }

    public void secondMethod() {
        float b = 1f;
        System.out.println("in second method");
        b = 2f;
    }
}