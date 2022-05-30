public class ExThreadTest implements Runnable {
    public static void main(String[] args) throws InterruptedException {
        for (int i = 0; i < 5; i++) {
            Thread t1 = new Thread(new ExThreadTest(null),"A"+i);
            Thread t2 = new Thread(new ExThreadTest(t1),"B"+i);
            Thread t3 = new Thread(new ExThreadTest(t2),"C"+i);
            t1.start();
            t2.start();
            t3.start();
            Thread.sleep(10); //这里是要保证只有t1、t2、t3为一组，进行执行才能保证t1->t2->t3的执行顺序。
            //t1 : A
            //t2 : B before : A
            //t3 : C before : C
        }

    }

    private Thread beforeThread;

    public ExThreadTest(Thread beforeThread) {
        this.beforeThread = beforeThread;
    }

    @Override
    public void run() {
        if(beforeThread!=null) {
            try {
                beforeThread.join(); 
                System.out.println(Thread.currentThread().getName()+" invoke "+beforeThread.getName()+"'s join");
            }catch(Exception e){
                e.printStackTrace();
            }
        }else {
            System.out.println(Thread.currentThread().getName() + " don't need to join");
        }
    }
}