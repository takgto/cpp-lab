CXX      = g++
CXXFLAGS = -O2 -std=c++17 -Wall
TARGET   = app
OBJS     = main.o util.o

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

main.o: main.cpp util.h
	$(CXX) $(CXXFLAGS) -c main.cpp

util.o: util.cpp util.h
	$(CXX) $(CXXFLAGS) -c util.cpp

clean:
	rm -f $(TARGET) $(OBJS)
