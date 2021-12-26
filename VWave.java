import com.sun.tools.attach.VirtualMachine;

/**
 * VWave Bootstrap
 * use VirtualMachine to attach an agent on the JVM
 */
public class VWave {

    public static void main(String[] args) throws Exception{
        String pid = args[0];
        String agentPath = args[1];
        VirtualMachine vm = VirtualMachine.attach(pid);
        vm.loadAgent(agentPath);
    }
}