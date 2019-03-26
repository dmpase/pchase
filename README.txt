NEW ENVIRONMENT VARIABLES
=========================

seconds (or sec): a floating point value that represents the number of seconds 
each sample will run.

tpr_max, tpr_min: (threads per run, max and min), an integer that represents 
the upper and lower boundaries on the number of threads used per sample.

sockets: Possible values are {uni, multi, both}. This environment variable 
gives the number of sockets to be used in the run. "uni" means only one socket 
will be used. "multi" means all sockets will be used. "both" means two sets 
of experiments will be run, one of "uni" and one of "multi".

locality: Possible values are {local, remote, both, near, far, all}. This 
environment variable indicates the locality of data buffers used. "local" 
means all data resides within the same NUMA domain where the thread resides. 
"remote" means combinations involving data one hop away are used. "both" means 
experiments involving data that is both local and remote will be performed. 
"near" is a synonym for "remote". "far" means experiments involving data that 
are two hops away will be performed. "all" means three sets of experiments 
will be performed, namely, "local", "remote" and "far".

bit: Possible values are {32, 64}. bit=32 means run the 32-bit version of the 
benchmark. bit=64 means run the 64-bit version of the benchmark. Some of the
benchmarks have only 64-bit code, and do not operate in 32-bit mode.

NOTES
=====

(1) Tried pinning threads to specific logical cores in order to improve
repeatability of results. Pinning threads did NOT improve the consistency --
it made it much worse. 

There is also a flaw with the way it is (and can be) implemented. The basic 
approach was to create a queue containing the IDs of all cores within a NUMA 
domain, and set the thread's affinity to the head of the queue with each new 
thread. The flaw is that logical cores that share a physical core may be close
together or far apart, and there is no way for the benchmark to check. System
topology information is only available within the kernel, and not to programs
in user-space. That means the benchmark is not able to avoid mapping two
threads logical cores that share a physical core, while other physical cores
are idle, which would have a serious negative impact on benchmark performance.

(2) Tried using the x86 CLFLUSH instruction to clear the cache in order to
make some of the benchmark runs more consistent. In fact, the opposite
happened. Without the CLFLUSH, memory latencies do seem to bounce around, but
they seem to be completely consistent from one run to the next. Adding the
CLFLUSH caused the latencies to bounce around eratically, where the BEST
results with CLFLUSH matched the results without CLFLUSH.

In short, neither experiment improved benchmark performance at all. In both
cases the results were better without artificially flushing the cache and
without pinning threads to specific logical cores.
