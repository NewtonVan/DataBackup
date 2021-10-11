# 配置步骤
## 1 docker环境搭建
首先访问[boost下载页面](https://nchc.dl.sourceforge.net/project/boost/boost/1.73.0/boost_1_73_0.tar.gz)下载boost库文件，并放置与本项目env_confg目录下。

然后进入env_confg目录下，执行以下命令：
```shell
docker build -t dbk .

# 注意配置数据卷以支持项目映射
docker run -it -v <本项目绝对路径>:/DataBackup -p 127.0.0.1:8081:8081 -p 127.0.0.1:8082:8082 dbk /bin/bash
```

## 2 代码运行
首先进入容器，开两个终端，于容器内本项目路径下，执行以下命令：
```shell
# 启动后端服务
make run

# 启动前端服务（另开终端）
cd front_end && npm start
```

或快速启动（后端以后台任务运行）
```shell
make runAll
```

然后访问浏览器地址 [localhost 8082端口](http://localhost:8082/)，即可进行软件使用。
