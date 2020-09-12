		/**
* Author: Emily Bodenhamer
* Date: 5/20/20
* Lab 3 CS 470
**/
#include <iostream>
#include <time.h>
#include <fstream>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
using namespace std;

// global variables
bool endLoop = false;
typedef struct _args {
    int n; // num of elements in file
    double *elements;
    int **pairs; // set to -1
    int threads;
} args;
pthread_mutex_t myMutex;
pthread_mutex_t MutexMaster;
args *myArgs = (args *) calloc(1, sizeof(args));

// methods
/**
* Method that will free memory from matrices
**/
void freeMem(int row, int **matrix) {
    for (int i = 0; i < row; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

/**
* Method that will create memory for an array
**/
int *singleArray(int n) {
    int *arr = (int *) calloc(n, sizeof(int));

    // check if null
    if (arr == NULL) {
        printf("Memory allocation error.");
        exit(0);
    }
    return arr;
}

/**
* Method that will initialize a matrix
**/
void initPairs() {
    // create the memory
    myArgs->pairs = (int **) calloc(myArgs->threads, sizeof(int *));
    for (int i = 0; i < myArgs->threads; i++) {
        myArgs->pairs[i] = singleArray(2);
    }

    // fill in with -1
    for (int i = 0; i < myArgs->threads; i++) {
        for (int j = 0; j < 2; j++) {
            myArgs->pairs[i][j] = -1;
        }
    }

    // check if NULL
    if (myArgs->pairs == NULL) {
        printf("Memory allocation error.");
        exit(0);
    }
}

// bubble swap
void swap(double *xp, double *yp) {
    double temp = *xp;
    *xp = *yp;
    *yp = temp;
}

// bubble sort from geeks for geeks
void bubble(double *array, int n) {
    int i, j;
    for (i = 0; i < n - 1; i++)
        // Last i elements are already in place
        for (j = 0; j < n - i - 1; j++)
            if (array[j] > array[j + 1])
                swap(&array[j], &array[j + 1]);
}

// insertion sort
// http://www.algolist.net/Algorithms/Sorting/Insertion_sort
void insert(double *array, int n) {
    int i, j;
    double tmp;

    for (i = 1; i < n; i++) {
        j = i;
        while (j > 0 && array[j - 1] > array[j]) {
            tmp = array[j];
            array[j] = array[j - 1];
            array[j - 1] = tmp;
            j--;
        }
    }
}

// for qsort
static int compare(const void *a, const void *b) {
    if (*(double *) a > *(double *) b) return 1;
    else if (*(double *) a < *(double *) b) return -1;
    else return 0;
}

// pick sorting algorithms
void pickSort(double *array, int n, int pThread) {
    int ran = rand() % 3 + 1;
    int i;
    switch (ran) {
        case 1:
            // quick sort
            cout << "thread [" << pThread <<"] using qsort: ";
            qsort(array, n, sizeof(array[0]), compare);
            for (i = 0; i < n; i++)
                printf("%lf ", array[i]);
            cout << endl;
            break;
        case 2:
            // insert sort
            cout << "thread [" << pThread <<"] using insert sort: ";
            insert(array, n);
            for (i = 0; i < n; i++)
                printf("%lf ", array[i]);
            cout << endl;
            break;
        case 3:
            // bubble sort
            cout << "thread [" << pThread <<"] using bubble: ";
            bubble(array, n);
            for (i = 0; i < n; i++)
                printf("%lf ", array[i]);
            cout << endl;
            break;
    }
}

/**
* Method that will initialize i and j pairs
**/
void initIndices(int &i, int &j, args *myArgs) {
    // generate indices
    i = rand() % (myArgs->n);
    j = rand() % (myArgs->n);
    while (i == j) {
        i = rand() % (myArgs->n);
        j = rand() % (myArgs->n);
    }

    // if i is greater than j then swap 
    if (i > j) {
        int temp = i;
        i = j;
        j = temp;
    }
}

/**
* Method that will replace in the file the elements
* from the sorted subarray
**/
void replaceSort(int i, int j, args *myArgs, double* subArray, int pThread) {
    int temp = i, index = 0;
    FILE *myFile = fopen("sortFile.txt", "w");
    pthread_mutex_lock(&MutexMaster);
    cout << "thread [" << pThread <<"] The elements within the file now looks like this: " ;
    for (int k = 0; k < myArgs->n; ++k) {
        if (k == temp){
            myArgs->elements[k] = subArray[index];
            temp++;
            index++;
        }
        if (k == j){
            temp = -1;
        }
        fprintf(myFile, "%lf\t", myArgs->elements[k]);
        cout << myArgs->elements[k] << " ";
    }
    cout << "\n";
    fclose(myFile);
    pthread_mutex_unlock(&MutexMaster);
}

/**
* Method that will fill in the subarray
**/
void fillInSub(int i, int j, double * subArray) {
    string line;
    int index = 0, k = 0, temp = i;
    ifstream myFile("sortFile.txt");
    // open the file
    if (myFile.is_open()) {
        while (myFile >> line) {
            if (index == temp) {
                // input elements to subarray
                subArray[k] = stod(line);
                cout << line << " ";
                k++;
                temp++;
            }
            // if the pointer is out of bounds exit
            if (index == j)
                break;
            index++;
        }
        cout << "\n";
        // close the file
        myFile.close();
    } else {
        cout << "Unable to open file";
    }
}

/**
* Method that will check if there are any i and j pairs that
* are overlapping within the pairs matrix
**/
void checkConflicts(args *myArgs, int pThread, int i, int j) {
    // struct for nanosleep half a second
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = 5000L;

    bool conflict = true;
    // check i and j pairs for conflicts
    while (conflict == true) {
        conflict = false;
        // lock critical section
        pthread_mutex_lock(&myMutex);
        // check if any i and j conflict
        for (int k = 0; k < myArgs->threads; k++) {
            // if theres an i and j that conflicts then you unlock and sleep.
            if (!(i < myArgs->pairs[k][1] && j < myArgs->pairs[k][0]) && k != pThread && myArgs->pairs[k][1] != -1) {
                conflict = true;
                pthread_mutex_unlock(&myMutex);
                break;
            }
        }
        nanosleep(&tim, &tim2);
        // else if there is no conflict: break out of loop
    }
    // input the current threads i and j
    myArgs->pairs[pThread][0] = i;
    myArgs->pairs[pThread][1] = j;
    // unlock mutex
    pthread_mutex_unlock(&myMutex);
}

/**
* Thread method that will start the
* sorting program
**/
void *getIndex(void * pThread) {
    // change myStuct to args *
    int  pIndex = (int)(long)pThread;
    double *subArray;
    int i , j;

    // keep running until i and j is available
    // programrunning = true at start of main, false right before loop for thread joins
    while (endLoop  == false) {
        // generate indices
        initIndices(i, j, myArgs);

        // check if there are any conflicting i and j
       checkConflicts(myArgs, pIndex, i, j);

        // create sub array for thread
        int length = 1 + (j - i);
        subArray = (double *) calloc(length, sizeof(double));

        // lock file to fill in subarray
        pthread_mutex_lock(&myMutex);
        printf("\nthread [%i] i is %i, j is %i, The elements within these pairs are: ", pIndex, i, j);
        fillInSub(i, j, subArray);
        pthread_mutex_unlock(&myMutex);

        // then use sorting algorithm
        pickSort(subArray, length, pIndex);

        // lock to write to file
        pthread_mutex_lock(&myMutex);
        replaceSort(i,j, myArgs,subArray,pIndex);

        // fill in pair array with -1 for cur thread
        myArgs->pairs[pIndex][0] = -1;
        myArgs->pairs[pIndex][1] = -1;
        pthread_mutex_unlock(&myMutex);
        cout << "\n";
        free(subArray);
    }
    return nullptr;
}

/**
* Master Thread method that will
* loop until the file is sorted
**/
void *sorted(void *myStruct) {
    // struct for nanosleep half a second
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = 50000L;
    args *myArgs = (args *) myStruct;

    // make master thread array
    bool endAll = false;
    double *master = (double *) calloc(myArgs->n, sizeof(double));

    // keep iterating until the array is sorted
    while (endAll == false) {
        pthread_mutex_lock(&MutexMaster);
        int j = 0;
        // check if the file has been updated
        memcpy(master, myArgs->elements, myArgs->n* sizeof (double));
        pthread_mutex_unlock(&MutexMaster);

        for (int i = 0; i < myArgs->n - 1; i++) {
            j = i + 1;
            if (master[i] > master[j]) {
                endAll = true;
            }
        }
        // loop end criteria
        if (endAll == true){
            endAll = false;
        } else {
            endLoop = true;
            break;
        }
        //sleep
        nanosleep(&tim,&tim2);
    }
    cout << "\nMaster Thread is done!\n";
    free(master);
    return nullptr;
}

/**
* Generate the random 
* elements in the file
**/
void genRanFile(args *myArgs) {
    // create array and variables
    myArgs->elements = (double *) calloc(myArgs->n, sizeof(double));
    float max = 10.0;
    double input;
    // create a sort file
    FILE *myFile = fopen("sortFile.txt", "w+");
    cout << "The elements in your file unsorted are: ";
    for (int i = 0; i < myArgs->n; ++i) {
        // create double
        input = ((double) rand() / (double) (RAND_MAX)) * max;
        // save elements to array
        myArgs->elements[i] = input;
        // save double to file
        fprintf(myFile, "%lf\t", input);
        printf("%lf ", input);
    }
    cout << "\n";
    fclose(myFile);
}

/**
* Check if the number is positive
**/
bool isNum(string digit, int length) {
    // check if a positive int
    for (int i = 0; i < length; i++)
        if (isdigit(digit[i]) == false)
            return false;
    return true;
}

/**
* Check if the number is a valid input
**/
int validNum(const string &num) {
    try {
        // determine if input is a pos int
        if (isNum(num, num.length())) {
            int anInt = stoi(num);
            return anInt;
        } else {
            cout << "You must enter a positive integer.\n";
            exit(-1);
        }
    } catch (const exception e) {
        cout << "Invalid number. Exiting..\n";
        exit(-1);
    }
}

/**
* Main method that will perform
* the threading program
**/
int main(int argc, char *argv[]) {
    // initialize random seed
    srand(time(NULL));

    // get num of elements from cmd line
    if (argc > 1) {
        // check if valid num of elements
        myArgs->n = validNum(argv[1]);
    } else {
        cout << "You must enter how many elements you would like.\n";
        return -1;
    }

    // get num threads
    string line;
    printf("Input how many threads you would like: ");
    getline(std::cin, line);
    myArgs->threads = validNum(line);
    if(myArgs->threads > 1000){
        printf("Too many threads, input a smaller amount.\n");
        exit(-1);
    }

    // create file
    genRanFile(myArgs);

    // create an array of pairs for sorting only i and j
    initPairs();

    // create threads
    pthread_t master;
    pthread_t sortThreads[myArgs->threads];

    // myMutex initialization
    pthread_mutex_init(&myMutex, NULL);
    pthread_mutex_init(&MutexMaster, NULL);

    // master thread keeps checking until the threads are done.
    int masterSuccess = pthread_create(&master, NULL, sorted, (void *) (args *) (myArgs)); // i will be thread id
    
    // thread creation and run
    int success;
    for (int i = 0; i < myArgs->threads; i++) {
        // put n and threads id aka i into the struct.
        success = pthread_create(&sortThreads[i], NULL, getIndex,
                                 (void *) (long) i); // i will be thread id
    }

    //  join the threads to make sure all of them will be executed
    for (int i = 0; i < myArgs->threads; i++) {
        pthread_join(sortThreads[i], NULL);
    }
    pthread_join(master, NULL);

    cout << "Your file has been sorted! You can look at sortFile.txt to see your sorted numbers.\n";

    // free memory
    free(myArgs->elements);
    free(myArgs);
    freeMem(2,myArgs->pairs);
    return 0;
}