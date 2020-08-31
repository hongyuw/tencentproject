主函数在 myproxy.cpp

fd注册epoll进行客户端监听

当listen fd 有数据读入，执行accept，同时连接后端服务器，此时建立前后端fd的键值对，保存在unorderedmap

新数据读入时，进入epoll，如果fd键值对存在，fd进入队列，否则目前不做任何处理

worker线程任务执行fd的读取和转发，每次buffer目前1024，循环进行读取发送，读取多少就发多少，读取完成或异常就break

目前不支持健康检查，如果服务器断开连接，或者客户端断开连接之后，fdtab还没有进一步处理，计划将fd封装进结构体，加入server或者client属性

服务器未开启，启动proxy的情况下，前端发起连接，proxy会结束

执行
cmake .
make
./bin/DEMOPROXY

或
./build.sh