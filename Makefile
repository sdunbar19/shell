CFLAGS = -Wall -g -Iinclude -fno-omit-frame-pointer -fsanitize=address
CC = cc
LIBRARIES = test_utils error_handling global_consts tokenizer token token_test_utils predefined_commands utils
LIBRARIES_OUT = $(addprefix out/,$(LIBRARIES:=.o))
# LDFLAGS = -rdynamic

all: bin/mysh

out/mysh.o: mysh.c
	$(CC) $(CFLAGS) -c $^ -o $@ $(LDFLAGS)

out/testshell.o: testshell.c
	$(CC) $(CFLAGS) -c $^ -o $@ $(LDFLAGS)

out/test_suite_%.o: test_suite/test_suite_%.c
	$(CC) $(CFLAGS) -c $^ -o $@ $(LDFLAGS)

out/%.o: library/%.c
	$(CC) $(CFLAGS) -c $^ -o $@ $(LDFLAGS)

bin/mysh: $(LIBRARIES_OUT) out/mysh.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

bin/test_suite_%: $(LIBRARIES_OUT) out/test_suite_%.o out/test_utils.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

testshell: $(LIBRARIES_OUT) out/testshell.o bin/mysh
	$(CC) $(CFLAGS) $(LIBRARIES_OUT) out/testshell.o -o $@ $(LDFLAGS)

clean: 
	rm -f out/*.o; rm -f bin/*; rm -f output.txt; rm -f testshell

test: test.c
	$(CC) $(CFLAGS) -c test.c -o test.o
	$(CC) $(CFLAGS) test.o -o test

.PHONY: all clean

