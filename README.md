# text-adventure
Text-based adventure game.

Players can move from room to room, searching for the end room to win the game!

**Disclaimer:** There is some redundancy in the use of files to generate the game's room files. This was done for practice reading to and writing from files. A *cleaner* implementation would simply utilize the data structures generated to store game layout and room information.

# Usage
Compile with `make`

Compile and execute both executables with `make run`

`./build_rooms` generates a directory with files for each room. Each file contains information for a particular room. **Must be done first before executing adventure**

`./adventure` executes the game

`make build_rooms` to generate new files which will generate a new game board layout

`make clean` to cleanup files and executables
