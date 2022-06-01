
public class MethodTest {

    public static void main(String[] args) {
        MethodTest mt = new MethodTest();
        mt.firstMethod();
        mt.secondMethod();
    }

    public void firstMethod() {
        String a = "";
        System.out.println("in first method");
        a = "ssss";
    }

    public void secondMethod() {
        float b = 1.0f;
        System.out.println("in second method");
    }

    public void endMethodPrint() {
        System.out.println("end print");
    }
}