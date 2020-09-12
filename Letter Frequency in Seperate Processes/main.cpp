/*
 * Author: Emily Bodenhamer
 * Date: 5/1/2020
 * Lab 2
 * */

// imports
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

void findFrequency(string line, char alpha [], int i){
    int frequency, fd[2];

    // pipe error
    if (pipe(fd) == -1) {
        printf("Error creating pipe.\n");
        exit(1);
    }

    pid_t pid = fork();
    // fork error
    if (pid < 0) {
        perror("couldn't fork\n");
        exit(1);
    }
    // child process
    else if (pid == 0) {

        for (int j = 0; j < line.length(); ++j) {
            if (alpha[i] == tolower(line[j])){
                frequency++;
            }
        }
        // close reading and write to the pipe
        close(fd[0]);
        write(fd[1], &frequency, sizeof(int));
        // close writing and exit the child
        close(fd[1]);
        _exit(0);
    }
    // parent process
    else {
        // wait for child, close writing and read from the pipe
        wait(NULL);
        close(fd[1]);
        read(fd[0], &frequency, sizeof(int));

        // print the output for each letter and close the reading pipe
        printf("%c: %i\n", alpha[i], frequency);
        close(fd[1]);
    }
}

string readFile(char *argv) {
    ifstream infile(argv); //open the file
    string line, alpha;
    // read from the file and save it to alpha
    if (infile.is_open() && infile.good()) {
        cout << "Your file contains:\n";
        while (getline(infile, line)) {
            alpha += line;
        }
    // error no file
    } else {
        cout << "Failed to open file. Exiting.\n";
        exit(-1);
    }
    // close the file & return string
    infile.close();
    return alpha;
}

int main(int argc, char *argv[]) {

    // file handling
    if (argc > 1) {
        cout << "Opening " << argv[1] << endl;
    } else {
        cout << "No file name entered. Exiting.\n";
        return -1;
    }

    // read file
    string line = readFile(argv[1]);

    // create char array of alphabet
    char alpha [] = {'a', 'b', 'c', 'd', 'e', 'f', 'g',
                     'h', 'i', 'j', 'k', 'l', 'm', 'n',
                     'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                     'w', 'x', 'y', 'z'};

    // find frequency of all letters
    for (int i = 0; i < sizeof(alpha); ++i) {
        findFrequency(line, alpha, i);
    }
    return 0;
}
