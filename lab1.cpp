#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using namespace std;

// Глобальные переменные
mutex mtx;                  
condition_variable cv;      
bool ready = false;         
int count = 0;              
const int max_count = 10;   

// Функция потока-поставщика
void provider()
{
    while (true)
    {
        {
            unique_lock<mutex> lock(mtx);
            if (count >= max_count) 
                break;

            if (ready) 
                continue;

            ready = true;        
            count++;             
            cout << "Event " << count << " provided" << endl;
        }
        cv.notify_one();          
        this_thread::sleep_for(chrono::seconds(1)); 
    }
}

// Функция потока-потребителя
void consumer()
{
    while (true)
    {
        unique_lock<mutex> lock(mtx);
        if (count >= max_count && !ready) 
            break;

        if (!ready) 
        {
            cv.wait(lock);
            continue;
        }

        // Обрабатываем событие
        cout << "Event " << count << " consumed" << endl;
        ready = false; 
    }
}

int main()
{
    // Создаем и запускаем потоки
    thread t1(provider);
    thread t2(consumer);

    // Ожидаем завершения потоков
    t1.join();
    t2.join();

    return 0;
}
