This is the repository for a simple working shell. Runs successfully in a remote linux docker container (cannot guarantee success on other machines). Does not work on my local windows machine.  

To run the shell:
1. Run `make bin/mysh` or `make all`
2. Run `./bin/mysh`

Shell supports simple commands, `>`, `<`, `|`

Shell can be tested by running directly, running the test_suites (see outer Makefile for commands), or making use of the `testshell.c` script (reading the code would be more informative than an explanation here). A couple simple tests are already set up in the `testfiles` folder. Tests can be run by 
1. Run `make testshell`
2. Run `./testshell`
