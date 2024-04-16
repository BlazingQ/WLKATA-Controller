# 使用官方Ubuntu镜像作为基础镜像
FROM ubuntu:22.04

# 设置非交互式安装，避免安装过程中出现提示
ENV DEBIAN_FRONTEND=noninteractive

# 更新软件包列表并安装编译环境
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    iputils-ping \
    net-tools \
    && rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /app

# 将你的C++代码添加到镜像中
COPY . /app

# 编译你的应用程序
RUN make

# 暴露端口（假设你的应用监听12345端口）
EXPOSE 12345

# 运行应用程序
CMD ["./your_application"]
