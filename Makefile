all: main.cpp
	g++ --std=c++11 main.cpp -o main -framework GLUT -framework OpenGL
	./main

clean:
	rm -f main