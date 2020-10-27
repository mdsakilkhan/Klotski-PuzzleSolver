# Klotski-PuzzleSolver
6/1/2020

Klotski puzzle solver solves any given configuration by an user. In the program, I utilized the SDL library for the interface and user controls; through the use of mouse and keyboard inputs, users can drag and drop puzzle pieces or blocks. Strings of size 20 are used to represent the block configurations on the 4 by 5 board. I build functions that translated characters of the string to blocks on the board (and vise-versa). I used enumerator for the different types of blocks; and used the decision tree method to write possible movement rules for each type, through the use of switch and if statements. The Breadth-first search algorithm was used to find the shortest path through all the possible outcomes. I created a variation of the Zobrist hashing algorithm to improve the runtime by catching and eliminating mirroring configurations that are unnecessary in the solution step. I used one vector of strings for storing all the configurations generated by the BFS algorithm, which allowed users to iterate through the vector to see each step it takes to get to the solution.
