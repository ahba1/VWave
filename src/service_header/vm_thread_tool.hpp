#include <pthread.h>
#include <iostream>
#include <semaphore.h>

class VMHandler
{
private:
    uint64_t age;
    uint32_t priority;
    VMHandler *next;

public:

    VMHandler(uint64_t age, uint32_t priority, VMHandler *next): age(age), priority(priority), next(next)
    {

    }

    uint32_t GetPriority()
    {
        return priority;
    }

    virtual void Handle()
    {

    }

    void IncreaseAge()
    {
        age++;
    }

    uint64_t GetAge()
    {
        return age;
    }

    VMHandler *GetNext()
    {
        return next;
    }

    void SetNext(VMHandler *next)
    {
        this->next = next;
    }
};

class VMAsyncQueue
{
private:
    sem_t full;
    sem_t empty;
    pthread_mutex_t mutex;
    VMHandler *head;
    VMHandler *tail;
    uint32_t size;
    uint32_t max_size;
    uint8_t reject_strategy;
    uint32_t age_threshold;

    VMHandler *RealPoll();
    void RealOffer(VMHandler *data);

public:
    VMAsyncQueue(uint32_t max_size, uint8_t reject_strategy, uint32_t age_threshold);

    ~VMAsyncQueue();

    VMHandler *Poll();

    void Offer(VMHandler *data);

    void Clear();

    bool IsEmpty();

    void WaitForData();
};

class VMLoopStrategy
{
public:
    virtual bool OnLoop(uint64_t count, VMAsyncQueue* queue) = 0; 
};

class VMRunnable
{
public:
    virtual void *Run(void *args) = 0; 
};

class AbsLoopThread : public VMRunnable
{
private:
    pthread_t real_thread;
    volatile bool is_interrupt;
    volatile bool is_inited = false;

    void CreateRealThread();

protected:
    uint64_t loop_count = 0;

public:
    virtual void *Run(void *args);

    virtual void Loop() = 0;

    void Interrupt();
};

class TaskLoopThread : public AbsLoopThread
{
private:
    VMLoopStrategy *strategy;
    VMAsyncQueue *queue;

public:
    TaskLoopThread(VMLoopStrategy *strategy);

    ~TaskLoopThread();
    
    VMAsyncQueue GetQueue();

    void Loop();
};