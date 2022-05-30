#include "../service_header/vm_thread_tool.hpp"


namespace StaticVMAsyncQueueMethod
{
    VMHandler *GetBetter(VMHandler *first, VMHandler *second)
    {
        return first->GetPriority() < second->GetPriority() ? second : first; 
    }

    VMHandler *GetOlder(VMHandler *first, VMHandler *second)
    {
        if (first->GetAge() == second->GetAge())
        {
            return GetBetter(first, second);
        }
        return first->GetAge() < second->GetAge() ? second : first;
    }
}

class EmptyHeaderHandler : public VMHandler
{
public:
    EmptyHeaderHandler(): VMHandler(-1, 0, NULL) {}

    virtual void Handle() { return; }
};

VMAsyncQueue::VMAsyncQueue(uint32_t max_size, uint8_t reject_strategy, uint32_t age_threshold)
{
    this->max_size = max_size;
    this->reject_strategy = reject_strategy;
    this->age_threshold = age_threshold;

    this->head = new EmptyHeaderHandler();
    this->tail = new EmptyHeaderHandler();
    this->head->SetNext(tail);
    this->size = 0;

    sem_init(&full, 0, 0);
    sem_init(&empty, 0, max_size);
    pthread_mutex_init(&mutex, NULL);
}

VMAsyncQueue::~VMAsyncQueue()
{
    Clear();
    sem_destroy(&full);
    sem_destroy(&empty);
    pthread_mutex_destroy(&mutex);
}

VMHandler *VMAsyncQueue::RealPoll()
{
    if (IsEmpty())
    {
        return NULL;
    }
    VMHandler *index = head->GetNext();
    VMHandler *ret = index;
    while (index != tail)
    {
        ret = StaticVMAsyncQueueMethod::GetOlder(index, ret);
        index = index->GetNext();
    }
    if (ret->GetAge() < age_threshold)
    {
        index = head->GetNext();
        while (index != tail)
        {
            ret = StaticVMAsyncQueueMethod::GetBetter(index, ret);
            index->IncreaseAge();
            index = index->GetNext();
        }
    }
    size--;
    return ret;
}

void VMAsyncQueue::RealOffer(VMHandler *data)
{
    
    if (size != max_size)
    {
        VMHandler *next = head->GetNext();
        head->SetNext(data);
        data->SetNext(next);
        size++;
    }
    else 
    {
        switch (reject_strategy)
        {
            case 0x02:
                {
                    data->Handle();
                    delete data;
                    break;
                }
            case 0x04:
                {
                    VMHandler *h = head->GetNext();
                    head->SetNext(h->GetNext());
                    delete h;
                    size--;
                    RealOffer(data);
                    break;
                }
            case 0x08:
                {
                    delete data;
                }
                break;
            case 0x01:
            default:
                {
                    throw reject_strategy;
                }
        }
    }
}

VMHandler *VMAsyncQueue::Poll()
{
    VMHandler *ret;
    sem_wait(&full);
    ret = RealPoll();
    sem_post(&empty);
    return ret;
}

void VMAsyncQueue::Offer(VMHandler *data)
{
    sem_wait(&empty);
    pthread_mutex_lock(&mutex);
    RealOffer(data);
    pthread_mutex_unlock(&mutex);
    sem_post(&full);
}

void VMAsyncQueue::Clear()
{
    VMHandler *index = head->GetNext();
    while (index != tail)
    {
        VMHandler *temp = index->GetNext();
        delete index;
        index = temp;
    }
    delete head;
    delete tail;
}

bool VMAsyncQueue::IsEmpty()
{
    return head->GetNext() == tail;
}

void VMAsyncQueue::WaitForData()
{
    sem_wait(&full);
}

