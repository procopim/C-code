#include <sys/mman.h>   /*For mmap() function -- NA on windows as mmap is a POSIX call*/
#include <string.h>    /*For memcpy function*/
#include <fcntl.h>     /*For file descriptors*/
#include <stdlib.h>    /*For file descriptors*/
#include <stdio.h>

/*
A simple memory management simulator in C that supports paging. In this setup the logical address space (2^16 = 65,536 bytes) is larger than the physical
address space (2^15 bytes), and the page size is 256 bytes. The maximum no. of entries in the TLB = 16.

Example of page fault is as follows: if a logical address with page number, for example 15 resulted in a page fault, program reads in page 15 from BACKING STORE 
(remember that pages begin at 0 and are 256 bytes in size) and store it in an empty frame in physical memory. If an empty frame does not exist, we need to
replace the oldest page. Suppose page 2 was the oldest page in memory. In this case we bring in page 15 and store it in the frame occupied by page 2.
We update the page table to reflect this change. Any subsequent accesses to page 15 will be resolved by the page table (or TLB).*/

FILE *fptr;
#define BUFFER_SIZE 10  // addresses from file are no bigger than 10 characters
#define OFFSET_MASK 255 // based on Page size of 2^8 - 1
#define PAGES 256       // unit: bytes  |  logical/page size = pages 
#define FRAMES 128      // unit: single frames  | 2^7 (based on 2^15 physical address space size)
#define OFFSET_BITS 8   // based on page size of 2^8
#define PAGE_SIZE 256   // unit: bytes  | given by assignment spec 
#define TLBSIZE 16      // 16 entries to TLB given by assignment spec 
#define MEMORY_SIZE PAGE_SIZE*FRAMES // amt of bytes for main memory 
#define BACKING_STORE_SIZE 65536

typedef struct {
    /*should be initialized with parameters:
    array->F = 0;
    array->R = -1;
    array->TE = 0;
    */
    signed char arr[MEMORY_SIZE];
    int R;
    int F;
    int TE;
} circular; 

typedef struct {
    /*should be initialized with parameters:
    array->F = 0;
    array->R = -1;
    array->TE = 0;
    arr[16 rows][2 elements] ele0 = page, ele1 = frame
    */
    int arr[TLBSIZE][2];
    int R;
    int F;
    int TE;
} TLBentry; 

/*Function declarations for circular struct manipulation*/
int add(circular *param, signed char page[],int page_num, int *page_table);
void *delete(circular *param, int *page_table);
void *show_arr(circular *param);
signed char get_val(circular *param, int frame, int offset);
int search(int *a,int n,int key);
/*Function declarations for TLBentry struct manipulation*/
void *TLBadd(TLBentry *param, int page_num, int frame);
void *TLBdelete(TLBentry *param, int page_num);
void *TLBupdate(TLBentry *param, int preempt_page, int replace_page, int replace_frame);
int TLBsearch(TLBentry *param, int page);

int main(){

    /*initialize our main memory struct; our circular array abstraction is maintained by the circular struct*/
    circular *memarray = (circular *)malloc(sizeof(circular));
    memarray->F = 0;
    memarray->R = -1;
    memarray->TE = 0;
    
    /*initialize our TLB*/
    TLBentry *TLB = (TLBentry *)malloc(sizeof(TLBentry));
    TLB->F = 0;
    TLB->R = -1;
    TLB->TE = 0;

    /* int array page table. Indices are page numbers, value is frame #*/
    int page_table[PAGES];
    /*signed pointer to store the starting address of the memory mapped file*/
    signed char *mmapfptr;  

    fptr = fopen("addresses.txt","r");
    /*read-in buffer for lines read from file*/
    char buff[BUFFER_SIZE];

    /*initialize page table*/
    for (int i =0; i < PAGES; i++){
        page_table[i] = -1;
    }

    /*initialize TLBentry array to empty*/
    for (int k =0; k < TLBSIZE; k++){
        TLB->arr[k][0]=-1;
        TLB->arr[k][1]=-1;
    }

    /*declare page buffer for fetch from backing store */
    signed char page_buffer[PAGE_SIZE];
    /*frame number*/
    int frame;
    /*fault counter*/
    int fault_counter = 0;
    /*counter*/
    int counter =0;
    /*TLB hit counter*/
    int hit = 0;

    /*declare output values*/
    int out_virtual_address;
    int out_physical_address;
    signed char out_value;



    /*Read from addresses.txt til you read end of file.*/
    while (fgets(buff, BUFFER_SIZE, fptr) != NULL){
        //buff[strcspn(buff, "\n")] = 0;
        /*unsigned int is binary form;  atoi converts buff to int which is a string*/
        int logical_address = atoi(buff);
        /*shift logical right from binary logical address (isolates address, removes offset)*/
        int page_num = logical_address>>OFFSET_BITS;
        /*create offset for the given logical address*/
        int offset = logical_address & OFFSET_MASK;

        /*temporary variable until TLB implemented*/
        int tlb_index = -1;
        tlb_index = TLBsearch(TLB, page_num);

        /* is the page in the TLB? */
        if(tlb_index != -1){
            /* TLB HIT 
            GET the Page & Frame pair
            */
            out_virtual_address = logical_address;
            frame = TLB->arr[tlb_index][1];
            out_physical_address = (frame<<OFFSET_BITS) | offset;
            out_value = get_val(memarray, frame, offset);
            hit = hit + 1;
        }
        else{
            /* TLB MISS - 
            
            does the page table have a valid frame number at the given page_num index?  */
            if(page_table[page_num] != -1)
            {
                /*Yes - frame exists*/

                out_virtual_address = logical_address;
                /*store frame number*/
                frame = page_table[page_num];    
                /*create physical address using offset specific to this logical_address*/
                out_physical_address = (frame<<OFFSET_BITS) | offset;
                /*get_val returns the int value at the offset byte*/
                out_value = get_val(memarray,frame,offset);
                TLBadd(TLB, page_num, frame);
            }
            else
            {   /*PAGE FAULT - get page from backing store*/

                fault_counter = fault_counter +1;

                int mapfile_fd = open("BACKING_STORE.bin",O_RDONLY);
                /*mmap returns an address in virtual/logical space*/
                mmapfptr = mmap(0, BACKING_STORE_SIZE, PROT_READ, MAP_PRIVATE, mapfile_fd, 0);
                /*memcpy to page buffer, the desired page from backing store */
                memcpy(page_buffer, mmapfptr + (PAGE_SIZE*page_num), PAGE_SIZE);

                /*check if the front page is in the TLB*/
                int preempt_index;
                preempt_index = TLBsearch(TLB, *TLB->arr[TLB->F]);
                /*if the TLB is full - replace the page at front of array (update), else add it to the TLB*/
                if(TLB->TE == TLBSIZE){
                    TLBupdate(TLB, preempt_index, page_num, frame);
                }
                else{
                    TLBadd(TLB, page_num, frame);
                }
                

                frame = add(memarray, page_buffer, page_num, page_table);
                /*memory unmap function*/
                munmap(mmapfptr, BACKING_STORE_SIZE);

                /*create physical address using offset specific to the page in this frame*/
                out_physical_address = (frame*PAGE_SIZE+offset);
                //out_physical_address = (frame<<OFFSET_BITS) | offset;
                out_virtual_address = logical_address;
                /*get_val returns the signed char value at the offset byte*/
                out_value = get_val(memarray,frame,offset);
            }
        }
        counter = counter +1;
        printf("virtual address: %d | physical_address: %d | val: %d\n", out_virtual_address, out_physical_address, out_value);
    }
    fclose(fptr);

    printf("\nTotal addresses = %d\n",counter);
    printf("TLB hits = %d\n",hit);
    printf("fault count = %d\n",fault_counter);
    return 0;
};

/*
  @brief add function returns an int representing the rear index of circular memarray 
  @detail since add places at rear value, we know this is the frame the page is placed in
*/
int add(circular *param, signed char page[], int page_num, int *page_table){
    int frame;
    if(param->TE == FRAMES)
    {
        delete(param, page_table);   // deletes first item, shifts the front of array
        add(param, page, page_num, page_table); // adds element to end
    }
    else
    {
        param->R=(param->R+1)%FRAMES;
        /*copying the page to the circular struct memarray->arr[R*PAGE_SIZE] location*/        
        memcpy(param->arr+(param->R*PAGE_SIZE), page, PAGE_SIZE);

        param->TE=param->TE+1;
        frame = param->R;
        /*store the frame number in the page table*/
        page_table[page_num] = frame;

    }
    /*index val represents the frame number we just wrote to in memarry (our main mem)*/
    return param->R;
}

/*
  @brief delete function overwrites data at front index, and moves the front index to the next one 
*/
void *delete(circular *param, int *page_table){
    signed char del[256];
    int preempt_page;

    for (int i = 0; i < PAGE_SIZE; i++){
        del[i] = -1;
    }

    if(param->TE == 0)
    {
        printf("Queue is empty");
    }
    else
    {
        /* insert -1 to the front frame (FIFO)*/
        memcpy(param->arr+(param->F*PAGE_SIZE), del, PAGE_SIZE);
        /*find the page number index assigned  the front frame number value, in page table*/
        preempt_page = search(page_table, PAGES, param->F);

        //printf("\npage preempted: %d ", preempt_page);
        page_table[preempt_page] = -1;
        //printf("\nnew value in page-table %d\n", page_table[preempt_page]);

        param->F=(param->F+1)%FRAMES;           // move front of array to next item
        param->TE=param->TE-1;                  // decrement total elements by 1
    }
}

/*
  @brief get_val returns the signed char value at the offset byte
*/
signed char get_val(circular *param, int frame, int offset){
    signed char val = param->arr[frame*PAGE_SIZE+offset]; 
    return val;
}
/*
  @brief returns the index of the page (key) found in the page table
*/
int search(int *a,int n,int key)
{ 
    for(int i=0; i<n; i++)
    {
        if(a[i]==key)
        {
             return i;
        }
    } 
 }

/* returns the index if page exists in TLB - else returns -1 indicating page not present */
int TLBsearch(TLBentry *param, int page){
    /*TLB array abstraction:
    array[16 items][page,frame]   
    */
    for(int i=0; i<TLBSIZE; i++)
    {
        /*if page is found - return the item index*/
        if(param->arr[i][0] == page){
            return i;
        }
    }
    return -1;
}
/*
@brief TLBadd adds a page&frame pair to the TLB 
@detail function adjusts the Rear of the circular TLB array
@detail function calls delete if the TLB is full*/
void *TLBadd(TLBentry *param, int page_num, int frame){

    if(param->TE == TLBSIZE)
    {
        TLBdelete(param,page_num);     // deletes first TLB item, shifts the front of array
        TLBadd(param, page_num, frame); // adds element to end
    }
    else
    {   /*update the rear value of array*/
        param->R=(param->R+1)%TLBSIZE;  
        /*update elements for that rear index*/      
        param->arr[param->R][0]= page_num;
        param->arr[param->R][1]= frame;
        param->TE=param->TE+1;

        //printf("\nTLB index: %d  page: %d  frame: %d",param->R, param->arr[param->R][0], param->arr[param->R][1]);
    }
}
/*
@brief removes the front entry of TLB circular array and sets to -1
@detail shifts the front of array to next sequential index following removal*/
void *TLBdelete(TLBentry *param, int page_num){

    if(param->TE == 0)
    {
        printf("TLB is empty");
    }
    else
    {
        /* insert -1 to the front row (FIFO)*/
        param->arr[param->F][0] = -1;
        param->arr[param->F][1] = -1;
        param->F=(param->F+1)%FRAMES;           // move front of array to next item
        param->TE=param->TE-1;                  // decrement total elements by 1
    }
}
/*
@brief updates entry in TLB at a given preempt_index defined by program
*/
void *TLBupdate(TLBentry *param, int preempt_index, int replace_page, int replace_frame){
    /* update the new page at the same location as that of the page p */
    param->arr[preempt_index][0] = replace_page;
    param->arr[preempt_index][1] = replace_frame;
    if (preempt_index != -1) {
        param->TE=param->TE+1;
    }
}

