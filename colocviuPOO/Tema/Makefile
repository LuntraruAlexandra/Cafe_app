CG = g++
CFLAGS = #-Wall -Werror -Wextra -g

build: tema.cpp
	$(CG) tema.cpp -o tema $(CFLAGS)

run: tema
	./tema $(INPUT)

clean:
	rm -f tema