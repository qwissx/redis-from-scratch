CXX = g++
CXXFLAGS = -Wall -Wextra -O2
TARGET = redis
SRC = server.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
