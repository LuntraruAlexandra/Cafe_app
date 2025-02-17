CG = g++
CFLAGS = #-Wall -Werror -Wextra -g

build: app.cpp
	$(CG) app.cpp -o app $(CFLAGS)

run: app
	./app $(INPUT)

clean:
	rm -f app