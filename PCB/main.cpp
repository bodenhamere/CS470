/**
* Author: Emily Bodenhamer
* Date: 5/28/20
* Lab 4 CS 470
**/
#include <iostream>
#include <vector>
#include <unistd.h>
#include <algorithm>
using namespace std;

// fields
typedef struct _processor {
    char *name;
    int32_t id;
    unsigned char status;
    int32_t burst;
    int32_t base_reg;
    long long limit_reg;
    unsigned char priority;
} processor;
vector<processor> pcgs;
vector<processor> temp;

// method headers
void readFile(char *argv);
void write(int i, vector<processor> a, int index);
void roundRobin();
void priority();
void sort();
void aging(int j);
int findLoc(int j);

// methods
int main(int argc, char *argv[]) {
    // file handling
    if (argc > 1) {
        cout << "Opening " << argv[1] << endl;
    } else {
        cout << "No file name entered. Exiting.\n";
        return -1;
    }
    bool finish = true;
    while (finish) {
        // read file
        readFile(argv[1]);
        // finished executing all processes
        for (int i = 0; i < pcgs.size(); i++) {
            if (pcgs[i].burst != 0) {
                finish = false;
            }
        }

        // check if cpu bursts are all 0
        if (!finish) {
            finish = true;
        } else {
            break;
        }

        // perform round robin
        roundRobin();

        // copy pcg to temp to sort
        temp = pcgs;
        sort();

        // perform priority scheduling
        priority();

        // clear the vectors
        pcgs.clear();
        temp.clear();
    }
    return 0;
}

void readFile(char *argv) {
    // open file
    FILE *file;
    file = fopen(argv, "rb");  // r for read, b for binary
    // init variables
    char buffer[256];
    int process = 0, i = 0, mem = 0;
    // file handling
    if(file == NULL){
        cout << "Failed to open file. Exiting.\n";
        exit(-1);
    }

    // get file size
    fseek(file, 0L, SEEK_END);
    int size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    // read file
    while (ftell(file) != size) {
        // vector of structs
        pcgs.push_back(processor());
        // read file & save
        fread(buffer, 16, 1, file);
        pcgs[i].name = buffer;
        printf("process name: %s\n", pcgs[i].name);

        fread(&pcgs[i].id, sizeof(pcgs[i].id), 1, file);
        printf("process id: %i\n", pcgs[i].id);

        fread(&pcgs[i].status, sizeof(pcgs[i].status), 1, file);
        printf("activity status: %d\n", pcgs[i].status);

        fread(&pcgs[i].burst, sizeof(pcgs[i].burst), 1, file);
        printf("CPU burst time: %i\n", pcgs[i].burst);

        fread(&pcgs[i].base_reg, sizeof(pcgs[i].base_reg), 1, file);
        printf("base register: %i\n", pcgs[i].base_reg);

        fread(&pcgs[i].limit_reg, sizeof(pcgs[i].limit_reg), 1, file);
        printf("limit register: %llu\n", pcgs[i].limit_reg);

        fread(&pcgs[i].priority, sizeof(pcgs[i].priority), 1, file);
        printf("priority: %d\n", pcgs[i].priority);
        printf("\n");
        process++;
        mem += pcgs[i].limit_reg - pcgs[i].base_reg;
        i++;
    }
    printf("number of processes in the file: %i\n", process);
    printf("total number of memory allocated by the processes: %i\n", mem);
    printf("\n");
    fclose(file);
}

void write(int i, vector<processor> a, int index) {
    FILE *fp;
    fp = fopen("processes.bin", "rb+");

    // update activity
    fseek(fp, 38 * i + 20, SEEK_CUR);
    fwrite(&a[index].status, sizeof(a[index].status), 1, fp);

    // update burst
    fwrite(&a[index].burst, sizeof(a[index].burst), 1, fp);
    rewind(fp);

    // update priority
    fseek(fp, 38 * i + 37, SEEK_CUR);
    fwrite(&a[index].priority, sizeof(a[index].priority), 1, fp);
    fclose(fp);
}

void roundRobin() {
    int quantum = 0, counter = 0;
    for (int i = 0; i < pcgs.size(); i++) {
        // decrease the burst
        if (pcgs[i].burst != 0) {
            pcgs[i].burst--;
            // write changes to the file
            write(i, pcgs, i);
            printf("Round Robin on process [%i] id: %d CPU "
                   "burst is now: %d\n", i, pcgs[i].id, pcgs[i].burst);
            // execute
            quantum++;
//            sleep(1);
        } else {
            // skip and write to file
            counter++;
            pcgs[i].status = 0;
            write(i, pcgs, i);
        }
        // exit algorithm
        if (quantum == 30 || counter == pcgs.size())
            break;
        // go back to the beginning
        if (i == (pcgs.size() - 1)) {
            i = -1;
            counter = 0;
        }
    }
}

void sort() {
    // insert sort
    processor temp1;
    for (int i = 0; i < temp.size() - 1; i++) {
        for (int j = 0; j < temp.size() - 1 - i; j++) {
            if (temp[j].priority > temp[j + 1].priority) {
                temp1 = temp[j];
                temp[j] = temp[j + 1];
                temp[j + 1] = temp1;
            }
        }
    }
}

int findLoc(int j) {
    // find the location in the original vector
    for (int i = 0; i < pcgs.size(); i++) {
        if (temp[j].id == pcgs[i].id) {
            return i;
        }
    }
}

void priority() {
    int quantum = 0, counter = 0, j;
    for (int i = 0; i < temp.size(); i++) {
        // decrease the burst
        if (temp[i].burst != 0) {
            temp[i].burst--;
            // find location in og
            if (temp[i].burst == 0)
                temp[i].status = 0;
            // write to file
            j = findLoc(i);
            write(j, temp, i);
            printf("Priority Scheduling on process [%i] id: %d CPU "
                   "burst is now: %d\n", j, temp[i].id, temp[i].burst);
            // stay on the current process
            quantum++;
            i = i - 1;
            counter = 0;
//            sleep(1);
        } else {
            // skip and write to file
            counter++;
            temp[i].status = 0;
            j = findLoc(i);
            write(j, temp, i);
        }
        // exit algorithm
        if (quantum == 30 || counter == pcgs.size()) {
            break;
        }
        // aging
        if (quantum % 2 == 0 && quantum != 0) {
            aging((i + 1));
        }
    }
}

void aging(int j) {
    int k;
    for (int i = 0; i < temp.size(); i++) {
        // decrease all priority except for current process
        if (temp[i].priority > 0 && j != i && temp[i].status != 0) {
            temp[i].priority--;
            // write to file
            k = findLoc(i);
            write(k, temp, i);
            printf("Aging on process id: %d CPU "
                   "priority is now: %d\n", temp[i].id, temp[i].priority);
        }
    }
    sort();
}