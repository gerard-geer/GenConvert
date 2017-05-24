#==============================================================================
# File: makefile
# Author: Gerard Geer
# License: GPL v3
# Purpose: It makes genconvert.
#==============================================================================
CC=g++
NAME=genconv
SRCDIR=src/
BINDIR=bin/
all:
	@echo Compiling * into $(NAME)...
	$(CC) $(SRCDIR)*.cc -o $(BINDIR)$(NAME)
	@echo Giving you permission to run it.
	chmod +x $(BINDIR)$(NAME) 
clean:
	@echo Removing object files.
	rm -f *.o
	@echo Cleaning out $(BINDIR).
	rm -f ./bin/*


