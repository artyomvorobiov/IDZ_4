#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>

const int bufSize = 3;
int buf[3] = {-1,-1,-1} ; //буфер(кабинеты)
int cab[3] = {-1, -1, -1};
int front = 0 ; //индекс для чтения из буфера
int rear = 0 ; //индекс для записи в буфер
int count = 0 ; //количество занятых ячеек буфера
int y;
// int t = 0;
// инициализатор генератора случайных чисел

pthread_mutex_t mutex ; // мьютекс для условных переменных

// поток-писатель блокируется этой условной переменной,
// когда количество занятых ячеек становится равно bufSize
pthread_cond_t not_full ;

// поток-читатель блокируется этой условной переменной,
// когда количество занятых кабинетов становится равно 0
pthread_cond_t not_empty ;

//стартовая функция потоков – посетители
[[noreturn]] void *Producer(void *param) {
    int pNum = *((int*)param);
    // t = rand()&pNum;
    int data, i ;
    while (1) {
        //создать элемент для буфера
        //поместить элемент в буфер
        pthread_mutex_lock(&mutex) ; //защита операции записи

        //заснуть, если количество занятых кабинетов равно 0
        while (count == 0) {
            pthread_cond_wait(&not_full, &mutex) ;
        }

        //запись в общий буфер
        front = (front+1)%bufSize ;
        count-- ;
        // count++ ; //появилась занятая ячейка

        //конец критической секции
        pthread_mutex_unlock(&mutex) ;
        //sleep(1);
        if (cab[front] != -1) {
            // count++ ;
            y = cab[front];
            cab[front] = -1;
            //обработать полученный элемент
            printf("Patient %d: LEFT from cabinet [%d]\n", y, front + 1);
            sleep(3);
        }
        //sleep(3);
        //разбудить потоки-администраторы после добавления элемента в буфер
        pthread_cond_broadcast(&not_empty) ;
    }
    return NULL;
}

//стартовая функция потоков – администраторы
[[noreturn]] void *Consumer(void *param) {
    int cNum = *((int*)param);
    int result ;
    while (1) {
        //извлечь элемент из буфера
        pthread_mutex_lock(&mutex) ; //защита операции чтения
        //заснуть, если количество занятых кабинетов равно 0
        while (count == 3) {
            pthread_cond_wait(&not_empty, &mutex) ;
        }

        //изъятие из общего буфера – начало критической секции
        // front = (front+1)%bufSize ; //критическая секция
        rear = (rear+1)%bufSize ;
        count++ ; //занятая ячейка стала свободной

        //конец критической секции

        pthread_mutex_unlock(&mutex) ;
        //разбудить потоки-писатели после получения элемента из буфера
        pthread_cond_broadcast(&not_full) ;
        if ((cab[rear%bufSize] == -1) && (cab[(rear+1)%bufSize] != cNum) && (cab[(rear+2)%bufSize] != cNum)) {
            // count-- ;
            cab[rear] = cNum;
            printf("Patient %d: ENTERED cabinet [%d]\n", cNum, rear + 1);
            sleep(3);
        }
        //sleep(3);
    }
    return NULL;
}

int main() {
    int i = 0, n;
    //инициализация мутексов и семафоров
    pthread_mutex_init(&mutex, nullptr) ;
    pthread_cond_init(&not_full, nullptr) ;
    pthread_cond_init(&not_empty, nullptr) ;


    //запуск постетителей и администраторов
    std::cout << "Enter the number of patients today:" << std::endl;
    std::cin >> n;
    if (n > 0) {

        pthread_t threadP[2];
        int producers[2];
        for (i = 0; i < 2; i++) {
            producers[i] = i + 1;
            pthread_create(&threadP[i], nullptr, Producer, (void *) (producers + i));
        }

        pthread_t threadC[n];
        int consumers[n];
        for (i = 0; i < n; i++) {
            consumers[i] = i + 1;
            pthread_create(&threadC[i], nullptr, Consumer, (void *) (consumers + i));
        }



        for (i = 0; i < n; i++) {
            pthread_join(threadP[i], nullptr);
        }

        for (i = 0; i < 2; i++) {
            pthread_join(threadC[i], nullptr);
        }
    }
    else if (n == 0) {
        std::cout << "Today there were no patients! Doctors can chill." << std::endl;
    }
    else {
        std::cout << "There cannot be less than 0 patients." << std::endl;
    }
    return 0;
}
