CC = gcc
CG = g++
CFLAGS = #-Wall -Werror -Wextra -g

build_tester: tema.cpp tester.c compare_files.cpp copy_files.cpp
	$(CG) compare_files.cpp -o compare_files $(CFLAGS)
	$(CG) copy_files.cpp -o copy_files $(CFLAGS)
	$(CG) tema.cpp -o tema $(CFLAGS)
	$(CC) tester.c -o tester $(CFLAGS)

run_tests: tester
	./tester

clean:
	rm -f tema tester copy_files compare_files