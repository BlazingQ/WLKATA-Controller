# 编译器设置 - 使用 g++。
CC=g++

# 编译器标志 -g 添加调试信息，-Wall 开启额外警告，-std=c++11 使用 C++11 标准。
CFLAGS=-g -Wall

# 目标可执行文件名
TARGET=controller

# 构建目标可执行文件，依赖于 controller.o 和 tcpdocker.o。
all: $(TARGET)

# 链接生成最终的可执行文件
$(TARGET): controller.o tcpdocker.o
	$(CC) $(CFLAGS) -o $(TARGET) controller.o tcpdocker.o

# 编译 controller.cpp 生成对象文件。
controller.o: controller.cpp tcpdocker.h
	$(CC) $(CFLAGS) -c controller.cpp

# 编译 tcpdocker.cpp 生成对象文件。
tcpdocker.o: tcpdocker.cpp tcpdocker.h
	$(CC) $(CFLAGS) -c tcpdocker.cpp

# 清理生成的文件
clean:
	rm -f *.o $(TARGET)
