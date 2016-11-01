# Admissions

Here are two of my independent programming projects.

Inside each folder is the source code for each program written in C++. Additionally inside the folder is a subfolder "Binaries" that contains the executable version built for Windows computers.

Connect Four is an artificial intelligence that plays the game Connect Four against a human opponent. In this game different color chips are dropped into a grid with the goal of obtaining four in a row of your color chip on a horizontal, vertical, or diagonal line. The only other rule is that if no one in admissions can defeat the AI then I must be accepted (note: the converse need not be true). To make a move click on the column you would like to drop your chip in, and it will be placed in the bottom-most unfilled row. When there is a winner or a draw the command line will display the result and prompt for further input. The program requires a graphics chip with OpenGL 3.3 compatibility.

Derivatizor is a command line program that takes a function of one variable and returns its derivative. For example, typing in "4sin(x^2) + 2x" will return "8*x*cos(x^2) + 2"

I included all of the code along with the Microsoft Visual Studio project file so that the source code could be compiled directly from this repository. I understand that I have included a lot, so for the sake of speed the most important source code files I would recommend the admissions department review are:

Connect Four/AI.cpp, Derivatizor/simplify.cpp

And of course I would recommend running the executable files under the "Binaries" folder to see what the finished product looks like.
