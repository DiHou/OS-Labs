//
//  main.cpp
//  MMU
//
//  Created by Hou Di on 11/14/15.
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

/******define pte bitfield******/
struct PTE {
    unsigned int present:1;
    unsigned int modified:1;
    unsigned int referenced:1;
    unsigned int pagedout:1;
    unsigned int findex:6; //support 64 physical frames
    
    PTE();
};

PTE::PTE(){
    present = 0;
    modified = 0;
    referenced = 0;
    pagedout = 0;
    findex = 0;
}


/******define MMU class******/

class MMU {
    
public:
    int frameTaken;
    int instrCount;
    int randCount;
    int ofs;
    int count;
    int pageReplaces;
    int unmaps;
    int maps;
    int pageIns;
    int pageOuts;
    int pageZeros;
    long long unsigned totalcost;
    string pageAlgo;
    
    bool spaceflag;
    
    vector<int> rVal;
    
    MMU();
    
    void MAP(string input, string rfile, bool Oflag, bool Pflag, bool Fflag, bool Sflag, int frameNum, string algo);
    void getAlgoType(string algo);
    void getRand(string randfile);
    int RANDOMout(vector<int>& FT, int _vindex);
    int P_CLOCKout(vector<int>& FT, vector<PTE>& PT, int _vindex);
    int V_CLOCKout(vector<int>& FT, vector<PTE>& PT, int _vindex);
    int NRUout(vector<int>& FT, vector<PTE>& PT, int _vindex);
    void S_Print();
    
};


MMU::MMU(){
    frameTaken = 0;
    instrCount = 0;
    randCount = 0;
    ofs = 0;
    spaceflag = 1;
    count=0;
    pageReplaces = 0;
    
    unmaps = 0;
    maps = 0;
    pageIns = 0;
    pageOuts = 0;
    pageZeros = 0;
    totalcost = 0;
    
}


/******define algorithm functions******/


int FIFOout(vector<int>& FT) {
    int outVIdx = FT.front();
    FT.erase(FT.begin());
    return outVIdx;
}

void FIFOin(vector<int>& FT, int _vindex) {
    FT.push_back(_vindex);
}



int SCout(vector<int>& FT, vector<PTE>& PT) {
    int outVIdx = 0;
    int i = 0;
    
    while (true) {
        if (PT[FT.front()].referenced == 1) {
            PT[FT.front()].referenced = 0; //clear reference bit to zero
            int temp = FT.front();
            FT.erase(FT.begin());
            FT.push_back(temp);
            
        }
        
        else {
            outVIdx = FT.front();
            FT.erase(FT.begin());
            break;
        }
        
        ++i;
        
    }
    
    return outVIdx;
}

void SCin(vector<int>& FT, int _vindex) {
    FT.push_back(_vindex);
}



int MMU:: RANDOMout(vector<int>& FT, int _vindex) {
    if (ofs == randCount) {
        ofs = 0;
    }
    
    int outFidx = rVal[ofs] % FT.size();
    int outVidx = FT[outFidx];
    FT[outFidx] = _vindex;
    ofs++;
    return outVidx;
    
}


int LRUout(vector<int>& FT) {
    int outVIdx = FT.front();
    FT.erase(FT.begin());
    return outVIdx;
    
}

void LRUin(vector<int>& FT, int _vindex) {
    FT.push_back(_vindex);
}

int MMU::P_CLOCKout(vector<int>& FT, vector<PTE>& PT, int _vindex) {
    int outVIdx = 0;
    
    while (true) {
        if (PT[FT[count]].referenced == 1) {
            PT[FT[count]].referenced = 0; //clear reference bit to zero
            count++;
            if (count == FT.size()) {
                count = 0;
            }
            
        }
        
        else {
            outVIdx = FT[count];
            FT[count] = _vindex;
            count++;
            if (count == FT.size()) {
                count = 0;
            }
            break;
        }
        
    }
    
    return outVIdx;
}


int MMU::V_CLOCKout(vector<int>& FT, vector<PTE>& PT, int _vindex) {
    int outVIdx = 0;
    
    while (true) {
        if (PT[count].present == 1){
            if (PT[count].referenced == 1) {
                PT[count].referenced = 0;
                
                count++;
                if (count == PT.size())     count = 0;
            }
            
            else {
                outVIdx = count;
                auto pos = find(FT.begin(), FT.end(), outVIdx) - FT.begin();
                FT[pos] = _vindex;
                
                count++;
                if (count == PT.size())     count = 0;
                break;
            }
        }
        
        else {
            count++;
            if (count == PT.size())     count = 0;
        }
    }
    
    return outVIdx;
}



int MMU::NRUout(vector<int>& FT, vector<PTE>& PT, int _vindex) {
    int outVIdx = 0;
    
    //initialize a two dimentional vector with 4 classes
    vector<int>rank[4];
    for (int i = 0; i < PT.size(); ++i) {
        if (PT[i].present == 1)
            rank[PT[i].referenced * 2 + PT[i].modified].push_back(i);
    }
    
    //start from class 0, find the vindex in random
    for (int i = 0; i < 4; ++i) {
        if (rank[i].size() > 0) {
            if (ofs == randCount)   ofs = 0;
            outVIdx = rank[i][rVal[ofs] % rank[i].size()];
            break;
        }
    }
    ofs++;
    
    auto pos = find(FT.begin(), FT.end(), outVIdx) - FT.begin();
    FT[pos] = _vindex;
    
    pageReplaces++;
    if (pageReplaces == 10) {
        pageReplaces = 0;
        for (int i = 0; i < PT.size(); ++i)
            PT[i].referenced = 0;
        
    }
    
    return outVIdx;
    
}


int P_AGING(vector<int>& FT, vector<PTE>& PT, vector<unsigned int>& p_age, int _vindex){
    
    int outVIdx = 0;
    for (int i = 0; i < p_age.size(); ++i) {
        unsigned int r = PT[FT[i]].referenced;
        p_age[i] = ((p_age[i]>>1) | (r<<31));
        PT[FT[i]].referenced = 0;
        
    }
    
    int pick = 0;
    for (int i = 0; i < p_age.size(); ++i) {
        if (p_age[pick] > p_age[i]) {
            pick = i;
            
        }
    }
    p_age[pick] = 0;
    outVIdx = FT[pick];
    FT[pick] = _vindex;
    
    return outVIdx;
}



int V_AGING(vector<int>& FT, vector<PTE>& PT, vector<unsigned int>& v_age, int _vindex){
    
    int outVIdx = 0;
    
    for (int i = 0; i < PT.size(); ++i) {
        unsigned int r = PT[i].referenced;
        v_age[i] = ((v_age[i]>>1) | (r<<31));
        PT[i].referenced = 0;
        
    }
    
    int pick = 0;
    //set pick equal to the first valid element
    for (int i = 0; i < v_age.size(); ++i) {
        if (PT[i].present) {
            pick = i;
            break;
        }
    }
    
    for (int i = 0; i < v_age.size(); ++i) {
        if (PT[i].present) {
            if (v_age[pick] > v_age[i])
                pick = i;
        }
    }
    v_age[pick] = 0;
    outVIdx = pick;
    auto pos = find(FT.begin(), FT.end(), outVIdx) - FT.begin();
    FT[pos] = _vindex;
    
    return outVIdx;
}


/**************************/
/**************************/


void MMU::getAlgoType(string algo) {
    if (algo == "f")    pageAlgo = algo;
    if (algo == "s")    pageAlgo = algo;
    if (algo == "N")    pageAlgo = algo;
    if (algo == "l")    pageAlgo = algo;
    if (algo == "r")    pageAlgo = algo;
    if (algo == "c")    pageAlgo = algo;
    if (algo == "X")    pageAlgo = algo;
    if (algo == "a")    pageAlgo = algo;
    if (algo == "Y")    pageAlgo = algo;
}


void MMU:: getRand(string randfile) {
    
    ifstream readRand(randfile);
    readRand >> randCount; // first NO is random total number
    int r = 0;
    for(int i = 0; i < randCount; ++i) {
        readRand >> r;
        rVal.push_back(r);
    }
    
    readRand.close();
}



void P_Print(vector<PTE>& PT) {
    
    for (int i = 0; i < PT.size(); ++i) {
        
        
        if (PT[i].present) {
            cout << i << ":";
            
            if (PT[i].referenced)   cout << "R";
            else                    cout << "-";
            
            if (PT[i].modified)     cout << "M";
            else                    cout << "-";
            
            if (PT[i].pagedout)     cout << "S ";
            else                    cout << "- ";
        }
        
        else {
            if (PT[i].pagedout)     cout << "# ";
            else                    cout << "* ";
            
        }
        
    }
    
    cout << endl;
    
}


void F_Print(vector<int>& FT, vector<PTE>& PT, int FTaken, string algo) {
    
    for (int i = 0; i < FT.size(); ++i) {
        
        if (algo == "f" || algo == "l" || algo == "s") {
            for (int j = 0; j < PT.size(); ++j) {
                if (PT[j].findex == i && PT[j].present) {
                    cout << j << " ";
                    break;
                }
            }
        }
        
        else {
            if (FT[i] != -1) {
                cout << FT[i] << " ";
            }
            
        }
        
    }
    
    for (int i = FTaken; i < FT.size(); ++i) {
        cout << "* ";
    }
    
    cout << endl;
    
}


void MMU::S_Print() {
    
    
    totalcost = ((long long unsigned)(maps + unmaps)) * 400 + ((long long unsigned)(pageIns + pageOuts)) * 3000 + ((long long unsigned)(pageZeros)) * 150 + ((long long unsigned)(instrCount));
    
    
    printf("SUM %d U=%d M=%d I=%d O=%d Z=%d ===> %llu\n", instrCount, unmaps, maps, pageIns, pageOuts, pageZeros, totalcost);
    
}



void MMU::MAP(string input, string rfile, bool Oflag, bool Pflag, bool Fflag, bool Sflag, int frameNum, string algo) {
    
    getAlgoType(algo);
    getRand(rfile);
    vector<PTE>pageTable(64);
    //cout<<pageTable.size()<<endl;
    vector<int>frameTable(frameNum, -1);
    vector<unsigned int>p_age(frameNum);
    vector<unsigned int>v_age(64);
    //cout << frameTable.size() << endl;
    // using getline: http://stackoverflow.com/questions/7868936/read-file-line-by-line
    string line;
    ifstream read(input);
    
    while (getline(read, line) ) {
        if (line[0] != '#') {
            int mbit, vindex;
            
            //stringstream is defined in the <sstream> header
            //http://stackoverflow.com/questions/9611230/building-an-istringstream-with-a-string-temporary
            stringstream iss(line);
            iss >> mbit >> vindex;
            
            //check if vindex is bigger than 64
            
            if (vindex >= 64) {
                printf("vpage out of range\n");
                exit(0);
            }
            
            
            
            
            if (Oflag) {
                cout << "==> inst: " << line << endl;
            }
            
            if(pageTable[vindex].present == 0) {
                //at the beginning the frameTable is empty
                if (frameTaken < frameNum) {
                    frameTable[frameTaken] = vindex;
                    pageTable[vindex].findex = frameTaken;
                    pageTable[vindex].present = 1;
                    pageTable[vindex].referenced = 1;
                    
                    
                    //auto pos = find(frameTable.begin(), frameTable.end(), vindex) - frameTable.begin();
                    //cout <<pos<<endl;
                    
                    if (pageTable[vindex].modified == 1)
                        pageTable[vindex].modified = 1;
                    else
                        pageTable[vindex].modified = mbit;
                    
                    if (pageTable[vindex].pagedout == 0) {
                        if (Oflag){
                            printf("%d: ZERO       %2d\n", instrCount, frameTaken);
                            printf("%d: MAP    %2d  %2d\n", instrCount, vindex, frameTaken);
                        }
                        pageZeros++;
                        maps++;
                    }
                    
                    frameTaken++;
                    instrCount++;
                }
                
                //the frameTable is full, call paging algorithm
                else {
                    spaceflag = 0;
                    int outVIndex = 0;
                    //cout << pageTable[outVIndex].referenced<<endl;
                    if (pageAlgo == "f")    outVIndex = FIFOout(frameTable);
                    if (pageAlgo == "s")    outVIndex = SCout(frameTable, pageTable); //SecondChance
                    if (pageAlgo == "r")    outVIndex = RANDOMout(frameTable, vindex);
                    if (pageAlgo == "l")    outVIndex = LRUout(frameTable);
                    if (pageAlgo == "c")    outVIndex = P_CLOCKout(frameTable, pageTable, vindex);
                    if (pageAlgo == "X")    outVIndex = V_CLOCKout(frameTable, pageTable, vindex);
                    if (pageAlgo == "N")    outVIndex = NRUout(frameTable, pageTable, vindex);
                    if (pageAlgo == "a")    outVIndex = P_AGING(frameTable, pageTable, p_age, vindex);
                    if (pageAlgo == "Y")    outVIndex = V_AGING(frameTable, pageTable, v_age, vindex);
                    //cout << pageTable[outVIndex].findex<<endl;
                    
                    //if (pageAlgo == "N")    outVIndex = NRU(pageTable);
                    
                    /*****page out*****/
                    
                    pageTable[outVIndex].present = 0;
                    
                    if (Oflag)
                        printf("%d: UNMAP  %2d  %2d\n", instrCount, outVIndex, pageTable[outVIndex].findex);
                    unmaps++;
                    
                    if (pageTable[outVIndex].modified) {
                        pageTable[outVIndex].pagedout = 1;
                        
                        if (Oflag)
                            printf("%d: OUT    %2d  %2d\n", instrCount, outVIndex, pageTable[outVIndex].findex);
                        
                        pageTable[outVIndex].modified = 0;
                        pageOuts++;
                    }
                    
                    
                    /*****page in*****/
                    if (pageAlgo == "f")    FIFOin(frameTable, vindex);
                    if (pageAlgo == "s")    SCin(frameTable, vindex);
                    if (pageAlgo == "l")    LRUin(frameTable, vindex);
                    
                    
                    pageTable[vindex].present = 1;
                    pageTable[vindex].referenced = 1;
                    pageTable[vindex].findex = pageTable[outVIndex].findex;
                    
                    
                    if (pageTable[vindex].modified == 1){
                        pageTable[vindex].modified = 1;
                        //cout << (pageTable[vindex].modified?1:0) <<endl;
                    }
                    else{
                        pageTable[vindex].modified = mbit;
                        //cout << mbit << endl;
                    }
                    if (pageTable[vindex].pagedout == 0) {
                        if (Oflag) {
                            printf("%d: ZERO       %2d\n", instrCount, pageTable[outVIndex].findex);
                            printf("%d: MAP    %2d  %2d\n", instrCount, vindex, pageTable[outVIndex].findex);
                        }
                        maps++;
                        pageZeros++;
                        
                    }
                    else {
                        if (Oflag) {
                            printf("%d: IN     %2d  %2d\n", instrCount, vindex, pageTable[outVIndex].findex);
                            printf("%d: MAP    %2d  %2d\n", instrCount, vindex, pageTable[outVIndex].findex);
                        }
                        maps++;
                        pageIns++;
                    }
                    
                    
                    instrCount++;
                    
                }
                
            }
            
            //being referenced
            else {
                
                ////////*********LRU********////////
                if (pageAlgo == "l") {
                    auto pos = find(frameTable.begin(), frameTable.end(), vindex) - frameTable.begin();
                    //cout <<pos<<endl;
                    
                    frameTable.erase(frameTable.begin() + pos);
                    
                    if (spaceflag) {
                        frameTable.insert(frameTable.begin() + (frameTaken-1), vindex);
                        
                        //auto pos = find(frameTable.begin(), frameTable.end(), vindex) - frameTable.begin();
                        //cout <<pos<<endl;
                    }
                    else{
                        frameTable.push_back(vindex);
                        //cout <<vindex<<endl;
                    }
                }
                ////////********************////////
                
                
                if (pageTable[vindex].modified == 1)
                    pageTable[vindex].modified = 1;
                else
                    pageTable[vindex].modified = mbit;
                
                pageTable[vindex].referenced = 1;
                instrCount++;
                
            }
            
        }
        
    }
    
    
    if (Pflag)      P_Print(pageTable);
    if (Fflag)      F_Print(frameTable, pageTable, frameTaken, pageAlgo);
    if (Sflag)      S_Print();
    
}



// ./mmu [-a<algo>] [-o<options>] [–f<num_frames>] inputfile randomfile
// i.e. ./mmu –aX –o[OPFS] infile rfile selects the Clock Algorithm for Virtual Pages
// and creates output for operations, final page table content, final frame table content and summary line

int main(int argc, char * argv[]){
    string a = "l"; //paging algorithm, "l" by default
    string o = "";
    string f = "32"; //frame number, "32" by default
    int c;
    
    while ((c = getopt (argc, argv, "a:o:f:")) != -1) {
        switch (c){
            case 'a':
                a = optarg;
                break;
                
            case 'o':
                o = optarg;
                break;
                
            case 'f':
                f = optarg;
                break;
        }
    }
    
    bool Oflag = 0, Pflag = 0, Fflag = 0, Sflag = 0, pflag = 0, fflag = 0, aflag = 0;
    for (int i = 0; i < o.length(); ++i) {
        if (o[i] == 'O')    Oflag = 1;
        if (o[i] == 'P')    Pflag = 1;
        if (o[i] == 'F')    Fflag = 1;
        if (o[i] == 'S')    Sflag = 1;
        if (o[i] == 'p')    pflag = 1;
        if (o[i] == 'f')    fflag = 1;
        if (o[i] == 'a')    aflag = 1;
        
    }
    
    
    int fNum = atoi(f.c_str()); // convert str to int
    
    MMU Simulator;
    Simulator.MAP(argv[optind], argv[optind+1], Oflag, Pflag, Fflag, Sflag, fNum, a);
    
    return 0;
}
