CC = gcc
CG = g++
CFLAGS = #-Wall -Werror -Wextra -g

build_tester: app.cpp tester.c compare_files.cpp copy_files.cpp
	$(CG) compare_files.cpp -o compare_files $(CFLAGS)
	$(CG) copy_files.cpp -o copy_files $(CFLAGS)
	$(CG) app.cpp -o app $(CFLAGS)
	$(CC) tester.c -o tester $(CFLAGS)

run_tests: tester
	./tester

clean:
	rm -f app tester copy_files compare_files