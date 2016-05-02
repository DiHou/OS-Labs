#include <iostream>
#include <unistd.h>
#include <string>
#include <cstdio>
#include <string>
#include <vector>
#include <fstream>
#include <queue>
#include <cstdlib>
#include <algorithm>
#include <stdio.h>

using namespace std;


/*****define Process class********/

class Process {
public:
    int AT, TC, CB, IO, FT, TT, IT, CW, beginTC, procID, sPrio, dPrio, CBleft;
    
    Process(int at, int tc, int cb, int io, int procid);
};

//**constructor**
Process::Process(int at, int tc, int cb, int io, int procid){
    AT = at;
    TC = tc;
    CB = cb;
    IO = io;
    FT = 0;
    TT = 0;
    IT = 0;
    CW = 0;
    beginTC = tc;
    procID = procid;
    sPrio = 0;
    dPrio = 0;
    CBleft = 0;
    
}



/*****define Event class********/

class Event {
public:
    static int eventCount;
    int eventAT, eventFT, eventID, pid, state;
    
    Event();
    Event(int eventat, int eventft, int eventpid, int eventstate);
};


//**constructor**
int Event::eventCount = 0; //used for giving timestamp for each new event called

Event::Event() {
    
}

Event::Event(int eventat, int eventft, int eventpid, int eventstate){
    eventAT = eventat;
    eventFT = eventft;
    pid = eventpid;
    state = eventstate;
    eventID = eventCount++;
}



/*****define Scheduler class********/


class Scheduler {
public:
    string schedulerType;
    int quantum;
    
    Scheduler();
    
    virtual Event select(vector<Event>& runQ, vector<Process>& procRecord) {
        return * new Event();
    };
    
};

class FCFS : public Scheduler {
public:
    FCFS();
    Event select(vector<Event>& runQ, vector<Process>& procRecord);
};

class LCFS : public Scheduler {
public:
    LCFS();
    Event select(vector<Event>& runQ, vector<Process>& procRecord);
};

class SJF : public Scheduler {
public:
    SJF();
    Event select(vector<Event>& runQ, vector<Process>& procRecord);
};

class RR : public Scheduler {
public:
    RR(int rquantum);
    Event select(vector<Event>& runQ, vector<Process>& procRecord);
};

class PRIO : public Scheduler {
public:
    PRIO(int pquantum);
    Event select(vector<Event>& runQ, vector<Process>& procRecord);
};


//**constructor**
Scheduler::Scheduler() {
    quantum = 0;
}


FCFS::FCFS() {
    schedulerType = "FCFS";
}

Event FCFS::select(vector<Event> &runQueue, vector<Process>& procRecord) {
    
    for (int i = 0; i < runQueue.size(); ++i) {
        if (runQueue[i].eventFT < runQueue[0].eventFT || (runQueue[i].eventFT == runQueue[0].eventFT && runQueue[i].eventID < runQueue[0].eventID)) {
            
            Event temp = runQueue[i];
            runQueue[i] = runQueue[0];
            runQueue[0] = temp;
        }
        
    }
    Event first = runQueue[0];
    runQueue.erase(runQueue.begin());
    return first;
    
}


LCFS::LCFS() {
    schedulerType = "LCFS";
}

Event LCFS::select(vector<Event> &runQueue, vector<Process>& procRecord) {
    for (int i = 0; i < runQueue.size(); ++i) {
        if (runQueue[i].eventFT > runQueue[0].eventFT || (runQueue[i].eventFT == runQueue[0].eventFT && runQueue[i].eventID > runQueue[0].eventID)) {
            
            Event temp = runQueue[i];
            runQueue[i] = runQueue[0];
            runQueue[0] = temp;
        }
        
    }
    Event first = runQueue[0];
    runQueue.erase(runQueue.begin());
    return first;
}


SJF::SJF() {
    schedulerType = "SJF";
}

Event SJF::select(vector<Event> &runQueue, vector<Process>& procRecord) {
    
    for (int i = 0; i < runQueue.size(); ++i) {
        if (procRecord[runQueue[i].pid].TC < procRecord[runQueue[0].pid].TC || (procRecord[runQueue[i].pid].TC == procRecord[runQueue[0].pid].TC && runQueue[i].eventID < runQueue[0].eventID)) {
            
            Event temp = runQueue[i];
            runQueue[i] = runQueue[0];
            runQueue[0] = temp;
        }
        
    }
    Event first = runQueue[0];
    runQueue.erase(runQueue.begin());
    return first;
    
}


RR::RR(int rquantum) {
    schedulerType = "RR";
    quantum = rquantum;
}

Event RR::select(vector<Event> &runQueue, vector<Process>& procRecord) {
    for (int i = 0; i < runQueue.size(); ++i) {
        if (runQueue[i].eventFT < runQueue[0].eventFT || (runQueue[i].eventFT == runQueue[0].eventFT && runQueue[i].eventID < runQueue[0].eventID)) {
            
            Event temp = runQueue[i];
            runQueue[i] = runQueue[0];
            runQueue[0] = temp;
        }
        
    }
    Event first = runQueue[0];
    runQueue.erase(runQueue.begin());
    return first;

}


PRIO::PRIO(int pquantum) {
    schedulerType = "PRIO";
    quantum = pquantum;
}

Event PRIO::select(vector<Event> &runQueue, vector<Process>& procRecord){

    for (int i = 0; i < runQueue.size(); ++i) {
        if (procRecord[runQueue[i].pid].dPrio > procRecord[runQueue[0].pid].dPrio || (procRecord[runQueue[i].pid].dPrio == procRecord[runQueue[0].pid].dPrio && runQueue[i].eventID < runQueue[0].eventID)) {
            
            Event temp = runQueue[i];
            runQueue[i] = runQueue[0];
            runQueue[0] = temp;
        }
        
    }
    Event first = runQueue[0];
    runQueue.erase(runQueue.begin());
    return first;

}



/*****define DES class********/

class DES {
public:
    int procCount, randCount, ofs, CPUoff, IOoff, maxFT; //maxFT used for picking out the lastest FT among all processes
    double CPUoffperiod, IOoffperiod, TTsum, CWsum;
    bool verbose, CPUbusy;
    
    vector<Process> procRecord;
    vector<int> rVal;
    vector<Event> runQueue;
    vector<Event> eventQueue;
    
    int myrandom(int burst);

    void getProc(string input);
    void getRand(string rfile);
    void addToEvent(int at, int ft, int eventid, int state);
    void getSchedType(string schedType);
    void makeEventQ();

    void Prepare(bool vFlag, string input, string rfile, string schedType);
    void Simulate();
    void checkEvent(Event event);
    void Print();
    
    DES();
    Scheduler * type;
    
};

//**DES constructor**

DES::DES() {
    procCount = 0;
    randCount = 0;
    ofs = 0;
    CPUoff = 0;
    IOoff =0;
    maxFT = 0;
    CPUoffperiod = 0;
    IOoffperiod = 0;
    TTsum = 0;
    CWsum = 0;
    verbose = false;
    CPUbusy = false;
}


/********************************/


void DES::getProc(string input) {
    int AT, TC, CB, IO;
    int ProcID = 0;
    ifstream readFile(input);
    while (readFile >> AT >> TC >> CB >> IO) {
        procRecord.push_back(*new Process(AT, TC, CB, IO, ProcID));
        ++procCount; //start from 1
        ProcID++; //start from 0
    }
    readFile.close();
}


void DES::makeEventQ() {
    //initially all events are of sorted time order
    for (int i=0; i < procRecord.size(); ++i){
        //the eventAT and eventFT in created state are same
        eventQueue.push_back(*new Event(procRecord[i].AT, procRecord[i].AT, i, 1));
    
    //cout<<eventQueue[i].eventID<<endl;
    }
}


int DES::myrandom(int burst) {
    if (ofs == randCount) {
        ofs = 0;
    }
    int bursttime = 1 + rVal[ofs] % burst;
    ofs++;
    return bursttime;
}


void DES::getRand(string rfile) {
    
    ifstream readRand(rfile);
    readRand >> randCount; // first NO is random total number
    int r = 0;
    for(int i = 0; i < randCount; ++i) {
        readRand >> r;
        rVal.push_back(r);
    }
    
    for (int i = 0; i < procCount; ++i) { //call the first several rands to assign static priority for each process
        procRecord[i].sPrio = myrandom(4);
        procRecord[i].dPrio = procRecord[i].sPrio - 1;
    }
    readRand.close();
}



void DES::getSchedType(string schedType) {
    if (schedType[0] == 'F')
        type = new FCFS();
    if (schedType[0] == 'L')
        type = new LCFS();
    if (schedType[0] == 'S')
        type = new SJF();
    if (schedType[0] == 'R') {
        int rquantum = 0;
        for (int i=1; i < schedType.size(); ++i) {
            rquantum *= 10;
            int digit = schedType[i] - '0';
            rquantum += digit;
        }
        type = new RR(rquantum);
        //cout << rquantum << endl;
    }
    if (schedType[0] == 'P') {
        int pquantum = 0;
        for (int i=1; i<schedType.size(); ++i) {
            pquantum *= 10;
            int digit = schedType[i] - '0';
            pquantum += digit;
            
        }
        type = new PRIO(pquantum);
    }
}


void DES::Prepare(bool vFlag, string input, string rfile, string schedType) {
    verbose = vFlag;
    getProc(input);
    makeEventQ();
    getRand(rfile);
    getSchedType(schedType);
    
    //cout << (*type).schedulerType << endl;
    /* for (int i=0;i<procRecord.size();i++) {
     cout<<procRecord[i].AT <<" "<<procRecord[i].TC<<" "<<procRecord[i].CB<< " "<< procRecord[i].IO<<" "<<procRecord[i].ProcID<< " "<<procRecord[i].StaticPRIO<<endl;
     }*/
   
}


void DES::checkEvent(Event event) {
    
    
    //create - ready, time arrival and finish of "create" state is same
    //cout << event.state<<endl;
    if (event.state == 1) {
        runQueue.push_back(*new Event(event.eventAT, event.eventFT, event.pid, 2));
        
    }
    
    
    
    //ready - run
    if (event.state == 2) {
        procRecord[event.pid].CW += event.eventFT - event.eventAT;
        if (type->schedulerType == "FCFS" || type->schedulerType =="LCFS" || type->schedulerType =="SJF" ) {
            int nextCB = myrandom(procRecord[event.pid].CB);
            if (nextCB > procRecord[event.pid].TC) {
                nextCB = procRecord[event.pid].TC;
            }
            addToEvent(event.eventFT, event.eventFT + nextCB, event.pid, 3);
        }
        
        
        
        if (type->schedulerType == "RR")         {
            if (procRecord[event.pid].CBleft == 0) {
                procRecord[event.pid].CBleft = myrandom(procRecord[event.pid].CB);
                if (procRecord[event.pid].CBleft > procRecord[event.pid].TC) { procRecord[event.pid].CBleft = procRecord[event.pid].TC; }
            }
            
            if (procRecord[event.pid].CBleft <= type->quantum){
                addToEvent(event.eventFT, event.eventFT + procRecord[event.pid].CBleft, event.pid, 3);
                procRecord[event.pid].CBleft = 0;
            }
            else{
                addToEvent(event.eventFT, event.eventFT + type->quantum, event.pid, 5);
                procRecord[event.pid].CBleft -= type->quantum;
            }
        }
        
        
        
        if (type->schedulerType == "PRIO") {
            
           if (procRecord[event.pid].CBleft == 0){
                procRecord[event.pid].CBleft = myrandom(procRecord[event.pid].CB);
                
                if (procRecord[event.pid].CBleft > procRecord[event.pid].TC) {
                    procRecord[event.pid].CBleft = procRecord[event.pid].TC;
                }
           }
            
           if (procRecord[event.pid].CBleft <= type->quantum) {
                addToEvent(event.eventFT, event.eventFT + procRecord[event.pid].CBleft, event.pid, 3);
                procRecord[event.pid].CBleft = 0;
                
           } else {
                //preemptive, CBleft > 0,
                addToEvent(event.eventFT, event.eventFT + type->quantum, event.pid, 5);
                procRecord[event.pid].CBleft -= type->quantum;
           }
        }
    }
    
    //run - block
    if (event.state == 3){
        
        CPUbusy = false;
        procRecord[event.pid].TC -= event.eventFT - event.eventAT;
        
        CPUoff = event.eventFT;
        
        //check if end of process
        if (procRecord[event.pid].TC == 0) {
            procRecord[event.pid].FT = event.eventFT;
            procRecord[event.pid].TT = procRecord[event.pid].FT - procRecord[event.pid].AT;
            maxFT = procRecord[event.pid].FT;
            TTsum += procRecord[event.pid].TT;
            CWsum += procRecord[event.pid].CW;

            return;
        }
    
        
        if (type->schedulerType == "PRIO") {
            procRecord[event.pid].dPrio = procRecord[event.pid].sPrio - 1;
        }
        
        
        int rand = myrandom(procRecord[event.pid].IO);
        addToEvent(event.eventFT, event.eventFT + rand, event.pid, 4);
        
        if (IOoff < event.eventFT){
            IOoffperiod += event.eventFT - IOoff;
        }
            
        if (IOoff < event.eventFT + rand){
            IOoff = event.eventFT + rand;
        }
      
    }
    
    
    
    //run - ready, preempted
    if (event.state == 5) {
        procRecord[event.pid].TC -= event.eventFT - event.eventAT;
        CPUbusy = false;
        CPUoff = event.eventFT;
        
        //check if end of process
        if (procRecord[event.pid].TC == 0) {
            procRecord[event.pid].FT = event.eventFT;
            procRecord[event.pid].TT = procRecord[event.pid].FT - procRecord[event.pid].AT;
            maxFT = procRecord[event.pid].FT;
            TTsum += procRecord[event.pid].TT;
            CWsum += procRecord[event.pid].CW;

            return;
        }
        
        
        if (type->schedulerType == "PRIO"){
        procRecord[event.pid].dPrio--;
        }
        
        //cout <<procRecord[event.pid].dPrio<<endl;
        runQueue.push_back(*new Event(event.eventFT, event.eventFT, event.pid, 2));
       
    }

    
    
    //4.block - ready
    if (event.state == 4)
    {
        procRecord[event.pid].IT += event.eventFT - event.eventAT;
        runQueue.push_back(*new Event(event.eventFT, event.eventFT, event.pid, 2));
    }
}




// eventQueue sorted by time of arrival
void DES::addToEvent(int at, int ft, int eventid, int state)
{
    eventQueue.push_back(*new Event(at, ft, eventid, state));
    for (int i = (int)eventQueue.size()-1; i > 0 ; --i) {
        if ((eventQueue[i-1].eventFT > eventQueue[i].eventFT) || (eventQueue[i-1].eventFT == eventQueue[i].eventFT && eventQueue[i-1].eventFT > eventQueue[i].eventFT)) {
            
            Event temp = eventQueue[i];
            eventQueue[i] = eventQueue[i-1];
            eventQueue[i-1] = temp;
        } else {
            break;
        }
    }
    
}


void DES::Simulate() {
        
        while (true){
            if (!CPUbusy && !runQueue.empty()) {
               //cout<<type->selectFirst(runQueue, procRecord).eventID<<endl;
                //cout<< (eventQueue.front() < type->selectFirst(runQueue, procRecord)?1:0)<<endl;
                if (eventQueue.empty() || eventQueue.front().eventFT != runQueue[runQueue.size()-1].eventFT) {
                    //cout << runQueue.front().eventID <<" " << runQueue.front().pid << " "<<runQueue.front().state << endl;
                    
                    //for PRIO, check if events dynamic priorities are all -1
                    if (type->schedulerType == "PRIO") {
                        int count = 0;
                        for (int i = 0; i < runQueue.size(); ++i) {
                            if (procRecord[runQueue[i].pid].dPrio <= -1) {
                                count++;
                            } else {
                                break;
                            }
                        }
                            
                            //cout << count<<endl;
                        if (count == runQueue.size()) {
                            for (int i = 0; i < runQueue.size(); ++i)
                                procRecord[runQueue[i].pid].dPrio = procRecord[runQueue[i].pid].sPrio - 1;
                        }
                    }
                    
                    //call scheduler
                    Event event = type->select(runQueue, procRecord);
                    
                    //cout <<event.pid<<" "<< event.eventID<<" "<<procRecord[event.pid].sPrio<< " "<<procRecord[event.pid].dPrio <<endl;
                    
                    if (event.eventFT > CPUoff) {
                        CPUoffperiod += event.eventFT - CPUoff;
                        CPUoff = event.eventFT;
                    }
                    CPUbusy = true;
                    addToEvent(event.eventFT, CPUoff, event.pid, 2);
                    //cout << eventQueue.front().eventID <<" " << eventQueue.front().pid <<endl;
                }
            }
            
            if (!eventQueue.empty()) {
                //cout << eventQueue.front().eventID <<" " << eventQueue.front().pid <<endl;
                checkEvent(eventQueue.front());
                //cout << runQueue.front().eventID <<" " << runQueue.front().pid <<endl;
                //cout << eventQueue.front().eventID <<" " << eventQueue.front().pid <<endl;
                eventQueue.erase(eventQueue.begin());
            }
            
            else
                break;
        }
}



void DES::Print()
{
    //print title
    if(type->schedulerType == "FCFS" || type->schedulerType == "LCFS" || type->schedulerType == "SJF")
        cout << type->schedulerType<<endl;
    else
        cout << type->schedulerType<<" "<<type->quantum<<endl;
    
    
    //print summary
    for (int i = 0; i < procRecord.size(); ++i) {
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n", i, procRecord[i].AT, procRecord[i].beginTC, procRecord[i].CB, procRecord[i].IO, procRecord[i].sPrio, procRecord[i].FT, procRecord[i].TT, procRecord[i].IT, procRecord[i].CW);
    }
    
    
    IOoffperiod += maxFT - IOoff;
    double CPUutilization = (maxFT-CPUoffperiod) / maxFT * 100;
    double IOutilization = (maxFT - IOoffperiod) / maxFT * 100;
    double aveTT = TTsum / procCount;
    double aveCW = CWsum / procCount;
    double throughput = procCount * 100.0 / maxFT;
    
    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", maxFT, CPUutilization, IOutilization, aveTT, aveCW, throughput);
}




// <program> [-v] [-s<schedspec>] inputfile randfile
// from command line:  –s [ FLS | R<num> | P<num> ]
// i.e. -v –sR10 input rfile
int main(int argc, char * argv[]){
    bool vFlag = false; //verbose flag
    string sValue = ""; // scheduler type
    int c;
    
    while ((c = getopt (argc, argv, "vs:")) != -1) {
        
        switch (c){
            case 'v':
                vFlag = true;
                break;
                
            case 's':
                sValue = optarg;
                break;
        }
    }
    
    DES simulator;
    //argv[optind] - first non-option argument
    simulator.Prepare(vFlag, argv[optind], argv[optind + 1], sValue);
    simulator.Simulate();
    simulator.Print();
    
    return 0;
}
