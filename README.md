# CSE240A Branch Predictor Project

Implement gshare and tournament branch predictors. Also implement a custom predictor within a 64K bits budget to beat gshare and tournament. We have implemented TAGE and Perceptron as our custom predictors.

## Table of Contents
  * [Introduction](#introduction)
  * [Traces](#traces)
  * [Running your predictor](#running-your-predictor)
  * [Implementing the predictors](#implementing-the-predictors)
    - [Gshare](#gshare)
    - [Tournament](#tournament)
    - [TAGE](#tage)
    - [Perceptron](#perceptron)
    - [Things to note](#things-to-note)

## Introduction

Highly accurate branch predictors are desirable in modern day processors to maximize performance. In this project we have implemented three branch prediction mechanisms: g-share, tournament and TAGE and compared the misprediction rate on the provided benchmarks. We also examine the storage requirements and tradeoffs for the TAGE predictor that impact the accuracy of the predictor.


## Traces

These predictors will make predictions based on traces of real programs.  Each line in the trace file contains the address of a branch in hex as well as its outcome (Not Taken = 0, Taken = 1):

```
<Address> <Outcome>
Sample Trace from int_1:

0x40d7f9 0
0x40d81e 1
0x40d7f9 1
0x40d81e 0
```


## Running your predictor

In order to build the predictors you simply need to run `make` in the src/ directory of the project.  You can then run the program on an uncompressed trace as follows:   

`./predictor <options> [<trace>]`

If no trace file is provided then the predictor will read in input from STDIN. Some of the traces we provided are rather large when uncompressed so we have distributed them compressed with bzip2. If you want to run your predictor on a compressed trace, then you can do so by doing the following:

`bunzip2 -kc trace.bz2 | ./predictor <options>`

In either case the `<options>` that can be used to change the type of predictor
being run are as follows:

```
 --help       Print this message
 --verbose    Print predictions on stdout
 --<type>     Branch prediction scheme:
    static
    gshare:<# ghistory>
    tournament:<# ghistory>:<# lhistory>:<# index>
    tage:<# tagged component index bits>:<# first tagged component history>:<# tagged components>:<# base predictor index bits>
    perceptron:<# number of perceptrons>:<# history length>

```
An example of running a gshare predictor with 10 bits of history would be:   

`bunzip2 -kc ../traces/int1_bz2 | ./predictor --gshare:10`

A bash script is provided to run all benchmarks for all predictors. Run `make` in the project directory and then run `bash runbenchmarks.sh` and it will produce an output like this:

```
Running int_1.bz2 benchmark..
gshare:     Branches: 3771697 Incorrect: 521958 Misprediction Rate: 13.839
tournament: Branches: 3771697 Incorrect: 485664 Misprediction Rate: 12.877
tage:      Branches: 3771697 Incorrect: 345638 Misprediction Rate: 9.164
perceptron:Branches: 3771697 Incorrect: 341755 Misprediction Rate: 9.061
Running int_2.bz2 benchmark..
gshare:     Branches: 3755315 Incorrect: 15776 Misprediction Rate: 0.420
tournament: Branches: 3755315 Incorrect: 17291 Misprediction Rate: 0.460
tage:      Branches: 3755315 Incorrect: 11702 Misprediction Rate: 0.312
perceptron:Branches: 3755315 Incorrect: 15305 Misprediction Rate: 0.408
Running fp_1.bz2 benchmark..
gshare:     Branches: 1546797 Incorrect: 12765 Misprediction Rate: 0.825
tournament: Branches: 1546797 Incorrect: 15329 Misprediction Rate: 0.991
tage:      Branches: 1546797 Incorrect: 12636 Misprediction Rate: 0.817
perceptron:Branches: 1546797 Incorrect: 12641 Misprediction Rate: 0.817
Running fp_2.bz2 benchmark..
gshare:     Branches: 2422049 Incorrect: 40641 Misprediction Rate: 1.678
tournament: Branches: 2422049 Incorrect: 74677 Misprediction Rate: 3.083
tage:      Branches: 2422049 Incorrect: 26701 Misprediction Rate: 1.102
perceptron:Branches: 2422049 Incorrect: 23344 Misprediction Rate: 0.964
Running mm_1.bz2 benchmark..
gshare:     Branches: 3014850 Incorrect: 201871 Misprediction Rate: 6.696
tournament: Branches: 3014850 Incorrect: 153547 Misprediction Rate: 5.093
tage:      Branches: 3014850 Incorrect: 83394 Misprediction Rate: 2.766
perceptron:Branches: 3014850 Incorrect: 105757 Misprediction Rate: 3.508
Running mm_2.bz2 benchmark..
gshare:     Branches: 2563897 Incorrect: 259929 Misprediction Rate: 10.138
tournament: Branches: 2563897 Incorrect: 223019 Misprediction Rate: 8.698
tage:      Branches: 2563897 Incorrect: 193244 Misprediction Rate: 7.537
perceptron:Branches: 2563897 Incorrect: 250405 Misprediction Rate: 9.767
Average misprediction for gshare predictor is: 5.5993
Average misprediction for tournament predictor is: 5.2003
Average misprediction for tage is: 3.6163
Average misprediction for perceptron is: 4.0875
```

## Implementing the predictors

There are 4 methods which are implemented in the predictor.c file. They are: **init_predictor**, **make_prediction**, and **train_predictor** and **cleanup**.

`void init_predictor();`

This will be run before any predictions are made.  This is where you will initialize any data structures or values you need for a particular branch predictor 'bpType'.  All switches will be set prior to this function being called.

`uint8_t make_prediction(uint32_t pc);`

You will be given the PC of a branch and are required to make a prediction of TAKEN or NOTTAKEN which will then be checked back in the main execution loop. You may want to break up the implementation of each type of branch predictor into separate functions to improve readability.

`void train_predictor(uint32_t pc, uint8_t outcome);`

Once a prediction is made a call to train_predictor will be made so that you can update any relevant data structures based on the true outcome of the branch. You may want to break up the implementation of each type of branch predictor into separate functions to improve readability.

`void cleanup();`

At the end of running the trace cleanup is called for each predictor to free up the dynamically allocated structures to avoid memory leaks.

#### Gshare

```
Configuration:
    ghistoryBits    // Indicates the length of Global History kept
```
The Gshare predictor is characterized by XORing the global history register with the lower bits (same length as the global history) of the branch's address.  This XORed value is then used to index into a 1D BHT of 2-bit predictors.

#### Tournament
```
Configuration:
    ghistoryBits    // Indicates the length of Global History kept
    lhistoryBits    // Indicates the length of Local History kept in the PHT
    pcIndexBits     // Indicates the number of bits used to index the PHT
```

You will be implementing the Tournament Predictor popularized by the Alpha 21264.  The difference between the Alpha 21264's predictor and the one you will be implementing is that all of the underlying counters in yours will be 2-bit predictors.  You should NOT use a 3-bit counter as used in one of the structure of the Alpha 21264's predictor.  See the Alpha 21264 paper for more information on the general structure of this predictor.  The 'ghistoryBits' will be used to size the global and choice predictors while the 'lhistoryBits' and 'pcIndexBits' will be used to size the local predictor.

#### TAGE

The TAGE predictor proposed by A.Seznec and Pierre Michaud builds upon the PPM predictor. Originally PPM was used for text compression and then applied in branch prediction. Today TAGE is considered to be one of the best branch predictors and there are modifications proposed for it. For this project, we use the original TAGE predictor with a slight modification of using linear histories as compared to geometric histories. The original paper can be found [here](https://www.irisa.fr/caps/people/seznec/JILP-COTTAGE.pdf).

#### Perceptron

The Perceptron predictor proposed by Daniel A. Jimenez and Calvin Lin uses a table of perceptrons indexed by Program Counter to give accurate predictions. This predictor works well for linearly separable branches. The original paper can be found [here](https://www.cs.utexas.edu/~lin/papers/hpca01.pdf)

#### Things to note

All history are initialized to NOTTAKEN.  History registers are updated by shifting in new history to the least significant bit position.
```
Ex. 4 bits of history, outcome of next branch is NT
  T NT T NT   <<  NT
  Result: NT T NT NT
```
```
All 2-bit predictors should be initialized to WN (Weakly Not Taken).
They should also have the following state transitions:

        NT      NT      NT
      ----->  ----->  ----->
    ST      WT      WN      SN
      <-----  <-----  <-----
        T       T       T
```

The Choice Predictor used to select which predictor to use in the Alpha 21264 Tournament predictor should be initialized to Weakly select the Global Predictor.

