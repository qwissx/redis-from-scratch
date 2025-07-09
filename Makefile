CXX = g++
CXXFLAGS = -Wall -Wextra -O2
TARGET01 = server
TARGET02 = client
SRC01 = server.cpp
SRC02 = client.cpp
UTILS = utils.cpp

all: $(TARGET01) $(TARGET02)

$(TARGET01): $(SRC01)
	$(CXX) $(CXXFLAGS) $(SRC01) $(UTILS) -o $(TARGET01)

$(TARGET02): $(SRC02)
	$(CXX) $(CXXFLAGS) $(SRC02) $(UTILS) -o $(TARGET02)

clean:
	rm -f $(TARGET01) $(TARGET02)
