# LabScreenplay

A utility for working with screenplays.

Parses screenplays written in fountain markdown format.

Very simple main program re-emits the screenplay to c:/tmp/test.fountain for validation.

The parser in main parses a script in markdown format into a simple C++ data structure. main then re-emits, to prove that it didn't lose anything. The parser detects title page information like author and copyright, inventories all the characters and locations, finds all the direction notes and dialog, and stashes it all.

