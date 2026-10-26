#include <stdio.h>

/*
My constants and masks.
*/

#define VALID_BIT 0x10u
#define READ_BIT 0x08u
#define WRITE_BIT 0x04u
#define EXEC_BIT 0x02u
#define FRAME_MASK 0x7FFE0u
#define OFFSET_MASK 0x1Fu
#define ADDRESS_MASK 0x3FFFu
#define MAX_FRAMES 100
#define MAX_OPERATIONS 1000
#define PAGE_SIZE 32

//--------------STRUCTS--------------
/*
Represents a frame.
*/
typedef struct{
    unsigned int frameNumber;
    unsigned int entries[8];
} Frame;

/*
Represents an operation.
*/
typedef struct{
    char type;
    unsigned int address;
} Operation;

/*
Represents the MMU with the frame numbers, operation numbers and two arrays for frames and operations.
*/
typedef struct{
    int numFrames;
    int opNums;
    Frame frames[MAX_FRAMES];
    Operation operations[MAX_OPERATIONS];
} MMU;
//--------------END OF STRUCTS--------------

//--------------MMU FUNCTIONS--------------
/*
Cut the input file into data to put into the structs.
*/
int initMMU(MMU *mmu, FILE *fp){

    if(fscanf(fp, "%d", &mmu->numFrames) != 1){
        printf("Error reading frame numbers from file\n");
        fclose(fp);
        return -1;
    }

    for(int i = 0; i < mmu->numFrames; ++i){
        if(fscanf(fp, " frame: %u %u %u %u %u %u %u %u %u", 
            &mmu->frames[i].frameNumber,
            &mmu->frames[i].entries[0],
            &mmu->frames[i].entries[1],
            &mmu->frames[i].entries[2],
            &mmu->frames[i].entries[3],
            &mmu->frames[i].entries[4],
            &mmu->frames[i].entries[5],
            &mmu->frames[i].entries[6],
            &mmu->frames[i].entries[7]) != 9){
                printf("Error reading frame\n");
                fclose(fp);
                return 1;
            }
    }

    if(fscanf(fp, "%d", &mmu->opNums) != 1){
        printf("Error reading operation numbers from file\n");
        fclose(fp);
        return -1;
    }

    for(int i = 0; i < mmu->opNums; ++i){
        if(fscanf(fp, " %c %u", 
            &mmu->operations[i].type,
            &mmu->operations[i].address) != 2){
                printf("Error reading operation\n");
                fclose(fp);
                return 1;
            }
        }
    return 0;
}

/*
Helper function to find the matching frame value index.
*/
int findFrame(MMU *mmu, unsigned int frameNumber){
    for(int i = 0; i < mmu->numFrames; ++i){
        if(mmu->frames[i].frameNumber == frameNumber){
            return i;
        }
    }
    return -1;
}

/*
Print the permissions available to the entry.
*/
void printPermission(unsigned int entry){
    printf("%c%c%c", (entry & READ_BIT) ? 'R' : '-',
    (entry & WRITE_BIT) ? 'W' : '-',
    (entry & EXEC_BIT) ? 'X' : '-');
}

/*
Check if the entry has the permission type.
*/
int hasPermission(unsigned int entry, char type){
    switch(type){
        case 'R':
            return (entry & READ_BIT) != 0;
        case 'W':
            return (entry & WRITE_BIT) != 0;
        case 'X':
            return (entry & EXEC_BIT) != 0;
        default:
            return 0;
    }
}

/*
Goes through each level and checks if there is a page fault. If no page fault, print success message with the relavent information.
*/
void checkEntry(MMU *mmu){
    for(int i = 0; i < mmu->opNums; ++i){

        unsigned int address = mmu->operations[i].address;
        char type = mmu->operations[i].type;

        if(address & ~ADDRESS_MASK){
            printf("Op #%d (%c 0x%08X) Page Fault. Logical address has non-zero bits in bits 31-14\n", i, type, address);
            continue; 
        }

        unsigned int page = address >> 5;
        unsigned int offset = address & OFFSET_MASK;

        unsigned int level1 = (page >> 6) & 7;
        unsigned int level2 = (page >> 3) & 7;
        unsigned int level3 = page & 7;

        unsigned int entry1 = mmu->frames[0].entries[level1];
        if(!(entry1 & VALID_BIT)){
            printf("Op #%d (%c 0x%08X) Page Fault. Entry %u in table at frame %u is not set to valid\n", i, type, address, level1, mmu->frames[0].frameNumber);
            continue;
        }

        unsigned int frame2 = (entry1 & FRAME_MASK) >> 5;
        int index2 = findFrame(mmu, frame2);

        if(index2 < 0){
            printf("Op #%d (%c 0x%08X) Page Fault. Frame %u is not present in memory\n", i, type, address, frame2);
            continue;
        }

        unsigned int entry2 = mmu->frames[index2].entries[level2];
        if(!(entry2 & VALID_BIT)){
            printf("Op #%d (%c 0x%08X) Page Fault. Entry %u in table at frame %u is not set to valid\n", i, type, address, level2, mmu->frames[index2].frameNumber);
            continue;
        }

        unsigned int frame3 = (entry2 & FRAME_MASK) >> 5;
        int index3 = findFrame(mmu, frame3);

        if(index3 < 0){
            printf("Op #%d (%c 0x%08X) Page Fault. Frame %u is not present in memory\n", i, type, address, frame3);
            continue;
        }

        unsigned int entry3 = mmu->frames[index3].entries[level3];

        if(!(entry3 & VALID_BIT)){
            printf("Op #%d (%c 0x%08X) Page Fault. Entry %u in table at frame %u is not set to valid\n", i, type, address, level3, mmu->frames[index3].frameNumber);
            continue;
        }

        unsigned int frameAddress = entry3 & FRAME_MASK;
        unsigned int physicalAddress = frameAddress | offset;

        if(!hasPermission(entry3, type)){
            printf("Op #%d (%c 0x%08X) Page Fault. Illegal access to frame %u due to entry %u in table at frame %u with access ", i, type, address, frameAddress/PAGE_SIZE, level3, frame3);
            printPermission(entry3);
            printf("\n");
            continue;
        }

        printf("Op #%d (%c 0x%08X) Success. Physical address 0x%08X (Frame %u, Access ", i, type, address, physicalAddress, frameAddress/PAGE_SIZE);
        printPermission(entry3);
        printf("\n");
    }  
}

//--------------MAIN--------------
int main(int argc, char *argv[]){
    printf("Assignment 4: MMU Simulator by Emily Hull\n");

    if(argc < 2){
        printf("Usages: %s <filename>\n", argv[0]);
        return -1;
    }

    FILE *fp = fopen(argv[1], "r");
    if(fp == NULL){
        printf("Unable to open file %s\n", argv[1]);
        perror("File open failure.");
        return -1;
    }

    MMU mmu;

    initMMU(&mmu, fp);

    checkEntry(&mmu);

    return 0;
}
//--------------END OF MAIN--------------