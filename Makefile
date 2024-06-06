# DEBUG := -ggdb

all: shared
	g++ main.cpp -o main -fpermissive $(DEBUG)

shared: shared.cpp
	g++ shared.cpp -o libshared.dll -shared -fPIC