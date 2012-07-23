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
    int pid; //process ID code
    int tarq; //time of arrival into the ready queue
    int prio; //priority code
    int tncpu; //total number of cpu bursts of the process
    int * cpuburst; //CPU burst times
    int * ioburst; //IO burst times
    int wait; //total wait time
    int turnaround; //total turnaround time
    int readytime; //next ready time
};

struct qNode {
    struct pcb *pcb;
    qNode *link;
};

class lqueue {
private:
    qNode *front, *rear;

public:

    lqueue() {
        front = NULL;
        rear = NULL;
    }

    void add(pcb *n) {
        qNode *tmp;
        tmp = new qNode;
        if (tmp == NULL)
            cout << "\nQUEUE FULL";

        tmp->pcb = n;
        tmp->link = NULL;
        if (front == NULL) {
            rear = front = tmp;
            return;
        }
        rear->link = tmp;
        rear = rear->link;
    }

    pcb * del() {
        if (front == NULL) {
            return NULL;
        }
        qNode *tmp;
        struct pcb *n;
        n = front->pcb;
        tmp = front;
        front = front->link;
        delete tmp;
        return n;
    }

    ~lqueue() {
        if (front == NULL)
            return;
        qNode *tmp;
        while (front != NULL) {
            tmp = front;
            front = front->link;
            delete tmp;
        }
    }
};

struct node {
    struct pcb *pcb;
    struct node *next;
};


int MAXtarq = 0;

void readFile(struct node * root) {
    struct node * conductor = root;
    string line;

    //    ifstream infile ("/Users/melstrom/Dropbox/EECE 315/proj4/testfile.txt");
    ifstream infile("C:/Users/cary/Documents/GitHub/proj4/testfile.txt");

    if ((infile.is_open())) {
        while (infile.good()) {
            struct pcb * pcb1;
            pcb1 = new pcb;
            /* Creates a node at the end of the list */
            if (conductor->next == 0) {
                conductor->pcb = pcb1;
            }

            if (conductor == 0) {
                printf("Out of memory");
                exit(-1);
            }

            int idx1 = 0;
            int idx2 = 0;
            getline(infile, line);
            istringstream buffer(line);
            int value;

            for (int i = 0; i < 4; i++) {
                buffer >> value;

                switch (i) {
                    case 0:
                        pcb1->pid = value;
                        break;
                    case 1:
                        pcb1->tarq = value;
                        if (value > MAXtarq)
                            MAXtarq = value;
                        break;
                    case 2:
                        pcb1->prio = value;
                        break;
                    case 3:
                        pcb1->tncpu = value;
                        pcb1->cpuburst = new int [value];
                        pcb1->ioburst = new int [value - 1];
                        break;
                    default:
                        printf("Impossible to end up here");
                        break;
                }
            }

            for (int k = 0; k < (pcb1->tncpu)*2 - 1; k++) {
                if (k % 2 == 0)
                    buffer >> pcb1->cpuburst[idx1++];

                else
                    buffer >> pcb1->ioburst[idx2++];
            }

            pcb1->turnaround = 0;
            pcb1->wait = 0;
            pcb1->readytime = 0;
            /* initialize the new memory */
            conductor->next = new node;
            conductor = conductor->next;
            conductor->next = 0;
        }
    }
}

//Sorts the jobs according to arrival time

void sortTimes(int MAXtarq, lqueue * queue, struct node * root) {
    struct node * conductor = root;

    for (int i = 0; i <= MAXtarq; i++) {
        conductor = root;
        while (1) {
            if (conductor->pcb->tarq == i)
                queue->add(conductor->pcb);

            conductor = conductor->next;
            if (conductor->next == 0)
                break;
        }
    }
}

//Calculates the wait time for ready item in CPU queue

void calculateWait(pcb * cpuptr, int time) {
    int wait = time - cpuptr->readytime;
    if (wait > 0) {
        cpuptr->wait += wait;
        cpuptr->turnaround += wait;

    }
}

//Skip through 0 time CPU bursts

int skipCompleted(pcb * cpuptr) {
    for (int i = 0; i < cpuptr->tncpu; i++) {
        if (cpuptr->cpuburst[i] != 0)
            return i;
    }
}
//Inserts and sorts the arriving job into CPU queue based on priority

lqueue * priorityInsertion(pcb * ptr, pcb * cpuptr, lqueue * cpuqueue) {
    lqueue * swapqueue = new lqueue;

    if (cpuptr == 0) {
        swapqueue->add(ptr);
    } else {
        swapqueue->add(cpuptr);
        cpuptr = cpuqueue->del();
        if (cpuptr == 0) {
            swapqueue->add(ptr);
            return swapqueue;
        }
        while (cpuptr != 0) {
            if (cpuptr->prio >= ptr->prio) {
                swapqueue->add(ptr);
                while (cpuptr != 0) {
                    swapqueue->add(cpuptr);
                    cpuptr = cpuqueue->del();
                }
                return swapqueue;
                cpuptr = cpuqueue->del();
            }

            swapqueue->add(cpuptr);
            cpuptr = cpuqueue->del();
            if (cpuptr == 0) {
                swapqueue->add(ptr);
                return swapqueue;

            }
        }
    }
    return swapqueue;
}

pcb * checkIOqueue(int time, pcb * cpuptr, pcb * ioptr, lqueue * cpuqueue, lqueue * ioqueue) {
    if (ioptr != 0) {
        if ((ioptr->readytime) <= time) {
            cpuqueue->add(ioptr);
            return ioqueue->del();
        }
    }
}

//Does IO

void doIO(int time, int i, pcb * cpuptr) {
    cpuptr->turnaround += cpuptr->ioburst[i];
    cpuptr->readytime = time + cpuptr->ioburst[i];
    cpuptr->ioburst[i] = 0;
}

lqueue * ioInsertion(pcb * ioptr, pcb * cpuptr, lqueue * ioqueue) {
    if (ioptr == 0) {
        ioqueue->add(cpuptr);
        return ioqueue;
    } else {
        lqueue * ioswap = new lqueue;
        while (ioptr != 0) {
            if (cpuptr->readytime < ioptr->readytime) {
                ioswap->add(cpuptr);
                while (ioptr != 0) {
                    ioswap->add(ioptr);
                    ioptr = ioqueue->del();
                }
                return ioswap;
            }
            ioswap->add(ioptr);
            ioptr = ioqueue->del();
            if (ioptr == 0) {
                ioswap->add(cpuptr);
                return ioswap;
            }
        }
        return ioswap;
    }
}

//Prints PCB information

void calculateAverage(int i, int totalwait, int totalturnaround) {
    printf("Average Wait Time:%d\n"
            "Average Turnaround Time:%d\n", (totalwait / i), (totalturnaround / i));
}

void printReport(struct node * root) {
    struct node *conductor;
    conductor = root;

    int i = 0;
    int totalwait = 0;
    int totalturnaround = 0;
    while (1) {
        i++;
        printf("\nPID:%d\n"
                "Turnaround time:%d\n"
                "Wait Time:%d\n\n",
                conductor->pcb->pid,
                conductor->pcb->turnaround,
                conductor->pcb->wait);
        totalwait += conductor->pcb->wait;
        totalturnaround += conductor->pcb->turnaround;
        conductor = conductor->next;

        if (conductor->next == 0)
            break;
    }
    calculateAverage(i, totalwait, totalturnaround);
}

void fifo() {
    int time = 0;
    struct node * root = new node;
    root->next = 0;

    readFile(root);

    lqueue * queue = new lqueue;
    sortTimes(MAXtarq, queue, root);

    lqueue * cpuqueue = new lqueue;
    lqueue * ioqueue = new lqueue;

    pcb * cpuptr;
    pcb * ioptr = ioqueue->del();
    pcb * ptr = queue->del();

    while (1) {
        if (ptr != 0) {
            if ((ptr->tarq) <= time) {
                cpuqueue = priorityInsertion(ptr, cpuptr, cpuqueue);
                ptr->readytime = time;
                ptr = queue->del();
            }
        }

        if (ioptr != 0) {
            if ((ioptr->readytime) <= time) {
                cpuqueue = priorityInsertion(ioptr, cpuptr, cpuqueue);
                ioptr = ioqueue->del();
            }
        }
        cpuptr = cpuqueue->del();

        if (cpuptr != 0) {
            calculateWait(cpuptr, time);
            int i = skipCompleted(cpuptr);

            //CPU Processing
            for (int k = 0; k < cpuptr->cpuburst[i]; k++) {
                printf("|%d", cpuptr->pid);
                time++;

                //Check for new arrivals
                if (ptr != 0) {
                    if (ptr->tarq <= time) {
                        cpuqueue->add(ptr);
                        ptr = queue->del();
                    }
                }
                //Check IO queue
                if (ioptr != 0) {
                    if ((ioptr->readytime) <= time) {
                        cpuqueue->add(ioptr);
                        ioptr = ioqueue->del();
                    }
                }
            }

            cpuptr->turnaround += cpuptr->cpuburst[i];
            cpuptr->cpuburst[i] = 0;

            //Do IO if CPU complete
            if (i < cpuptr->tncpu - 1) {
                doIO(time, i, cpuptr);
                ioqueue = ioInsertion(ioptr, cpuptr, ioqueue);
                ioptr = ioqueue->del();
            }


        } else
            time++;

        //Complete if all jobs have arrived, and IO/CPU is idle
        if ((time >= MAXtarq) && (cpuptr == 0) && (ioptr == 0))
            break;

    }
    printf("\nFIFO complete\n");
    printReport(root);
}

void RR() {
    int time = 0;
    int quantum = 0;
    struct node * root = new node;
    root->next = 0;

    readFile(root);

    lqueue * queue = new lqueue;
    sortTimes(MAXtarq, queue, root);

    lqueue * cpuqueue = new lqueue;
    lqueue * ioqueue = new lqueue;

    pcb * cpuptr;
    pcb * ioptr = ioqueue->del();
    pcb * ptr = queue->del();

    printf("Quantum time for RR:\n");
    scanf("%d", &quantum);
    while (1) {
        if (ptr != 0) {
            if ((ptr->tarq) <= time) {
                cpuqueue->add(ptr);
                ptr->readytime = time;
                ptr = queue->del();
            }
        }

        if (ioptr != 0) {
            if ((ioptr->readytime) <= time) {
                cpuqueue->add(ioptr);
                ioptr = ioqueue->del();
            }
        }
        cpuptr = cpuqueue->del();

        if (cpuptr != 0) {
            calculateWait(cpuptr, time);
            int i = skipCompleted(cpuptr);

            //CPU Processing
            for (int k = 0; k < quantum; k++) {
                printf("|%d", cpuptr->pid);
                cpuptr->turnaround++;
                cpuptr->cpuburst[i]--;
                time++;

                //Check for new arrivals
                if (ptr != 0) {
                    if (ptr->tarq <= time) {
                        ptr->readytime = time;
                        cpuqueue->add(ptr);
                        ptr = queue->del();
                    }
                }
                //Check IO queue
                if (ioptr != 0) {
                    if ((ioptr->readytime) <= time) {
                        cpuqueue->add(ioptr);
                        ioptr = ioqueue->del();
                    }
                }
                //Do IO if CPU complete
                if ((i < cpuptr->tncpu - 1) && (cpuptr->cpuburst[i]) == 0) {
                    doIO(time, i, cpuptr);
                    ioqueue = ioInsertion(ioptr, cpuptr, ioqueue);
                    ioptr = ioqueue->del();
                    break;
                }
            }
            if (cpuptr->cpuburst[i] != 0) {
                cpuptr->readytime = time;
                cpuqueue->add(cpuptr);
            }
        } else
            time++;

        //Complete if all jobs have arrived, and IO/CPU is idle
        if ((time >= MAXtarq) && (cpuptr == 0) && (ioptr == 0))
            break;

    }
    printf("\nFIFO complete\n");
    printReport(root);
}

void priority() {
    int time = 0;
    struct node * root = new node;
    root->next = 0;

    readFile(root);

    lqueue * queue = new lqueue;
    sortTimes(MAXtarq, queue, root);

    lqueue * cpuqueue = new lqueue;
    lqueue * ioqueue = new lqueue;

    pcb * cpuptr;
    pcb * ioptr = ioqueue->del();
    pcb * ptr = queue->del();

    while (1) {
        if (ptr != 0) {
            if ((ptr->tarq) <= time) {
                cpuqueue = priorityInsertion(ptr, cpuptr, cpuqueue);
                ptr->readytime = time;
                ptr = queue->del();
            }
        }

        if (ioptr != 0) {
            if ((ioptr->readytime) <= time) {
                cpuqueue = priorityInsertion(ioptr, cpuptr, cpuqueue);
                ioptr = ioqueue->del();
            }
        }
        cpuptr = cpuqueue->del();

        if (cpuptr != 0) {
            calculateWait(cpuptr, time);
            int i = skipCompleted(cpuptr);

            //CPU Processing
            for (int k = 0; k < cpuptr->cpuburst[i]; k++) {
                printf("|%d", cpuptr->pid);
                time++;

                //Check for new arrivals
                if (ptr != 0) {
                    if (ptr->tarq <= time) {
                        cpuqueue = priorityInsertion(ptr, cpuptr, cpuqueue);
                        cpuptr = cpuqueue->del();
                        ptr = queue->del();
                    }
                }
                //Check IO queue
                if (ioptr != 0) {
                    if ((ioptr->readytime) <= time) {
                        cpuqueue = priorityInsertion(ioptr, cpuptr, cpuqueue);
                        cpuptr = cpuqueue->del();
                        ioptr = ioqueue->del();
                    }
                }
            }

            cpuptr->turnaround += cpuptr->cpuburst[i];
            cpuptr->cpuburst[i] = 0;

            //Do IO if CPU complete
            if (i < cpuptr->tncpu - 1) {
                doIO(time, i, cpuptr);
                ioqueue = ioInsertion(ioptr, cpuptr, ioqueue);
                ioptr = ioqueue->del();
            }


        } else
            time++;

        //Complete if all jobs have arrived, and IO/CPU is idle
        if ((time >= MAXtarq) && (cpuptr == 0) && (ioptr == 0))
            break;

    }
    printf("\nPriority complete\n");
    printReport(root);

}

void prioritypreempt() {
    int time = 0;
    struct node * root = new node;
    root->next = 0;

    readFile(root);

    lqueue * queue = new lqueue;
    sortTimes(MAXtarq, queue, root);

    lqueue * cpuqueue = new lqueue;
    lqueue * ioqueue = new lqueue;

    pcb * cpuptr;
    pcb * ioptr = ioqueue->del();
    pcb * ptr = queue->del();

    while (1) {
        if (ptr != 0) {
            if ((ptr->tarq) <= time) {
                cpuqueue = priorityInsertion(ptr, cpuptr, cpuqueue);
                ptr->readytime = time;
                ptr = queue->del();
            }
        }

        if (ioptr != 0) {
            if ((ioptr->readytime) <= time) {

                cpuqueue = priorityInsertion(ioptr, cpuptr, cpuqueue);
                cpuptr = cpuqueue->del();
                ioptr = ioqueue->del();
            }
        }
        cpuptr = cpuqueue->del();

        if (cpuptr != 0) {
            calculateWait(cpuptr, time);
            int i = skipCompleted(cpuptr);

            //CPU Processing
            while (cpuptr->cpuburst[i] > 0) {
                //Check for new arrivals
                if (ptr != 0) {
                    if (ptr->tarq <= time) {
                        if (ptr->prio < cpuptr->prio) {
                            lqueue * swapqueue = new lqueue;
                            swapqueue->add(ptr);
                            swapqueue->add(cpuptr);
                            cpuptr = cpuqueue->del();
                            while (cpuptr != 0) {
                                swapqueue->add(cpuptr);
                                cpuptr = cpuqueue->del();
                            }
                            cpuqueue = swapqueue;
                            ptr = queue->del();
                            //                            cpuptr = cpuqueue->del();
                            break;

                        } else {
                            cpuqueue = priorityInsertion(ptr, cpuptr, cpuqueue);
                            cpuptr = cpuqueue->del();
                            ptr = queue->del();
                        }
                    }
                }

                //Check IO queue
                if (ioptr != 0) {
                    if ((ioptr->readytime) <= time) {
                        cpuqueue = priorityInsertion(ioptr, cpuptr, cpuqueue);
                        cpuptr = cpuqueue->del();
                        ioptr = ioqueue->del();
                    }
                }

                printf("|%d", cpuptr->pid);
                cpuptr->turnaround++;
                cpuptr->cpuburst[i]--;
                time++;
                //Do IO if CPU complete
                if ((i < cpuptr->tncpu - 1) && (cpuptr->cpuburst[i] == 0)) {
                    doIO(time, i, cpuptr);
                    ioqueue = ioInsertion(ioptr, cpuptr, ioqueue);
                    ioptr = ioqueue->del();
                }


            }


        } else
            time++;

        //Complete if all jobs have arrived, and IO/CPU is idle
        if ((time >= MAXtarq) && (cpuptr == 0) && (ioptr == 0))
            break;

    }
    printf("\nPriority (preemp) complete\n");
    printReport(root);
}

/*
 *
 */
int main(int argc, char** argv) {

    fifo();
    //        RR();
    priority();
    prioritypreempt();

    return 0;
}