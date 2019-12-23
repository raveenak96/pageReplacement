

#include <stdio.h>
#include <stdlib.h>

#include "simulator.h"

//used to store page history to create our pattern
typedef struct _page_history {

    int nextPage[MAXPROCPAGES];
    int numPages;
} PageHistory;

//holds the pattern of pages for each of our processes
typedef struct _page_pattern {
    PageHistory pageHistory[MAXPROCPAGES];
    int prevPage;
} PagePattern;

//created a struct for a process, page pair for easier access
typedef struct _proc_page {
    int proc;
    int page;
} ProcPage;

static PagePattern pattern[MAXPROCESSES];

//use our knowledge to decide which pages to page out/in
void performPaging(ProcPage* needed, ProcPage* neededSoon, ProcPage* remove, int numNeeded, int numNeededSoon, int numRemove,Pentry* q) {

    int needIdx,needSIdx,curProc,curPage;
    int pagedOut = 0;
    //attempt to page in the pages needed now, otherwise page out one of our removable pages
    for(needIdx=0;needIdx<numNeeded;needIdx++) {
        curProc = needed[needIdx].proc;
        curPage = needed[needIdx].page;
        if(!q[curProc].pages[curPage]) {
            if(!pagein(curProc,curPage)) {

                if(pagedOut >= numRemove) {
                    break;
                } else {
                    pageout(remove[pagedOut].proc,remove[pagedOut].page);
                    pagedOut++;
                }
            }
            
        }
    }
    //attempt to page in pages we will need soon, otherwise page out one of our removable ones
    for(needSIdx=0;needSIdx<numNeededSoon;needSIdx++) {
        curProc = neededSoon[needSIdx].proc;
        curPage = neededSoon[needSIdx].page;
        if(!q[curProc].pages[curPage]) {
            if(!pagein(curProc,curPage)) {
                if(pagedOut >= numRemove) {
                    break;
                } else {
                
                    pageout(remove[pagedOut].proc,remove[pagedOut].page);
                    pagedOut++;
                } 
            }

        }
    }

}

void pageit(Pentry q[MAXPROCESSES]) {

    /* This file contains the stub for a predictive pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time

    /* Local vars */
    int proc;
    int page;
    int pc;
    PageHistory procPgHist;
    /* initialize static vars on first run */
    if(!initialized){
	/* Init complex static vars here */
        for(proc=0;proc < MAXPROCESSES;proc++) {
            for(page=0;page<MAXPROCPAGES;page++) {
                pattern[proc].pageHistory[page].numPages = 0;
            }
        }
	   initialized = 1;
    }

    //pages we don't need and able to swap out
    int numRemove = 0;
    ProcPage remove[MAXPROCPAGES*MAXPROCESSES];
    

    //pages that will be needed soon
    int numNeededSoon = 0;
    ProcPage neededSoon[MAXPROCPAGES*MAXPROCESSES];

    //update the page pattern/history
    //for each process add that current page comes after previous page
    int prevPage,presPage,tmppage;
    for(proc=0;proc<MAXPROCESSES;proc++) {

        prevPage = pattern[proc].prevPage;
        presPage = q[proc].pc / PAGESIZE;
        if(presPage != prevPage) {
            PageHistory* next = &(pattern[proc].pageHistory[prevPage]);
            int added = 0;
            for(tmppage=0;tmppage < next->numPages;tmppage++) {
                if(next->nextPage[tmppage]==presPage) {
                    added = 1;
                    break;
                }
            }
            if(!added) {
                next->nextPage[next->numPages] = presPage;
                (next->numPages)++;
            }
            
        }
        pattern[proc].prevPage = presPage;
    }

    //pages needed now
    int numNeeded = 0;
    ProcPage needed[MAXPROCESSES];
    
    //updating the needed,neededSoon, and remove data structures
    int nSIndex,tmppage2;
    for(proc=0;proc<MAXPROCESSES;proc++) {
        pc = q[proc].pc;
        page = pc/PAGESIZE;

        if(q[proc].active) {
            //update which pages are needed now (the current page)
            needed[numNeeded].proc = proc;
            needed[numNeeded].page = page;
            numNeeded++;
            //update which ones will be needed soon
            procPgHist = pattern[proc].pageHistory[page];
            for(nSIndex=0;nSIndex < procPgHist.numPages;nSIndex++) {
                neededSoon[numNeededSoon].proc = proc;
                neededSoon[numNeededSoon].page = procPgHist.nextPage[nSIndex];
                numNeededSoon++;
            }
            //set our pages we can potentially remove (not needed now or soon)
            for(tmppage2=0;tmppage2<q[proc].npages;tmppage2++) {

                if(q[proc].pages[tmppage2]) {

                    //make sure pages aren't currently needed
                    int swappable = 1;
                    int neededIdx,neededSIdx;
                    for(neededIdx= numNeeded-1;neededIdx>=0;neededIdx--) {
                        if(needed[neededIdx].proc==proc) {
                            if(needed[neededIdx].page==tmppage2) {
                                swappable = 0;
                            }
                        } else {
                            break;
                        }
                    }

                    //make sure pages won't be used soon
                    for(neededSIdx=numNeededSoon-1;neededSIdx>=0;neededSIdx--) {
                        if(neededSoon[neededSIdx].proc==proc) {
                            if(swappable && neededSoon[neededSIdx].page==tmppage2) {
                                swappable = 0;
                            }
                        } else {
                            break;
                        }
                    }

                    //pages not needed now or soon 
                    if(swappable) {
                        remove[numRemove].page = tmppage2;
                        remove[numRemove].proc = proc;
                        numRemove++;
                    }

                }
            }


        }
    }
    performPaging(needed,neededSoon,remove,numNeeded,numNeededSoon,numRemove,q);
    /* advance time for next pageit iteration */
    tick++;
}
