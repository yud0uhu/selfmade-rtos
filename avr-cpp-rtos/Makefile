OBJS += syscall.o main.o
TARGET ?= mini
CXX := g++
CXXFLAGS =
CXXFLAGS += -std=c++17
CXXFLAGS += -Wall

.SUFFIXES: .cpp .o

all :		$(TARGET)

$(TARGET) :	$(OBJS)
		$(CXX) $(OBJS) -o $(TARGET) $(CXXFLAGS)

.cpp.o :		$<
		$(CXX) -c $< $(CXXFLAGS)

clean :
		rm -f $(OBJS) $(TARGET)