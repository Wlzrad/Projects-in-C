# Projects-in-C
this repository contains programs I wrote purely in C, without the use of external libraries


# Bridge Repair
I wrote this program for the project for the Design of Algorithms subject in Semester 2 of first year. It takes an input containing a 2x2 grid of variable dimensions that proposes a route from start to finish, along with a number of blockages along the route. The program I wrote processes this information and evaluates the proposed route to see if it is valid or blocked. If it's blocked, then it will dynamically repair the route by re-routing around the blocks, taking the shortest route possible to the destination.

**how to use**: navigate to source directory and run "gcc -o app bridgerepair.c", then run the program, feeding in any of the test inputs: "app < test0.txt"
