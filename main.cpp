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
#include <sstream>

using namespace std;

struct pcb {
    int pid;    //process ID code
    int tarq;   //time of arrival into the ready queue
    int prio;   //priority code
    int tncpu;  //total number of cpu bursts of the process
    int * cpuburst;
    int * ioburst;
    int wait;   //total wait time
    int turnaround; //total turnaround time
};

struct node {
  struct pcb;
  struct node *next;
};


void readFile() {
    ifstream infile;
    char fileToOpen[80];
    char buffer[80];
    char line[20];
    char * pch;


    printf("Where is the file?");
    scanf("%s", fileToOpen);
    infile.open(fileToOpen, ifstream::in);
    while (infile.is_open()) {
        pcb * pcb;
        int idx1=0;
        int idx2=0;
        infile.getline(buffer, 80);
        istringstream buffer(line);
        int value;
        //strtok and move data to pcb
        for (int i = 0; i < 4; i++) {
            buffer >> value;
            switch (i) {
                case 1:
                    pcb->pid = value;
                    break;
                case 2:
                    pcb->tarq = value;
                    break;
                case 3:
                    pcb->prio = value;
                    break;
                case 4:
                    pcb->tncpu = value;
                    pcb->cpuburst = new int [value];
                    pcb->ioburst = new int [value-1];
                    break;
                default:
                    printf("Impossible to end up here");
                    break;
            }
        }
        for (int k = 0; k < (pcb->tncpu)*2-1; k++) {
            if (k%2==0)
                buffer >> pcb->cpuburst[idx1++];
            else
                buffer >> pcb->ioburst[idx2++];
        }
    }
}


/*
 * 
 */
int main(int argc, char** argv) {

    readFile();
    return 0;
}

