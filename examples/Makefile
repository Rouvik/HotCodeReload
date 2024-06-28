# DEBUG := -ggdb
CXX := g++

all: shared
	$(CXX) demo.cpp -o demo $(DEBUG)

shared: shared.cpp
	$(CXX) shared.cpp -o libshared.dll -shared -fPIC