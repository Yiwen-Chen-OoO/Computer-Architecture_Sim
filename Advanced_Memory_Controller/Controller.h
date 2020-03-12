#ifndef __CONTROLLER_HH__
#define __CONTROLLER_HH__

#include "Bank.h"
#include "Queue.h"

// Bank
extern void initBank(Bank *bank);
// FAIRNESS CALCU
static bool BLISS = true;
static bool SHARE = false;
const int APP = 0;
// Queue operations
extern Queue* initQueue();
extern void pushToQueue(Queue *q, Request *req);
extern void migrateToQueue(Queue *q, Node *_node);
extern void deleteNode(Queue *q, Node *node);

// CONSTANTS
static unsigned MAX_WAITING_QUEUE_SIZE = 64;
static unsigned BLOCK_SIZE = 64; // cache block size
static unsigned NUM_OF_CHANNELS = 4; // 4 channels/controllers in total
static unsigned NUM_OF_BANKS = 32; // number of banks per channel

// DRAM Timings
static unsigned nclks_channel = 15;
static unsigned nclks_read = 53;
static unsigned nclks_write = 53;

static unsigned BlackListThreshold = 4;
static unsigned BlackListCleaning = 1000;
// PCM Timings
// static unsigned nclks_read = 57;
// static unsigned nclks_write = 162;

// Controller definition
typedef struct Controller
{
    // The memory controller needs to maintain records of all bank's status
    Bank *bank_status;

    // Current memory clock
    uint64_t cur_clk;

    // Channel status
    uint64_t channel_next_free;

    // A queue contains all the requests that are waiting to be issued.
    Queue *waiting_queue;

    // A queue contains all the requests that have already been issued 
    // but are waiting to complete.
    Queue *pending_queue;

    /* For decoding */
    unsigned bank_shift;
    uint64_t bank_mask;
    int last_scheduled;
    int request_served[8];
    uint64_t blacklist_until[8];
    // The controller->request_served[i] represent the i th core

}Controller;

Controller *initController()
{
    Controller *controller = (Controller *)malloc(sizeof(Controller));
    controller->bank_status = (Bank *)malloc(NUM_OF_BANKS * sizeof(Bank));
    for (int i = 0; i < NUM_OF_BANKS; i++)
    {
        initBank(&((controller->bank_status)[i]));
    }
    controller->cur_clk = 0;
    controller->channel_next_free = 0;

    controller->waiting_queue = initQueue();
    controller->pending_queue = initQueue();

    controller->bank_shift = log2(BLOCK_SIZE) + log2(NUM_OF_CHANNELS);
    controller->bank_mask = (uint64_t)NUM_OF_BANKS - (uint64_t)1;
    controller->last_scheduled = 8; // since valid range 0-7
    for (int i = 0;i<7;i++){
        controller->request_served[i] = 0;
    }

    return controller;
}

unsigned ongoingPendingRequests(Controller *controller)
{
    unsigned num_requests_left = controller->waiting_queue->size + 
                                 controller->pending_queue->size;

    return num_requests_left;
}

bool send(Controller *controller, Request *req)
{
    if (SHARE == false){
        if (req->core_id == APP){
            if (controller->waiting_queue->size == MAX_WAITING_QUEUE_SIZE)
            {
                return false;
            }

            // Decode the memory address
            req->bank_id = ((req->memory_address) >> controller->bank_shift) & controller->bank_mask;
        
            // Push to queue
            pushToQueue(controller->waiting_queue, req);

            return true;
        }

    }else{
            if (controller->waiting_queue->size == MAX_WAITING_QUEUE_SIZE)
            {
                return false;
            }

            // Decode the memory address
            req->bank_id = ((req->memory_address) >> controller->bank_shift) & controller->bank_mask;
        
            // Push to queue
            pushToQueue(controller->waiting_queue, req);

            return true;
        
    }
    
    
}

void tick(Controller *controller)
{
    // Step one, update system stats
    ++(controller->cur_clk);
    // printf("Clk: ""%"PRIu64"\n", controller->cur_clk);
    for (int i = 0; i < NUM_OF_BANKS; i++)
    {
        ++(controller->bank_status)[i].cur_clk;
        // printf("%"PRIu64"\n", (controller->bank_status)[i].cur_clk);
    }
    // printf("\n");

    // Step two, serve pending requests
    if (controller->pending_queue->size)
    {
        Node *first = controller->pending_queue->first;
        if (first->end_exe <= controller->cur_clk)
        {
            /*
            printf("Clk: ""%"PRIu64"\n", controller->cur_clk);
            printf("Address: ""%"PRIu64"\n", first->mem_addr);
            printf("Channel ID: %d\n", first->channel_id);
            printf("Bank ID: %d\n", first->bank_id);
            printf("Begin execution: ""%"PRIu64"\n", first->begin_exe);
            printf("End execution: ""%"PRIu64"\n\n", first->end_exe);
            */

            deleteNode(controller->pending_queue, first);
        }
    }
    
    if (BLISS == false){
        if (controller->waiting_queue->size)
        {
            // Implementation One - FR-FCFS
            Node *first = controller->waiting_queue->first;
            for (int i=0;i<(controller->waiting_queue->size);i++)
            {
                int target_bank_id = first->bank_id;

                if ((controller->bank_status)[target_bank_id].next_free <= controller->cur_clk && 
                    controller->channel_next_free <= controller->cur_clk)
                {
                    first->begin_exe = controller->cur_clk;
                    if (first->req_type == READ)
                    {
                        first->end_exe = first->begin_exe + (uint64_t)nclks_read;
                    }
                    else if (first->req_type == WRITE)
                    {
                        first->end_exe = first->begin_exe + (uint64_t)nclks_write;
                    }
                    // The target bank is no longer free until this request completes.
                    (controller->bank_status)[target_bank_id].next_free = first->end_exe;
                    controller->channel_next_free = controller->cur_clk + nclks_channel;

                    migrateToQueue(controller->pending_queue, first);
                    deleteNode(controller->waiting_queue, first);
                }
            }
            Node *firstTemp = first->next;
            first = firstTemp;
            
        }
    }
    else 
    {
        // Step three, BLISS
        if (controller->waiting_queue->size)
        {
            // Implementation One - FCFS --> FR-FCFS
            Node *first = controller->waiting_queue->first;
            for (int i = 0; i < controller->waiting_queue->size; i++)
            {
                //Dealing with Non-Black List
                
                // Request_served Counter 
                if (first->core_id == controller->last_scheduled){
                    controller->request_served[first->core_id]++;
                } else {
                    controller->request_served[first->core_id] = 0;
                }

                // Determine if a core is blocked 
                // a core would not be blocked agin if alreadt blocked
                if (controller->request_served[first->core_id] >= BlackListThreshold &&
                    controller->blacklist_until[first->core_id] >= controller->cur_clk)
                {
                    controller->request_served[first->core_id] = 0;
                    controller->blacklist_until[first->core_id] = controller->cur_clk + BlackListCleaning;
                }
                //for non blk list
                if (controller->blacklist_until[first->core_id] <= controller->cur_clk){
                    int target_bank_id = first->bank_id;
                
                    if ((controller->bank_status)[target_bank_id].next_free <= controller->cur_clk && 
                        controller->channel_next_free <= controller->cur_clk)
                    {
                        first->begin_exe = controller->cur_clk;
                        if (first->req_type == READ)
                        {
                            first->end_exe = first->begin_exe + (uint64_t)nclks_read;
                        }
                        else if (first->req_type == WRITE)
                        {
                            first->end_exe = first->begin_exe + (uint64_t)nclks_write;
                        }
                        // The target bank is no longer free until this request completes.
                        (controller->bank_status)[target_bank_id].next_free = first->end_exe;
                        controller->channel_next_free = controller->cur_clk + nclks_channel;
                        controller->last_scheduled = first->bank_id;

                        migrateToQueue(controller->pending_queue, first);
                        deleteNode(controller->waiting_queue, first);
                    }
                }
                Node *firstTemp = first->next;
                first = firstTemp;       


                
                
            }
        }
    }

}

#endif
