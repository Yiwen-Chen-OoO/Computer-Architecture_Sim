#ifndef __REQUEST_HH__
#define __REQUEST_HH__

#define __STDC_FORMAT_MACROS
#include <inttypes.h> // uint64_t

typedef enum Request_Type{READ, WRITE}Request_Type;

// Instruction Format
typedef struct Request
{
    int core_id; // The core-id/app-id sends the request.

    Request_Type req_type;

    uint64_t memory_address;

    /* Decoding Info */
    int channel_id;
    int bank_id; // Which bank it targets to.
    
    //Additional Property in core not in 
    // int num_req_served;
    //bool blacklist; //threshold NOT defined true if > threshold

    /* 
    MemSlowDown (min and max)
    Unfairness 
    
    */
}Request;

#endif
