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
    int readytime;	//next ready time
};

struct qNode
{
   struct pcb *pcb;
   qNode *link;
};

class lqueue {
private:
    qNode *front,*rear;

public:
    lqueue()
    {
        front=NULL;
        rear=NULL;
    }

    void add(pcb *n)
    {
        qNode *tmp;
        tmp=new qNode;
        if(tmp==NULL)
           cout<<"\nQUEUE FULL";

        tmp->pcb=n;
        tmp->link=NULL;
        if(front==NULL)
        {
            rear=front=tmp;
            return;
        }
            rear->link=tmp;
            rear=rear->link;
    }

    pcb * del()
    {
        if(front==NULL)
        {
            //cout<<"\nQUEUE EMPTY";
            return NULL;
        }
        qNode *tmp;
        struct pcb *n;
        n=front->pcb;
        tmp=front;
        front=front->link;
        delete tmp;
        return n;
    }

    ~lqueue()
    {
        if(front==NULL)
           return;
        qNode *tmp;
        while(front!=NULL)
        {
            tmp=front;
            front=front->link;
            delete tmp;
        }
     }
};


struct node {
  struct pcb *pcb;
  struct node *next;
};


struct node *root;
struct node *conductor;
int MAXtarq = 0;

void readFile(struct node * root) {
    conductor = root;
    string line;

//    ifstream infile ("/Users/melstrom/Dropbox/EECE 315/proj4/testfile.txt");
    ifstream infile ("C:/Users/cary/Documents/GitHub/proj4/testfile.txt");

    if ((infile.is_open())) {

        while (infile.good()) {
            struct pcb * pcb1;
            pcb1 = new pcb;
            /* Creates a node at the end of the list */
            if (conductor->next==0) {
                conductor->pcb = pcb1;
            }

            if ( conductor == 0 )
            {
                printf( "Out of memory" );
                exit(-1);
            }

            int idx1=0;
            int idx2=0;
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
                        pcb1->ioburst = new int [value-1];
                        break;
                    default:
                        printf("Impossible to end up here");
                        break;
                }
            }

            for (int k = 0; k < (pcb1->tncpu)*2-1; k++) {
                if (k%2==0)
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

void sortTimes(int MAXtarq, lqueue * queue, struct node * root) {
    conductor = root;

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

void priorityInsertion (pcb * ptr, pcb * cpuptr, lqueue * cpuqueue) {
    lqueue * swapqueue = new lqueue;
    cpuptr = cpuqueue->del();
    while (cpuptr != 0) {
        if (cpuptr->prio > ptr->prio) {
                swapqueue->add(cpuptr);
                cpuptr = cpuqueue->del();
        }
        else
                swapqueue->add(ptr);
    }
    cpuqueue = swapqueue;
}

void checkIOqueue (int time, pcb * cpuptr, pcb * ioptr, lqueue * cpuqueue, lqueue * ioqueue) {
    if (ioptr != 0) {
        if ((ioptr->readytime) >= time){
            priorityInsertion(ioptr, cpuptr, cpuqueue);
            ioqueue->del();
        }
    }
}

void doIO (int i, pcb * cpuptr) {
    cpuptr->turnaround += cpuptr->ioburst[i];
    cpuptr->readytime = cpuptr->turnaround;
    cpuptr->ioburst[i] = 0;
}

void printReport() {
    struct node *looper;
    looper = root;

    while (1) {
        printf("\nPID:%d\n"
                "Turnaround time:%d\n"
                "Wait Time:%d\n\n",
                looper->pcb->pid,
                looper->pcb->turnaround,
                looper->pcb->wait);
        looper = looper->next;

        if (looper->next == 0)
            break;
    }

}
void fifo () {
    root = new node;
    root->next = 0;

    int time=0;
    readFile(root);

    lqueue * queue = new lqueue;
    sortTimes(MAXtarq, queue, root);

    pcb * ptr = queue->del();

    while (ptr != 0) {
        int idx1=0;
        int idx2=0;
        ptr->wait = (ptr->wait + (time - ptr->turnaround));

        for(int i = 0; i < (ptr->tncpu)*2-1; i++) {
            if (i%2==0)
                time += ptr->cpuburst[idx1++];

            else
                time += ptr->ioburst[idx2++];
            printf("|%d", ptr->pid);
        }

        ptr->turnaround = (time - ptr->tarq);
        ptr = queue->del();
    }
    printf("\nFIFO complete\n");
}

void RR () {
    int temp, temp2, time = 0;
    int quantum;
    root = new node;
    root->next = 0;

    readFile(root);

    lqueue * queue = new lqueue;
    sortTimes(MAXtarq, queue, root);

    printf("Quantum time for RR:\n");
    scanf("%d", &quantum);
    
    lqueue * cpuqueue = new lqueue;
    pcb * cpuptr;
    pcb * ptr = queue->del();

    while (1) {
        if (ptr != 0) {
            if (ptr->readytime <= time) {
                cpuqueue->add(ptr);
                ptr->readytime = time;
                ptr = queue->del();
            }
        }

        cpuptr = cpuqueue->del();

        if (cpuptr != 0) {
            if (cpuptr->readytime > time) {
                cpuqueue->add(cpuptr);
                cpuptr = cpuqueue->del();
                break;
            }
            
            temp2 = cpuptr->readytime - time;
            if (temp2 > 0)
                cpuptr->wait = cpuptr->wait + temp2;

            int i = 0;

            while (i < (cpuptr->tncpu - 1)) {
                if (cpuptr->cpuburst[i]==0)
                    i++;
                else
                    break;
            }

            for (int k = 0; k < quantum; k++) {
                cpuptr->cpuburst[i] = cpuptr->cpuburst[i]-1;
                time++;
                cpuptr->turnaround = cpuptr->turnaround + 1;

                if (ptr != 0)

                    if (ptr->tarq <= time) {
                        cpuqueue->add(ptr);
                        temp = time;
                        ptr = queue->del();
                    }

                if (cpuptr->cpuburst[i] == 0) {
                    doIO (i, cpuptr);
                    break;
                }
            }
            if (i != (cpuptr->tncpu - 1))
                cpuqueue->add(cpuptr);
        }

        else
            time++;

        if ((time >= MAXtarq)&&(cpuptr==0))
            break;
    }
    printf("\nRR complete\n");
}

void priority () {
    int temp, temp2, time = 0;
    root = new node;
    root->next = 0;
	
    readFile(root);
	
    lqueue * queue = new lqueue;
    sortTimes(MAXtarq, queue, root);

    lqueue * cpuqueue = new lqueue;
    lqueue * ioqueue = new lqueue;
    
    pcb * cpuptr;
    pcb * ioptr;
    pcb * ptr = queue->del();
	
    while (1) {
        if (ptr != 0) {
            if ((ptr->readytime) <= time) {
                priorityInsertion(ptr, cpuptr, cpuqueue);
                ptr->readytime = time;
                ptr = queue->del();
            }
        }
        
        checkIOqueue (time, cpuptr, ioptr, cpuqueue, ioqueue);

        cpuptr = cpuqueue->del();
		
         if (cpuptr != 0) {
            
            temp2 = cpuptr->readytime - time;
            if (temp2 > 0)
                cpuptr->wait = cpuptr->wait + temp2;
            
            //Skip through 0 time CPU bursts
            int i = 0;
            while (i < (cpuptr->tncpu - 1)) {
                if (cpuptr->cpuburst[i]==0)
                    i++;
                else
                    break;
            }
		
            //CPU Processing
            for (int k = 0; k < cpuptr->cpuburst[i]; k++) {
                cpuptr->cpuburst[i] = cpuptr->cpuburst[i]-1;
                time++;
                cpuptr->turnaround = cpuptr->turnaround + 1;
		
                //Check for new arrivals
                if (ptr != 0){		
                    if (ptr->tarq <= time) {
                        priorityInsertion(ptr, cpuptr, cpuqueue);
                        temp = time;
                        ptr = queue->del();
                    }
                }
                
                //Do IO if CPU complete
                if (cpuptr->cpuburst[i] == 0) {
                    doIO (i, cpuptr);
                    ioptr = cpuptr;
                    ioqueue->add(ioptr);
                    break;
                }
            }
            
            //Re-adds to queue if there's more CPU
            if (i != (cpuptr->tncpu - 1))
                cpuqueue->add(cpuptr);
        }
		
        else
            time++;
		
        //Complete if all jobs have arrived, and IO/CPU is idle
        if ((time >= MAXtarq)&&(cpuptr==0))
            break;
	
    }
}

void prioritypreempt () {
    int temp, temp2, time = 0;
    root = new node;
    root->next = 0;

    readFile(root);

    lqueue * queue = new lqueue;
    sortTimes(MAXtarq, queue, root);

    lqueue * cpuqueue = new lqueue;
    lqueue * ioqueue = new lqueue;
    
    pcb * cpuptr;
    pcb * ioptr;
    pcb * ptr = queue->del();

    while (1) {
        if (ptr != 0) {
            if (ptr->readytime <= time) {
                priorityInsertion(ptr, cpuptr, cpuqueue);
                ptr->readytime = time;
                ptr = queue->del();
            }
        }
        
        checkIOqueue (time, cpuptr, ioptr, cpuqueue, ioqueue);

        cpuptr = cpuqueue->del();
        if (cpuptr != 0) {
            
            temp2 = cpuptr->readytime - time;
            if (temp2 > 0)
                cpuptr->wait = cpuptr->wait + temp2;
            
            //Skip through 0 time CPU bursts
            int i = 0;
            while (i < (cpuptr->tncpu - 1)) {
                if (cpuptr->cpuburst[i]==0)
                    i++;
                else
                    break;
            }
            
            //CPU Processing
            for (int k = 0; k < cpuptr->cpuburst[i]; k++) {
                cpuptr->cpuburst[i] = cpuptr->cpuburst[i]-1;
                cpuptr->turnaround = cpuptr->turnaround + 1;
                time++;
                
                //Check for new arrivals
                if (ptr != 0){		
                    if (ptr->tarq <= time) {
                        //Check preemption
                        if (ptr->prio > cpuptr->prio) {
                            lqueue * swapqueue = new lqueue;
                            swapqueue->add(ptr);
                            swapqueue->add(cpuptr);
                            cpuptr = cpuqueue->del();
                            while (cpuptr != 0) {
                                swapqueue->add(cpuptr);
                                cpuptr = cpuqueue->del();
                            }
                            cpuqueue = swapqueue;
                        }
                        //Non-preemptive insertion
                        priorityInsertion(ptr, cpuptr, cpuqueue);
                        temp = time;
                        ptr = queue->del();
                    }
                }
                
                //Do IO if CPU complete
                if (cpuptr->cpuburst[i] == 0) {
                        doIO (i, cpuptr);
                        ioptr = cpuptr;
                        ioqueue->add(ioptr);
                        break;
                }
            }
            
            //Re-adds to queue if there's more CPU
            if (i != (cpuptr->tncpu - 1))
                cpuqueue->add(cpuptr);
        }
        
        else
            time++;
        
        //Complete if all jobs have arrived, and IO/CPU is idle
        if ((time >= MAXtarq)&&(cpuptr==0)&&(ioptr==0))
            break;
    }
}
/*
 * 
 */
int main(int argc, char** argv) {

    fifo();
    printReport();

    RR();
    printReport();
    
    return 0;
}