//
//  main.cpp
//  ioscheduling
//
//  Created by Hou Di on 12/9/15.
//  Copyright © 2015 Hou Di. All rights reserved.
//

#include <iostream>
#include <unistd.h>
#include <string>
#include <cstdio>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <queue>
#include <cstdlib>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

/*******IOTask Class*******/

class IOTask {
    
    public:
    int ID;
    int timestamp;
    int track;
    int diskStart;
    int diskEnd;
    
    IOTask(int id, int time, int tk);
};

IOTask::IOTask(int id, int time, int tk){
    ID = id;
    timestamp = time;
    track = tk;
    diskStart = 0;
    diskEnd = 0;
    
}

/*******Scheduler Class*******/

class IOScheduler{
    
    public:
    int IOIdx, curIO, seekTime, readyPtr, curTrack, finishTime, issueTime, turnaround, curLast;
    int total_time, tol_movement, max_waittime, tol_seekTime, tol_waitingTime;
    double avg_turnaround, avg_waittime;
    
    string algorithm;
    
    bool verbose, IOflag, upflag;
    
    IOScheduler();
    
    
    void getFirstIO();
    void checkAdd();
    void checkFinish();
    void checkIssue();
    void FIFO();
    void SSTF();
    void SCAN();
    void CSCAN();
    void FSCAN();
    
    void print_request();
    void print_sum();
    
    
    
    void Run(string infile, string Algo, bool _v);
    vector<IOTask>IOList;
    vector<IOTask>IOFinish;
    
    
};


IOScheduler::IOScheduler() {
    
    IOIdx = 0;
    curIO = 0;
    seekTime = 0;
    readyPtr = 0;
    curTrack = 0;
    finishTime = 0;
    issueTime = 0;
    verbose = 0;
    IOflag = 1;
    upflag = 1;
    curLast = 0;
    
    
    total_time = 0;
    tol_movement = 0;
    max_waittime = 0;
    avg_turnaround = 0;
    avg_waittime = 0;
    tol_seekTime = 0;
    turnaround = 0;
    
    algorithm = "";
    
}


/************************/


void IOScheduler::getFirstIO(){
    
    if (verbose) {
        cout << "TRACE" << endl;
        printf("%d:%6d add %d\n", IOList[0].timestamp, IOList[0].ID, IOList[0].track);
        printf("%d:%6d issue %d %d\n", IOList[0].timestamp, IOList[0].ID, IOList[0].track, curTrack);
    }
    
    
    curIO = IOList[0].ID;
    issueTime = IOList[0].timestamp;
    seekTime = abs(IOList[0].track - curTrack);
    tol_seekTime += seekTime;
    finishTime = issueTime + seekTime;
    curTrack = IOList[0].track;
    turnaround += finishTime - IOList[0].timestamp;
}



void IOScheduler::checkAdd() {
    
    if (readyPtr == IOList.size() -1) {
        return;
    }
    
    if (readyPtr ==  -1) {
        if (verbose) {
            printf("%d:%6d add %d\n", IOList[0].timestamp, IOList[0].ID, IOList[0].track);
        }
        finishTime = IOList[0].timestamp;
        readyPtr++;
        return;
    }
    
    int follow = 0;
    for (int i = readyPtr + 1; IOList[i].timestamp <= finishTime; ++i) {
        follow = i;
        if (verbose) {
            printf("%d:%6d add %d\n", IOList[i].timestamp, IOList[i].ID, IOList[i].track);
        }
        if (i == IOList.size() - 1) {
            break;
        }
    }
    
    
    if (readyPtr < follow) {
        readyPtr = follow;
    }
    
    
    
}



void IOScheduler::checkFinish() {
    
    if (IOflag) {
        
        //cout << finishTime << " " << seekTime<<endl;
        if (verbose) {
            printf("%d:%6d finish %d\n", finishTime, curIO, finishTime-IOList[0].timestamp);
            //cout << finishTime <<" " <<IOList[0].timestamp<<endl;
        }
        
        IOList[0].diskStart = issueTime;
        IOList[0].diskEnd = finishTime;
        IOFinish.push_back(IOList[0]);
        
        IOList.erase(IOList.begin());
        readyPtr--;
        
        if (algorithm == "f") {
            curLast--;
        }
        
        
        IOflag = 0;
        //cout << readyPtr <<endl;
    }
    
    
    
}


void IOScheduler::FIFO() {
    
    while (IOList.size() > 0) {
        checkAdd();
        checkFinish();
        
        if (readyPtr > -1) {
            if (IOList[0].timestamp < finishTime ) {
                issueTime = finishTime;
            }else{
                issueTime = IOList[0].timestamp;
            }
            tol_waitingTime += issueTime - IOList[0].timestamp;
            max_waittime = max(max_waittime, issueTime-IOList[0].timestamp);
            
            if (verbose) {
                printf("%d:%6d issue %d %d\n", issueTime, IOList[0].ID, IOList[0].track, curTrack);
            }
            
            seekTime = abs(IOList[0].track - curTrack);
            tol_seekTime += seekTime;
            curIO = IOList[0].ID;
            curTrack = IOList[0].track;
            finishTime += seekTime;
            turnaround += finishTime - IOList[0].timestamp;
            IOflag = 1;
            //cout << finishTime << " " << seekTime << endl;
        }
    }
    
    print_request();
    print_sum();
}


void IOScheduler::SSTF() {
    
    while (IOList.size() > 0) {
        checkAdd();
        checkFinish();
        vector<int>seekLength;
        
        if (readyPtr > -1) {
            for (int i = 0; i <= readyPtr; ++i) {
                seekLength.push_back(abs(IOList[i].track - curTrack));
            }
            
            int shortest = seekLength[0];
            int idx = 0;
            for (int i = 0; i < seekLength.size(); ++i) {
                if (shortest == seekLength[i]) {
                    if (IOList[idx].ID > IOList[i].ID) {
                        idx = i;
                    }
                }
                if (shortest > seekLength[i]) {
                    shortest = seekLength[i];
                    idx = i;
                }
            }
            IOTask temp = IOList.front();
            IOList.front() = IOList[idx];
            IOList[idx] = temp;
            
            if (IOList[0].timestamp < finishTime ) {
                issueTime = finishTime;
            }else{
                issueTime = IOList[0].timestamp;
            }
            tol_waitingTime += issueTime - IOList[0].timestamp;
            max_waittime = max(max_waittime, issueTime-IOList[0].timestamp);
            
            if (verbose) {
                printf("%d:%6d issue %d %d\n", issueTime, IOList[0].ID, IOList[0].track, curTrack);
            }
            
            seekTime = abs(IOList[0].track - curTrack);
            tol_seekTime += seekTime;
            curIO = IOList[0].ID;
            curTrack = IOList[0].track;
            finishTime += seekTime;
            turnaround += finishTime - IOList[0].timestamp;
            IOflag = 1;
        }
    }
    
    
    print_request();
    print_sum();
    
}



void IOScheduler::SCAN() {

    while (IOList.size() > 0) {
        checkAdd();
        checkFinish();
        
        if (readyPtr > -1) {
            int idx = 0;
            
            while (true) {
                if(upflag) {
                    vector<int>upper;
                    for (int i = 0; i <= readyPtr; ++i) {
                        
                        if (IOList[i].track >= curTrack) {
                            
                            upper.push_back(i);
                        }
                    }
                    
                    if (upper.size() == 0) {
                        upflag = 0;
                        
                    } else {
                        
                        int smallest = IOList[upper[0]].track;
                        idx = upper[0];
                        for (int i = 0; i < upper.size(); ++i) {
                            if (smallest == IOList[upper[i]].track) {
                                if (IOList[idx].ID > IOList[upper[i]].ID) {
                                    idx = upper[i];
                                }
                            }
                            
                            if (smallest > IOList[upper[i]].track) {
                                smallest = IOList[upper[i]].track;
                                idx = upper[i];
                            }
                        }
                        break;
                        
                    }
                }
                
                if (!upflag) {
                    
                    vector<int>lower;
                    for (int i = 0; i <= readyPtr; ++i) {
                        if (IOList[i].track <= curTrack) {
                            lower.push_back(i);
                        }
                    }
                    
                    if (lower.size() == 0) {
                        upflag = 1;
                        
                    } else {
                        
                        int biggest = IOList[lower[0]].track;
                        idx = lower[0];
                        for (int i = 0; i < lower.size(); ++i) {
                            if (biggest == IOList[lower[i]].track) {
                                if (IOList[idx].ID > IOList[lower[i]].ID) {
                                    idx = lower[i];
                                }
                            }
                            
                            if (biggest < IOList[lower[i]].track) {
                                biggest = IOList[lower[i]].track;
                                idx = lower[i];
                            }
                        }
                        break;
                    }
                    
                }
            }
            
            
            
            
            
            //cout << idx << endl;
            IOTask temp = IOList.front();
            IOList.front() = IOList[idx];
            IOList[idx] = temp;
            
            
            if (IOList[0].timestamp < finishTime ) {
                issueTime = finishTime;
            }else{
                issueTime = IOList[0].timestamp;
            }
            tol_waitingTime += issueTime - IOList[0].timestamp;
            max_waittime = max(max_waittime, issueTime-IOList[0].timestamp);
            
            
            if (verbose) {
                printf("%d:%6d issue %d %d\n", issueTime, IOList[0].ID, IOList[0].track, curTrack);
            }
            
            seekTime = abs(IOList[0].track - curTrack);
            tol_seekTime += seekTime;
            curIO = IOList[0].ID;
            curTrack = IOList[0].track;
            finishTime += seekTime;
            turnaround += finishTime - IOList[0].timestamp;
            IOflag = 1;
        }
    }
    
    print_request();
    print_sum();

}



void IOScheduler::CSCAN() {

    while (IOList.size() > 0) {
        checkAdd();
        checkFinish();
        
        if (readyPtr > -1) {
            int idx = 0;
            
            vector<int>upper;
            for (int i = 0; i <= readyPtr; ++i) {
                if (IOList[i].track >= curTrack) {
                    
                    upper.push_back(i);
                    
                }
            }
            
            if (upper.size() == 0) {
                for (int i = 0; i <= readyPtr; ++i) {
                    upper.push_back(i);
                    
                }
            }
            int smallest = IOList[upper[0]].track;
            idx = upper[0];
            for (int i = 0; i < upper.size(); ++i) {
                if (smallest == IOList[upper[i]].track) {
                    if (IOList[idx].ID > IOList[upper[i]].ID)
                        idx = upper[i];
                    
                }
                
                if (smallest > IOList[upper[i]].track) {
                    smallest = IOList[upper[i]].track;
                    idx = upper[i];
                }
                
            }
           
            //cout << idx << endl;
            IOTask temp = IOList.front();
            IOList.front() = IOList[idx];
            IOList[idx] = temp;
            
            
            if (IOList[0].timestamp < finishTime ) {
                issueTime = finishTime;
            }else{
                issueTime = IOList[0].timestamp;
            }
            tol_waitingTime += issueTime - IOList[0].timestamp;
            max_waittime = max(max_waittime, issueTime-IOList[0].timestamp);
            
            if (verbose) {
                printf("%d:%6d issue %d %d\n", issueTime, IOList[0].ID, IOList[0].track, curTrack);
            }
            
            seekTime = abs(IOList[0].track - curTrack);
            tol_seekTime += seekTime;
            curIO = IOList[0].ID;
            curTrack = IOList[0].track;
            finishTime += seekTime;
            turnaround += finishTime - IOList[0].timestamp;
            IOflag = 1;
        }
    }
    
    print_request();
    print_sum();
    
}




void IOScheduler::FSCAN() {
    
    while (IOList.size() > 0) {
        checkAdd();
        checkFinish();
        
        if (curLast == -1) {
            curLast = readyPtr;
            upflag = 1;
        }
        
        //cout << readyPtr << " "<<curLast  << endl;
        
        if (curLast > -1) {
            int idx = 0;
            while (true) {
                if(upflag) {
                    vector<int>upper;
                    for (int i = 0; i <= curLast; ++i) {
                        
                        if (IOList[i].track >= curTrack) {
                            
                            upper.push_back(i);
                        }
                    }
                    
                    if (upper.size() == 0) {
                        upflag = 0;
                        
                    } else {
                        
                        int smallest = IOList[upper[0]].track;
                        idx = upper[0];
                        for (int i = 0; i < upper.size(); ++i) {
                            if (smallest == IOList[upper[i]].track) {
                                if (IOList[idx].ID > IOList[upper[i]].ID) {
                                    idx = upper[i];
                                }
                            }
                            
                            if (smallest > IOList[upper[i]].track) {
                                smallest = IOList[upper[i]].track;
                                idx = upper[i];
                            }
                        }
                        break;
                        
                    }
                }
                
                if (!upflag) {
                    
                    vector<int>lower;
                    for (int i = 0; i <= curLast; ++i) {
                        if (IOList[i].track <= curTrack) {
                            lower.push_back(i);
                        }
                    }
                    
                    if (lower.size() == 0) {
                        upflag = 1;
                        
                    } else {
                        
                        int biggest = IOList[lower[0]].track;
                        idx = lower[0];
                        for (int i = 0; i < lower.size(); ++i) {
                            if (biggest == IOList[lower[i]].track) {
                                if (IOList[idx].ID > IOList[lower[i]].ID) {
                                    idx = lower[i];
                                }
                            }
                            
                            if (biggest < IOList[lower[i]].track) {
                                biggest = IOList[lower[i]].track;
                                idx = lower[i];
                            }
                        }
                        break;
                    }
                    
                }
            }
           
            IOTask temp = IOList.front();
            IOList.front() = IOList[idx];
            IOList[idx] = temp;
            
            
            if (IOList[0].timestamp < finishTime ) {
                issueTime = finishTime;
            }else{
                issueTime = IOList[0].timestamp;
            }
            tol_waitingTime += issueTime - IOList[0].timestamp;
            max_waittime = max(max_waittime, issueTime-IOList[0].timestamp);
            
            
            if (verbose) {
                printf("%d:%6d issue %d %d\n", issueTime, IOList[0].ID, IOList[0].track, curTrack);
            }
            
            seekTime = abs(IOList[0].track - curTrack);
            tol_seekTime += seekTime;
            curIO = IOList[0].ID;
            curTrack = IOList[0].track;
            finishTime += seekTime;
            turnaround += finishTime - IOList[0].timestamp;
            IOflag = 1;
        }
    }
    
    print_request();
    print_sum();
    
    
}


void IOScheduler::print_request(){
    
    if (verbose) {
        cout << "IOREQS INFO" << endl;
        
        for (int i = 0; i < IOFinish.size(); ++i) {
            printf("%5d:%6d%6d%6d\n", IOFinish[i].ID, IOFinish[i].timestamp, IOFinish[i].diskStart, IOFinish[i].diskEnd);
        }
    }
}


void IOScheduler::print_sum(){
    total_time = finishTime;
    tol_movement = tol_seekTime;
    avg_waittime = ((double)tol_waitingTime) / IOFinish.size();
    avg_turnaround = ((double)turnaround) / IOFinish.size();
    //cout << submitTime << endl;
    
    printf("SUM: %d %d %.2lf %.2lf %d\n", total_time, tol_movement, avg_turnaround, avg_waittime, max_waittime);


}

void IOScheduler::Run(string infile, string algo, bool _v){
    
    verbose = _v;
    string line;
    ifstream read(infile);
    
    while (getline(read,line)) {
        if (line[0] != '#') {
            int stamp, trk;
            stringstream iss(line);
            iss >> stamp >> trk;
            IOList.push_back(*new IOTask(IOIdx, stamp, trk));
            IOIdx++;
            
        }
    }
    
    /*for (int i = 0; i < IOList.size(); ++i) {
     cout << IOList[i].ID << " " << IOList[i].timestamp<< " " << IOList[i].track<< endl;
     }*/
    
    
    getFirstIO();
    if (algo == "i")    FIFO();
    if (algo == "j")    SSTF();
    if (algo == "s")    SCAN();
    if (algo == "c")    CSCAN();
    if (algo == "f"){
        algorithm = "f";
        FSCAN();
        
    }
}


int main(int argc, char * argv[]) {
    
    bool v = 0;
    string s = "";
    int c;
    while ((c = getopt (argc, argv, "vs:")) != -1) {
        switch (c) {
            case 'v':
            v = 1;
            break;
            
            case 's':
            s = optarg;
            break;
            
        }
        
    }
    
    IOScheduler Simulator;
    Simulator.Run(argv[optind], s, v);
    
}
