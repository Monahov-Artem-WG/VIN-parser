#include <windows.h>
#include <iostream>

using namespace std;

#define N 5                //Число философов
#define LEFT (i-1)%N    //Левый сосед философа с номером i
#define RIGHT (i+1)%N    //Правый сосед философа сномером i
#define THINKING 0        //Философ размышляет
#define HUNGRY 1        //Философ получается получить вилки
#define EATING 2        //Философ ест

int state[N];            //Состояния каждого философа


struct Philosopher        //Описание философа: номер, алгоритм
{
    int number;
    int algorithm;
};




CRITICAL_SECTION cs;        //Для критических секций: синхрон. процессов    
CRITICAL_SECTION cs_forks;    //и синхр. вилок

HANDLE philMutex[N];    //Каждому философу по мьютексу
void think(int i)        //Моделирует размышления философа
{
    EnterCriticalSection(&cs);    //Вход в критическую секцию
    cout << "Philosopher number " << i << " thinking" << endl;
    LeaveCriticalSection(&cs);    //Выход из критической секции
}

void eat(int i)            //Моделирует обед философа
{
    EnterCriticalSection(&cs);    //Вход в критическую секцию
    cout << "Philosopher number " << i << " eating" << endl;
    LeaveCriticalSection(&cs);    //Выход из критической секции
}

void test(int i)    //Проверка возможности начать философом обед
{
    if (state[i] == HUNGRY && state[left] != EATING && state[right] != EATING)
    {
        state[i] = EATING;
        ReleaseMutex(philMutex[i]);

    }
}

void take_forks(int i)    //Взятие вилок
{
    EnterCriticalSection(&cs_forks);                //Вход в критическую секцию
    state[i] = HUNGRY;                                //Фиксация наличия голодного философа
    test(i);                                        //Попытка получить две вилки
    LeaveCriticalSection(&cs_forks);                //Выход из критической секции
    WaitForSingleObject(philMutex[i], INFINITE);    //Блокировка если вилок не досталось


}

void put_forks(int i)    //Философ кладет вилки обратно
{
    EnterCriticalSection(&cs_forks);    //Вход в критическую секцию    
    state[i] = THINKING;                    //Философ перестал есть
    test(LEFT);                            //Проверить может ли есть сосед слева
    test(RIGHT);                        //Проверить может ли есть сосед справа
    LeaveCriticalSection(&cs_forks);    //Выход из критической секции
}



DWORD WINAPI philosopher(void* lParam)    //Собственно модель философа
{
    Philosopher phil = *((Philosopher*)lParam);    //Получаем модель философа

    while (1)    //Моделируем обед
    {    //Берем вилки

        if (phil.algorithm == 2)    //Берем обе вилки
        {
            think(phil.number);        //Думаем
            take_forks(phil.number);//Берем вилки    
            eat(phil.number);        //Едим
        }
        //кладем вилки

        if (phil.algorithm == 2)    //Кладем вилки по третьему алгоритму
            put_forks(phil.number);

        Sleep(10);    //Даем время на переключение контекста
    }
}
int main()
{

    Philosopher phil[N];


    for (int i = 0; i < N; i++)    //Создаем мьютексы
    {
        philMutex[i] = CreateMutex(NULL, FALSE, NULL);

    }
    InitializeCriticalSection(&cs);        //Инициализируем
    InitializeCriticalSection(&cs_forks);    //критические секции

    DWORD id[N];        //Идентификаторы потоков
    HANDLE hThread[N];    //Потоки
    for (int i = 0; i < N; i++)//Создаем потоки
    {
        hThread[i] = CreateThread(NULL, NULL, &philosopher, &phil[i], NULL, &id[i]);

    }
    Sleep(INFINITE);    //Чтобы потоки успели выполнится с корректными значениями 
    while (1);
}