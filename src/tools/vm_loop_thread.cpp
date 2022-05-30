#include "../service_header/vm_thread_tool.hpp"

namespace StaticLoopThreadMethod
{
    void *Run(void *args)
    {
        ((VMRunnable*)args)->Run(args);
    }
}

void AbsLoopThread::CreateRealThread()
{
    if (is_inited)
    {
        return;
    }
    is_inited = true;
    is_interrupt = false;
    int err = pthread_create(&real_thread, NULL, StaticLoopThreadMethod::Run, (void*)this);
    if (err != 0)
    {
        //日志：创建线程失败
        return;
    }
    pthread_join(real_thread, NULL);
}

void *AbsLoopThread::Run(void *args)
{
    while (is_interrupt)
    {
        Loop();
    }
}

void AbsLoopThread::Interrupt()
{
    is_interrupt = true;
}

TaskLoopThread::TaskLoopThread(VMLoopStrategy *strategy)
{
    this->strategy = strategy;
    queue = new VMAsyncQueue(16, 0x08, 8);

}

void TaskLoopThread::Loop()
{
    if (strategy->OnLoop(loop_count, queue))
    {
        VMHandler *handler = queue->Poll();
        handler->Handle();
        delete handler;
    }
}

class SpinLoopStrategy: VMLoopStrategy
{
public:
    bool OnLoop(uint64_t count, VMAsyncQueue* queue)
    {
        return !queue->IsEmpty();
    }
};

class BlockLoopStrategy: VMLoopStrategy
{
public:
    bool OnLoop(uint64_t count, VMAsyncQueue* queue)
    {
        queue->WaitForData();
        return true;
    }
};

