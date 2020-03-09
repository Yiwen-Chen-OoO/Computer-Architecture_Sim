## Fairness
The fairness is evaluated using the **unfairness index**, which is the ratio of maximun of the memory slow down and minimun of the memory slow down. 

## Memory Slow Down

THe MemSlowDown is calculated by Memory stall time per instruction it experiences when running together with other threads divided by the memory stall time per instruction it experiences when running alone on hte same system


## Blacklisting Mechanism
1> App ID of the last scheduled request\
2> # of requests served from an application\
3> blacklist status of each application

### Memory controller
a. compare App ID with last scheduled request\
   if same, requested served ++\
   or, request = 0

b. request served > threshold, blacklisted = 1
request served = 0
