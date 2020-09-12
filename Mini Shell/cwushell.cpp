/*
 * Author: Emily Bodenhamer
 * Date: 4/22/2020
 * Lab 1
 * */

// imports
#include <iostream>
#include <cstring>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <algorithm>
#include <stdexcept>

// global variables
std::string promptName = "cwushell>"; // prompt name

std::string prevCmd; // var to hold the previous commands exit val

std::string cpuCmd[3] = { // hold cpuinfo switches
        "-c",
        "-t",
        "-n",
};

std::string memCmd[3] = { // hold meminfo switches
        "-t",
        "-u",
        "-c",
};

std::string newCmd[5] = { // hold custom commands
        "exit",
        "prompt",
        "cpuinfo",
        "meminfo",
        "man",
};

// functions
/*
 * isNum
 * This method takes the string digit and
 * determines if the string is an integer
 * or not.
 * Returns a bool val.
 */
bool isNum(std::string digit, int length) {
    // check if num is negative
    if (digit[0] == '-') {
        digit = digit.substr(1, length - 1);
        length = digit.length();
    }
    // check if pos int
    for (int i = 0; i < length; i++)
        if (isdigit(digit[i]) == false)
            return false;

    return true;
}

/*
 * help
 * This method outputs the help/man info
 * to the user depending on the specified
 * command.
 * Returns an int.
 */
int help(int typeCmd) {
    switch (typeCmd) {
        case 1:
            printf("NAME \n "
                   "\t exit - terminates the shell\n"
                   "SYNOPSIS\n"
                   "\t exit [n]\n"
                   "DESCRIPTION\n"
                   "\t Terminates the shell, either by calling the exit() "
                   "standard library routine. \n \t If an argument (n) is given, "
                   "it should be the exit \n \t value of the shell's execution. "
                   "Otherwise, the exit value should be the value returned "
                   "\n \t by the last executed command (or 0 if no commands were executed.)\n"
                   "\t Arguments permitted are any integer values for n "
                   "\n \t -h, --help \n \t \t can be used to look up help for this command.\n"
            );
            return 1;
        case 2:
            printf("NAME \n "
                   "\t prompt - changes the current shell prompt\n"
                   "SYNOPSIS\n"
                   "\t prompt <new_prompt>\n"
                   "DESCRIPTION\n"
                   "\t This command will change the current shell prompt to the new_prompt. \n \t "
                   "The default prompt is cwushell>. Typing prompt will restore the default"
                   " shell prompt.\n"
                   "\t Arguments permitted are any string values for new_prompt \n"
                   "\t -h, --help \n \t \tcan be used to look up help for this command.\n"
            );
            return 1;
        case 3:
            printf("NAME \n "
                   "\t cpuinfo - prints different cpu related information to the terminal. \n"
                   "SYNOPSIS\n"
                   "\t cpuinfo -switch\n"
                   "DESCRIPTION\n"
                   "\t Print on the screen different cpu related information based on the switch. \n"
                   "\t Arguments permitted are any string values for cpuinfo\n "
                   "\t -c \n \t \t will print the cpu clock (e.g. 3.2 GHz).\n"
                   "\t -t \n \t \t will print the cpu type (e.g. Intel).\n"
                   "\t -n \n \t \t will print the number of cores (e.g. 8).\n"
            );
            return 1;
        case 4:
            printf("NAME \n "
                   "\t meminfo - prints different memory related information to the terminal. \n"
                   "SYNOPSIS\n"
                   "\t meminfo -switch\n"
                   "DESCRIPTION\n"
                   "\t Print on the screen different memory related information based on the switch. \n"
                   "\t Arguments permitted are any string values for meminfo\n "
                   "\t -t \n \t \t will print the total RAM memory available in the system in bytes.\n"
                   "\t -u \n \t \t will print the used RAM memory.\n"
                   "\t -n \n \t \t will print the size of the L2 cache/core in bytes.\n"
            );
            return 1;
        case 5:
            printf("What manual page do you want? Please specify a command for more information!\n"
                   "\t You can use the following commands:\n"
                   "\t prompt or prompt <new_prompt> or prompt -h --help\n"
                   "\t exit or exit [n]or exit -h --help \n"
                   "\t cpuinfo or cpuinfo -t or cpuinfo -c or cpuinfo -n or cpuinfo  -h --help\n"
                   "\t meminfo or meminfo -t or meminfo -u or meminfo -c or meminfo  -h --help\n"
                   "\t or any of the Linux commands! \n");
            return 1;
        default:
            printf("Wrong input please try again. Check using the man command for help.\n");
            return 1;
    }
}

/*
 * prompt
 * This method takes the string parsed and
 * changes the prompt to the specified prompt,
 * if no prompt was given, then the default
 * prompt is returned. Help can also be called
 * for this command.
 * Returns an int.
 */
int prompt(std::string *parsed, int typeCmd) {
    if (parsed[0] == "prompt" && parsed[1] != "") {
        // output help
        if (parsed[1] == "-h" || parsed[1] == "--help") {
            return help(typeCmd);
        }
        // change prompt name
        promptName = parsed[1];
        // set to default
    } else if (parsed[0] == "prompt" && parsed[1] == "") {
        promptName = "cwushell>";
    }
    return 1;
}

/*
 * getExitVal
 * This method takes the string sysCmd and
 * a pipe is created, forking, and invoking the shell
 * is done to perform non-custom commands. The exit value
 * is obtained from these commands and saved for later use.
 * Returns a string that holds the exit value.
 */
std::string getExitVal(std::string sysCmd)  {
    char buffer[128];
    std::string result = "";

    // open a pipe
    FILE* pipe = popen(sysCmd.c_str(), "r");
    if (!pipe) {
        return "error";
    }

    // save the exit val
    while (!feof(pipe)) {

        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }

    pclose(pipe);
    return result;
}

/*
 * cmdExit
 * This method takes the string parsed
 * and exits the shell. If a parameter was given,
 * it is used as the exit value. If no parameter was given,
 * prevCmd is used as the exit value, if no commands had
 * exit values, then 0 is used. Help can also be called
 * for this command.
 * Returns an int.
 */
int cmdExit(std::string *parsed, int typeCmd) {
    if (parsed[1] != "") {
        // output help
        if (parsed[1] == "-h" || parsed[1] == "--help") {
            return help(typeCmd);
        }
        int length = parsed[1].length();

        // find if the parameter was was digit
        if (isNum(parsed[1], length)) {
            printf("exiting..\n");
            exit(stoi(parsed[1]));
        } else {
            printf("There was an error with your command. Check using the man command for help\n");
        }

    } else {
        // if not parameter was give, find cmd exit val
        if(prevCmd != ""){
            printf("exiting..\n");
            exit(stoi(prevCmd));
        } else {
            printf("exiting..\n");
            exit(0);
        }
    }
    return 1;
}

/*
 * cmdList
 * This method takes the string parsed
 * and finds the command that matches the
 * string.
 * Returns an int.
 */
int cmdList(std::string parsed, std::string *newCmd) {
    int typeCmd = 0;

    if (parsed == "") {
        return -1;
    }

    // calculate which command to process
    for (int i = 0; i <= newCmd->length(); i++) {
        if (newCmd[i].compare(parsed) == 0) {
            typeCmd = i + 1;
            break;
        }
    }

    return typeCmd;
}

/*
 * performCpu
 * This method updates the cpuinfos pointer
 * by opening the /proc/cpuinfo path and
 * adding the specified switch commands to the
 * pointer indexes.
 */
void performCpu(std::string *cpuinfos) {
    FILE *f = fopen("/proc/cpuinfo", "r");
    size_t n = 0;
    char *line = NULL;

    if (f == NULL) {
        fprintf(stderr, "Error opening file\n");
    } else {
        while (getline(&line, &n, f) > 0) {
            // output cpu clock
            if (strstr(line, "cpu MHz")) {
                cpuinfos[0] = line;
            // output cpu brand
            } else if (strstr(line, "model name")) {
                cpuinfos[1] = line;
            // output cpu cores
            } else if (strstr(line, "cpu cores")) {
                cpuinfos[2] = line;
            }
        }
    }
    free(line);
    fclose(f);
}

/*
 * cpuInfo
 * This method takes the string parsed
 * pointer and checks which switch was chosen
 * to execute.
 * Returns an int.
 */
int cpuInfo(std::string *parsed, int typeCmd) {
    if (parsed[1] != "") { // output help
        if (parsed[1] == "-h" || parsed[1] == "--help") {
            return help(typeCmd);
        }

        // find all cpuinfo
        typeCmd = cmdList(parsed[1], cpuCmd);
        std::string *cpuinfos = (std::string *) calloc(3, sizeof(std::string));
        performCpu(cpuinfos);

        // output only the specified data
        switch (typeCmd) {
            case 1:
                std::cout << cpuinfos[0];
                free(cpuinfos);
                return 1;
            case 2:
                std::cout << cpuinfos[1];
                free(cpuinfos);
                return 1;
            case 3:
                std::cout << cpuinfos[2];
                free(cpuinfos);
                return 1;
            default:
                printf("This command was invalid, Check using the man command for help\n");
                return 1;
        }
    } else {
        printf("This command was called without a parameter, here is the help for cpuinfo:");
        return help(typeCmd);
    }
}

/*
 * performMem
 * This method updates the meminfos pointer
 * by opening the /proc/meminfos and
 * \sys/devices/system/cpu/cpu0/cache/index2/size
 * path and adding the specified switch commands to the
 * pointer indexes.
 */
void performMem(std::string *meminfos) {
    FILE *f = fopen("/sys/devices/system/cpu/cpu0/cache/index2/size", "r");
    FILE *file = fopen("/proc/meminfo", "r");

    size_t n = 0;
    char *line = NULL;
    long toBytes;
    long used ;
    std::string temp = "";

    if (f == NULL || file == NULL) {
        fprintf(stderr, "Error opening file\n");
    } else {
        // find L2 cache, convert Kb to Bytes
        getline(&line, &n, f);
        temp += line;
        temp.erase(std::remove_if(temp.begin(), temp.end(), [](char c) { return isalpha(c); } ), temp.end());
        toBytes = stoi(temp) * 1000;
        meminfos[2] = std::to_string(toBytes).append(" Bytes of L2 cache/core.");
        temp = "";

        while (getline(&line, &n, file) > 0) {
            // find total RAM memory, convert Kb to Bytes
            if (strstr(line, "MemTotal")) {
                temp += line;
                temp.erase(std::remove_if(temp.begin(), temp.end(), [](char c) { return !isdigit(c); } ), temp.end());
                toBytes = stoi(temp) * 1000;
                temp = "";
            }
            // find used RAM memory, convert Kb to Bytes
            if (strstr(line, "MemFree")) {
                temp += line;
                temp.erase(std::remove_if(temp.begin(), temp.end(), [](char c) { return !isdigit(c); } ), temp.end());
                used = toBytes - (stoi(temp)*1000);
                meminfos[0] = std::to_string(toBytes).append(" Bytes of total RAM memory available.");
                meminfos[1] = std::to_string(used).append(" Bytes of used RAM memory.");
            }
        }
    }
    free(line);
    fclose(f);
    fclose(file);
}

/*
 * memInfo
 * This method takes the string parsed
 * pointer and checks which switch was chosen
 * to execute.
 * Returns an int.
 */
int memInfo(std::string *parsed, int typeCmd) {
    if (parsed[1] != "") { // output help
        if (parsed[1] == "-h" || parsed[1] == "--help") {
            return help(typeCmd);
        }

        // find all switch data
        typeCmd = cmdList(parsed[1], memCmd);
        std::string *meminfos = (std::string *) calloc(3, sizeof(std::string));
        performMem(meminfos);

        // output only the specified data
        switch (typeCmd) {
            case 1:
                std::cout << meminfos[0] << std::endl;
                free(meminfos);
                return 1;
            case 2:
                std::cout << meminfos[1] << std::endl;
                free(meminfos);
                return 1;
            case 3:
                std::cout << meminfos[2] << std::endl;
                free(meminfos);
                return 1;
            default:
                printf("This command was invalid, Check using the man command for help\n");
                return 1;
        }
    } else {
        printf("This command was called without a parameter, here is the help for cpuinfo:");
        return help(typeCmd);
    }
}

/*
 * man
 * This method takes the string parsed
 * pointer and checks which help is needed.
 * Returns an int.
 */
int man(std::string *parsed, int typeCmd) {
    if (parsed[1] == "") {
        return help(typeCmd);
    } else {
        int cmd = cmdList(parsed[1], newCmd);
        return help(cmd);
    }
}

/*
 * commands
 * This method takes the string parsed
 * pointer and checks which command was
 * called via a switch.
 * Returns an int.
 */
int commands(int typeCmd, std::string *parsed) {

    // find the specified command
    switch (typeCmd) {
        case 1:
            return cmdExit(parsed, typeCmd);
        case 2:
            return prompt(parsed, typeCmd);
        case 3:
            return cpuInfo(parsed, typeCmd);
        case 4:
            return memInfo(parsed, typeCmd);
        case 5:
            return man(parsed, typeCmd);
        default:
            return -1;
    }
}

/*
 * parseCmd
 * This method takes the string line
 * and parses each word into the parsed
 * pointer
 */
void parseCmd(const std::string line, std::string *parsed, int buffer) {
    std::stringstream stream(line);
    std::string cmds;
    int index = 0;
    // parse input
    while (stream >> cmds && index != buffer) {
        parsed[index] = cmds;
        index++;
    }
}

/*
 * loop
 * This method is the shell.
 *
 * The method ends when the flag != 1.
 */
void loop(void) {
    std::string line;
    int buffer = 250;
    int flag = 1;
    printf("Welcome to this shell! This program was intended for Linux.\n Please type 'man' to find what commands you can use!\n");

    while (flag == 1) {
        std::string *parsed = (std::string *) calloc(buffer, sizeof(std::string));
        // get user input
        printf("%s", promptName.c_str());
        getline(std::cin, line);
        printf("\n");

        // parse the input
        parseCmd(line, parsed, buffer);

        // find which command to perform
        int typeCmd = cmdList(parsed[0], newCmd);
        // perform the type of command
        flag = commands(typeCmd, parsed);

        // if there was no custom command, use a Linux one
        if (flag == -1){
            // save the exit val
            prevCmd = getExitVal(line.append(";echo $?"));
            if (stoi(prevCmd) != 0){
                printf("There was an error in your cmd, please open man for help\n");
            }
            flag = 1;
        } else {
            prevCmd = "";
        }
        // free pointer
        free(parsed);
    }

}

/*
 * main
 * Start the shell.
 */
int main(void) {
    loop();
    return 0;
}