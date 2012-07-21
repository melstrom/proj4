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

struct qNode
{
   //int data;
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
            cout<<"\nQUEUE EMPTY";
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

void readFile() {
    ifstream infile ("/Users/melstrom/Dropbox/EECE 315/proj4/testfile.txt");
    string line;


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
            /* initialize the new memory */
            conductor->next = 0;

            int idx1=0;
            int idx2=0;
            getline(infile, line);
            istringstream buffer(line);
            int value;
            //strtok and move data to pcb
            for (int i = 0; i < 4; i++) {
                buffer >> value;
                switch (i) {
                    case 0:
                        conductor->pcb->pid = value;
                        break;
                    case 1:
                        conductor->pcb->tarq = value;
                        if (value > MAXtarq)
                            MAXtarq = value;
                        break;
                    case 2:
                        conductor->pcb->prio = value;
                        break;
                    case 3:
                        conductor->pcb->tncpu = value;
                        conductor->pcb->cpuburst = new int [value];
                        conductor->pcb->ioburst = new int [value-1];
                        break;
                    default:
                        printf("Impossible to end up here");
                        break;
                }
            }
            for (int k = 0; k < (conductor->pcb->tncpu)*2-1; k++) {
                if (k%2==0)
                    buffer >> conductor->pcb->cpuburst[idx1++];
                else
                    buffer >> conductor->pcb->ioburst[idx2++];
            }

            conductor->next = new node;
            conductor = conductor->next;
        }
    }
}

void sortTimes(int MAXtarq, lqueue * queue) {
    struct node *sorter;
    

    for (int i = 0; i <= MAXtarq; i++) {
        sorter = root;
        while (1) {
            if (sorter->pcb->tarq == i)
                queue->add(sorter->pcb);
            sorter = sorter->next;
            if (sorter == 0)
                break;
        }
    }
}

void fifo () {
    int time;
    
    readFile();

    lqueue * queue = new lqueue;
    sortTimes(MAXtarq, queue);

    pcb * ptr = queue->del();
    while (ptr != 0) {
        for(int i=0; i<ptr->tncpu; i++) {
            time =+ ptr->cpuburst[i];
            printf("|%d", ptr->pid);
        }

        for(int k=0; k<ptr->tncpu-1; k++) {
            time =+ ptr->ioburst[k];
        }

        ptr->turnaround = (time - ptr->tarq);
        ptr->wait = (time - ptr->tarq - ptr->turnaround);
        
        ptr = queue->del();
    }
}

/*
 * 
 */
int main(int argc, char** argv) {
    
    root = new node;
    root->next = 0;
    conductor = root;

    fifo();
    return 0;
}

