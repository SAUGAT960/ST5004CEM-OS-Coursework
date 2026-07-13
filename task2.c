
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FRAMES 4
#define PAGES 8
#define REF_LEN 12

typedef struct { int page, valid, load, last; } Frame;
Frame mem[FRAMES];
int pt[PAGES];
int faults = 0, hits = 0, t = 0;

void init_mmu() {
    for(int i=0; i<FRAMES; i++) { mem[i].page=-1; mem[i].valid=0; mem[i].load=-1; mem[i].last=-1; }
    for(int i=0; i<PAGES; i++) pt[i] = -1;
    faults = 0; hits = 0; t = 0;
}

void access_page(int p, int algo) {
    t++;
    printf("\n[ACCESS t=%d] Requesting page %d\n", t, p);
    if(pt[p] >= 0) {
        hits++;
        mem[pt[p]].last = t;
        printf("[%s] PAGE HIT: Page %d in frame %d\n", algo==0?"FIFO":"LRU", p, pt[p]);
        return;
    }
    faults++;
    printf("[%s] PAGE FAULT: Page %d not in memory\n", algo==0?"FIFO":"LRU", p);

    int f = -1;
    for(int i=0; i<FRAMES; i++) if(!mem[i].valid) { f = i; break; }

    if(f < 0) {
        f = 0;
        if(algo == 0) {
            int oldest = mem[0].load;
            for(int i=1; i<FRAMES; i++) if(mem[i].load < oldest) { oldest = mem[i].load; f = i; }
            printf("[FIFO] Replacing page %d in frame %d with page %d\n", mem[f].page, f, p);
        } else {
            int old = mem[0].last;
            for(int i=1; i<FRAMES; i++) if(mem[i].last < old) { old = mem[i].last; f = i; }
            printf("[LRU] Replacing page %d in frame %d with page %d\n", mem[f].page, f, p);
        }
        pt[mem[f].page] = -1;
    } else {
        printf("[%s] Loading page %d into free frame %d\n", algo==0?"FIFO":"LRU", p, f);
    }
    mem[f].page = p; mem[f].valid = 1; mem[f].load = t; mem[f].last = t; pt[p] = f;

    printf("  Memory State: ");
    for(int i=0; i<FRAMES; i++) {
        if(mem[i].valid) printf("[F%d:P%d] ", i, mem[i].page);
        else printf("[F%d:EMPTY] ", i);
    }
    printf("\n");
}

void print_stats(const char* algo) {
    int total = hits + faults;
    printf("\n============================================================\n");
    printf("  %s STATISTICS\n", algo);
    printf("============================================================\n");
    printf("  Total Accesses: %d\n", total);
    printf("  Page Hits:      %d (%.2f%%)\n", hits, 100.0*hits/total);
    printf("  Page Faults:    %d (%.2f%%)\n", faults, 100.0*faults/total);
    printf("  Hit Ratio:      %.4f\n", (double)hits/total);
    printf("  Miss Ratio:     %.4f\n", (double)faults/total);
    printf("============================================================\n");
}

int main() {
    int ref[REF_LEN] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};

    printf("============================================================\n");
    printf("  ST5004CEM - Task 2: Memory Management Simulation\n");
    printf("============================================================\n");
    printf("  Virtual Pages: %d | Physical Frames: %d\n", PAGES, FRAMES);
    printf("  Reference String: ");
    for(int i=0; i<REF_LEN; i++) printf("%d ", ref[i]);
    printf("\n\n");

    printf("============================================================\n");
    printf("           FIFO SIMULATION\n");
    printf("============================================================\n");
    init_mmu();
    for(int i=0; i<REF_LEN; i++) access_page(ref[i], 0);
    print_stats("FIFO");

    printf("\n\n============================================================\n");
    printf("           LRU SIMULATION\n");
    printf("============================================================\n");
    init_mmu();
    for(int i=0; i<REF_LEN; i++) access_page(ref[i], 1);
    print_stats("LRU");

    return 0;
}
