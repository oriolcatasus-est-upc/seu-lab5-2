#include "mbed.h"
#include "rtos.h"
#include <Mutex.h>

typedef struct {
  int value;
} message_t;

Mutex mutex;

const int queue_size = 10;
ConditionVariable cond(mutex);
MemoryPool<message_t, queue_size> mpool;
Queue<message_t, queue_size> numbers_queue;

Thread thread_even;
Thread thread_odds;
Thread thread_first;
Thread thread_second;
Thread thread_third;

Timer global_timer;
Timer thread_change_timer;

int pointer = 0;
bool with_priority = true;

void stop_timer() {
    thread_change_timer.stop();
    int t = thread_change_timer.read_ms();
    if (t > 0) {
        printf("Time to change thread: %d\r\n", t);
        thread_change_timer.reset();
    }
}

void send_message(int value, const char* id)
{
    mutex.lock();
    stop_timer();
    message_t *message = mpool.alloc();
    message->value = value;
    numbers_queue.put(message);
    pointer++;
    printf("%s producer value %d\r\n", id, message->value);
    while (pointer == queue_size){
        printf("The queue is full, wait a consumer\r\n");
        cond.wait();
    }
    cond.notify_all();
    mutex.unlock();
    thread_change_timer.start();
}

void get_message(const char* id)
{
    mutex.lock();
    while (pointer == 0){
        printf("The queue is empty, wait for a producer.\r\n");
        cond.wait();
    }
    osEvent evt = numbers_queue.get();
    pointer--;
    if (evt.status == osEventMessage) {
        message_t *message = (message_t*)evt.value.p;
        printf("%s thread got value %.d .\r\n", id, message->value);
        mpool.free(message);
    }
    cond.notify_all();
    mutex.unlock();
}

void thread_even_manager()
{
    while(true)
    {
        //ThisThread::sleep_for(rand()%100);
        //ThisThread::sleep_for(1);
        //ThisThread::sleep_for(100);
        int even_value = 2*(rand()%100);
        send_message(even_value, "Even");
    }
}

void thread_odds_manager()
{
    while(true)
    {
        //ThisThread::sleep_for(rand()%100);
        //ThisThread::sleep_for(100);
        int even_value = 2*(rand()%100) + 1;
        send_message(even_value, "Odds");
    }
}

void thread_first_manager()
{
    while(true)
    {
        //ThisThread::sleep_for(rand()%3*500);
        //ThisThread::sleep_for(1);
        //ThisThread::sleep_for(100);
        get_message("First");
    }
}

void thread_second_manager()
{
    while(true)
    {
        //ThisThread::sleep_for(rand()%3*500);
        //ThisThread::sleep_for(1);
        //ThisThread::sleep_for(100);
        get_message("Second");
    }
}

void thread_third_manager()
{
    while(true)
    {
        //ThisThread::sleep_for(rand()%3*500);
        //ThisThread::sleep_for(1);
        //ThisThread::sleep_for(100);
        get_message("Third");
    }
}

// main() runs in its own thread in the OS
int main()
{
    printf("Starting program...\r\n");

    osThreadSetPriority(osThreadGetId(), osPriorityHigh7);

    thread_even.start(thread_even_manager);
    thread_odds.start(thread_odds_manager);
    thread_first.start(thread_first_manager);
    thread_second.start(thread_second_manager);
    thread_third.start(thread_third_manager);

    if (with_priority) {
        thread_even.set_priority(osPriorityHigh);
        thread_odds.set_priority(osPriorityHigh);
        thread_first.set_priority(osPriorityHigh);
    }

    global_timer.start();

    while(global_timer.read_ms() < 10000) {
        ThisThread::sleep_for(1000);
    }

    thread_even.terminate();
    thread_odds.terminate();
    thread_first.terminate();
    thread_second.terminate();
    thread_third.terminate();
}

