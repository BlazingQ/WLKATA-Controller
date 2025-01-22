# 编译器设置
CC=g++

# 编译器标志，添加-std=c++11以支持C++11特性
CFLAGS=-g -Wall -O2 -fPIC

# 链接器标志，指定要链接的库文件
LDFLAGS=-L./arm_converter/lib -L./arm_controller/lib \
        -Wl,-rpath=\$$ORIGIN/arm_converter/lib -Wl,-rpath=\$$ORIGIN/arm_controller/lib \
        -lverifier -lbach -fopenmp -lstdc++ -lz3 -lmathsat -lgmp -lcryptominisat5 -lcadical -lcontroller 

# 目标可执行文件名
TARGET=controller

# 项目中所有相关的源文件
SRC=main.cpp ArmControllerServer.cpp tcpdocker.cpp

# 对应的源文件生成的目标文件列表
OBJ=$(SRC:.cpp=.o)

# 默认目标（all）将构建目标程序
all: $(TARGET)

# 链接目标程序和verifier库
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) $(LDFLAGS)

# 编译每个源文件
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# 清理生成的文件
clean:
	rm -f *.o $(TARGET)