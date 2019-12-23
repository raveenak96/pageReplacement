
#include <stdio.h>
#include <stdlib.h>

#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) {

    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */
    
    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];

    /* Local vars */
    int proctmp;
    int pagetmp;
    int proc;
    int pc;
    int page;
    int lru;
    int l_timestamp;

    /* initialize static vars on first run */
    if(!initialized){
	for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
	    for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
		  timestamps[proctmp][pagetmp] = 0;
	    }
	}
	initialized = 1;
    }

    /* TODO: Implement LRU Paging */
    for(proc=0; proc<MAXPROCESSES; proc++) {
        pc = q[proc].pc;
        page = pc/PAGESIZE;

        if(q[proc].active) {
            //update the timestamp for the process and page
            timestamps[proc][page] = tick;


        if(!q[proc].pages[page]) {
            //try to page it in
            if(!pagein(proc,page)) {
                //find the least recently used page and page it out
                l_timestamp = -1;
                for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++) {

                    if(q[proc].pages[pagetmp] && (tick-timestamps[proc][pagetmp]) > l_timestamp) {
                        
                        l_timestamp = tick-timestamps[proc][pagetmp];
                        lru = pagetmp;
                    }
                }
                pageout(proc,lru);
            }
        } 
        }   
    }

    /* advance time for next pageit iteration */
    tick++;
}
