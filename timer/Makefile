objects := timer 
cxxflags := -std=c++11 -lpthread -w 
# -Wint-to-pointer-cast -Wdelete-incomplete -Wint-to-pointer-cast -Wint-to-pointer-cast -Wdelete-incomplete
all: $(objects)
timer: TestTimer.cpp WheelTimer.h
	g++ ./TestTimer.cpp $(cxxflags) -o timer

.PHONY: clean
clean:
	rm $(objects)