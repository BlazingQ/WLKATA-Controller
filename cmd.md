构建镜像：
docker build -t wlkata .

运行：
docker run --name wlkata-app --network host wlkata
docker run -d --name wlkata-app --network host wlkata
-d是为了隐去输出，同时不阻塞命令行


