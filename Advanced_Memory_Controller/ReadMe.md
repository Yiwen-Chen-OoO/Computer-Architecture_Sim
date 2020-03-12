## Fairness
The fairness is evaluated using the **unfairness index**, which is the ratio of maximun of the memory slow down and minimun of the memory slow down. 

## Memory Slow Down

THe MemSlowDown is calculated by Memory stall time per instruction it experiences when running together with other threads divided by the memory stall time per instruction it experiences when running alone on hte same system

$$ MemorySlowDown = \frac{MCPI_{shared}}{MCPI_{alone}}$$
<br/>


$$ Unfairness  = \frac{max(MemSlowDown)}{min(MemSlowDown)}$$

## Blacklisting Mechanism

    1> App ID of the last scheduled request\
    2> # of requests served from an application\
    3> blacklist status of each application

### Memory controller
a. compare App ID with last scheduled request\
&emsp; if same, requested served ++\
&emsp; or, request = 0

b. request served > threshold, blacklisted = 1
request served = 0

### Memory Requests are prioritized in the following order:

    1> Non-blacklist applications' requests
    2> Request hit to a free bank. <FR-FCFS>
    3> Older requests