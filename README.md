# text-adventure
Text-based Adventure Game for an Operating Systems 1 course.

The game is based on old text adventure games like [Colossal Cave Adventure](http://en.wikipedia.org/wiki/Colossal_Cave_Adventure)

Players can move from room to room by typing the name of the room. The goal is to reach the end room to win the game!

**Disclaimer:** There is some redundancy in the use of files to generate the game's room files. This was done for practice reading to and writing from files because it was an assignment for a course. A *cleaner* implementation would simply utilize the data structures generated to store game layout and room information.

# Usage
Compile with `make`

Compile and execute both executables with `make run`

`./build_rooms` generates a directory with files for each room. Each file contains information for a particular room. **Must be done first before executing adventure**

`./adventure` executes the game

`make build_rooms` to generate new files which will generate a new game board layout

`make clean` to cleanup files and executables
