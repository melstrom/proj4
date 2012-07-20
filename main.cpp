/* 
 * File:   main.cpp
 * Author: melstrom
 *
 * Created on July 17, 2012, 7:42 PM
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>

using namespace std;

typedef struct {
    int pid;    //process ID code
    int tarq;   //time of arrival into the ready queue
    int prio;   //priority code
    int tncpu;  //total number of cpu bursts of the process
    vector<int> bursttimes;
    int wait;   //total wait time
    int turnaround; //total turnaround time
} pcb;

struct node {
  struct pcb;
  struct node *next;
};


void readFile(pcb* pcb) {
    ifstream infile;
    char fileToOpen[80];
    char buffer[80];
    char line[20];
    char * pch;


    printf("Where is the file?");
    scanf("%s", fileToOpen);
    infile.open(fileToOpen, ifstream::in);
    while (infile.is_open()) {
        infile.getline(buffer, line);
        //strtok and move data to pcb
        for (int i = 0; i < 4; i++) {
            pch = strtok(buffer, " \t");
            switch (i) {
                case 1:
                    pcb->pid = pch;
                    break;
                case 2:
                    pcb->tarq = pch;
                    break;
                case 3:
                    pcb->prio = pch;
                    break;
                case 4:
                    pcb->tncpu = pch;
                    break;
                default:
                    printf("Impossible to end up here");
                    break;
            }
        }
        for (int k = 0; k < (pcb->tncpu)*2-1; k++) {
              pch = strtok(buffer, " \t");
              pcb->bursttimes.insert(k, pch);
        }
    }
}


/*
 * 
 */
int main(int argc, char** argv) {

    return 0;
}

