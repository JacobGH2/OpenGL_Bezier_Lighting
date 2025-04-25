all: main.cpp
	g++ main.cpp -o main -framework GLUT -framework OpenGL
	./main

clean:
	rm -f main