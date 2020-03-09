#include "Trace.h"

#include "Mem_System.h"

extern TraceParser *initTraceParser(const char * mem_file);
extern bool getRequest(TraceParser *mem_trace);

extern MemorySystem *initMemorySystem();
extern unsigned pendingRequests(MemorySystem *mem_system);
extern bool access(MemorySystem *mem_system, Request *req);
extern void tickEvent(MemorySystem *mem_system);

int main(int argc, const char *argv[])
{	
    if (argc != 2)
    {
        printf("Usage: %s %s\n", argv[0], "<mem-file>");

        return 0;
    }

    // Initialize a CPU trace parser
    TraceParser *mem_trace = initTraceParser(argv[1]);

    // Initialize the memory system
    MemorySystem *mem_system = initMemorySystem();
    // printf("%u\n", controller->bank_shift);
    // printf("%u\n", controller->bank_mask);

    uint64_t cycles = 0;

    bool stall = false;
    bool end = false;

    while (!end || pendingRequests(mem_system))
    {
        if (!end && !stall)
        {
            end = !(getRequest(mem_trace));
        }

        if (!end)
        {
            stall = !(access(mem_system, mem_trace->cur_req));
	    
            // printf("%u ", mem_trace->cur_req->core_id);
            // printf("%u ", mem_trace->cur_req->req_type);
            // printf("%"PRIu64" \n", mem_trace->cur_req->memory_address);
        }

        tickEvent(mem_system);
        ++cycles;
    }

    // TODO, de-allocate memory
    /*
    free(controller->bank_status);
    free(controller->waiting_queue);
    free(controller->pending_queue);
    free(controller);
    */
    printf("End Execution Time: ""%"PRIu64"\n", cycles);
}
