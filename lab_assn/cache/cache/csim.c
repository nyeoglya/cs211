// include
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "cachelab.h"

// data structure
typedef struct {
    int valid;
    unsigned long tag;
    unsigned long lru; // LRU timestamp
} Line;

typedef struct {
    Line *lines;
} Set;

// global variables
char verbose = 0;
unsigned s = 0;
unsigned E = 0;
unsigned b = 0;
char* filePath = "";

Set *cache = NULL; // cache
unsigned S = 0; // set count(=2^s)
unsigned long globalTime = 0; // global time for LRU

int hitCount = 0;
int missCount = 0;
int evicCount = 0;

// function declaration
void printUsage(char *argv0);
char argParser(int argc, char *argv[]);
void initCache();
void freeCache();
void accessData(unsigned long address);
void fileAnalyze(FILE *fp);

// print usage texts. copy of the csim-ref help text.
void printUsage(char *argv0) {
    printf("%s: Missing required command line argument\n", argv0);
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv0);
    puts("Options:");
    puts("  -v\t\tOptional verbose flag.");
    puts("  -h\t\tPrint this help message.");
    puts("  -s <num>\tNumber of set index bits.");
    puts("  -E <num>\tNumber of lines per set.");
    puts("  -b <num>\tNumber of block offset bits.");
    puts("  -t <file>\tTrace file.");
    puts("");
    puts("Examples:");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv0);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv0);
}

// argument parser. use getopt()
char argParser(int argc, char *argv[]) {
    char flag = 0x0; // flag to check if all of the necessary params(s,E,b,t) are in argv
    int opt; // temp value for current option
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
            case 'v': // verbose flag
                verbose = 1;
                break;
            case 's': // s flag
                sscanf(optarg, "%u", &s);
                flag = flag | 0x1;
                break;
            case 'E': // E flag
                sscanf(optarg, "%u", &E);
                flag = flag | 0x2;
                break;
            case 'b': // b flag
                sscanf(optarg, "%u", &b);
                flag = flag | 0x4;
                break;
            case 't': // t flag
                filePath = optarg;
                flag = flag | 0x8;
                break;
            case 'h': // for the other cases
            case '?':
            default:
                return 1;
        }
    }
    // check if s, E, b, t are all setted up
    return flag != 0xF;
}

// cache simulation initialization
void initCache() {
    S = 1u << s; // set count

    cache = (Set *)malloc(S * sizeof(Set));
    if (!cache) { // malloc failure handling
        fprintf(stderr, "Failed to allocate cache sets\n");
        exit(1);
    }

    for (unsigned i = 0; i < S; i++) { // allocate line to each set
        cache[i].lines = (Line *)malloc(E * sizeof(Line));
        if (!cache[i].lines) {
            fprintf(stderr, "Failed to allocate cache lines\n");
            exit(1);
        }
        for (unsigned j = 0; j < E; j++) {
            cache[i].lines[j].valid = 0;
            cache[i].lines[j].tag = 0;
            cache[i].lines[j].lru = 0;
        }
    }
}

// free global var. cache
void freeCache() {
    if (!cache) return;
    for (unsigned i = 0; i < S; i++) {
        free(cache[i].lines);
    }
    free(cache);
    cache = NULL;
}

// simulating cache data access
void accessData(unsigned long address) {
    unsigned long setIndex = (address >> b) & ((1UL << s) - 1UL);
    unsigned long tag = address >> (s + b);

    Set *set = &cache[setIndex];
    Line *lines = set->lines;

    int hitLine = -1;
    int emptyLine = -1;
    unsigned long minLRU = -1;
    int lruLine = -1;

    // check if hit in a certain set + find empty line & LRU line
    for (unsigned i = 0; i < E; i++) {
        if (lines[i].valid && lines[i].tag == tag) {
            hitLine = (int)i;
        }
        if (!lines[i].valid && emptyLine == -1) {
            emptyLine = (int)i;
        }
        if (lines[i].valid && lines[i].lru < minLRU) {
            minLRU = lines[i].lru;
            lruLine = (int)i;
        }
    }

    if (hitLine != -1) { // hit occur
        hitCount++;
        lines[hitLine].lru = globalTime; // update lru
        if (verbose) {
            printf(" hit");
        }
        return;
    }

    missCount++; // miss occur
    if (verbose) {
        printf(" miss");
    }

    if (emptyLine != -1) { // load data on a empty line
        lines[emptyLine].valid = 1;
        lines[emptyLine].tag = tag;
        lines[emptyLine].lru = globalTime;
    } else { // need evictions (no empty line)
        evicCount++;
        if (verbose) {
            printf(" eviction");
        }
        lines[lruLine].valid = 1;
        lines[lruLine].tag = tag;
        lines[lruLine].lru = globalTime;
    }
}

// analyze trace data from filePath
void fileAnalyze(FILE *fp) {
    char tempLine[100];

    while (fgets(tempLine, sizeof(tempLine), fp) != NULL) {
        char op;
        unsigned long address;
        unsigned size;

        globalTime++;

        if (sscanf(tempLine, " %c %lx,%u", &op, &address, &size) != 3) continue; // " %c" is in purpose of strip empty space may occur at the front
        if (op == 'I') continue;
        if (verbose) printf("%c %lx,%u", op, address, size); // verbose -> print line

        switch (op) {
            case 'L': case 'S':
                accessData(address);
                break;
            case 'M':
                accessData(address);
                accessData(address);
                break;
            default:
                break;
        }

        if (verbose) {
            printf("\n");
        }
    }
}

// main function
int main(int argc, char *argv[]) {
    if (argParser(argc, argv)) {
        printUsage(argv[0]);
        return 0;
    }

    initCache();

    FILE *fp = fopen(filePath, "r"); // read file from filePath

    if (!fp) { // error handling
        perror("error on reading file");
        freeCache();
        exit(-1);
    }

    fileAnalyze(fp); // file analysis

    // for safe program termination
    fclose(fp);
    freeCache();

    printSummary(hitCount, missCount, evicCount); // grading
    return 0;
}
