# sync_server

This project is an experimental version that was temporarily written for a tencent game. The code has many problems and needs a lot of modifications, but it show you how to use asynchronous hiredis in your network libs to access redis.

这个项目其实是很早期给腾讯做游戏的时候临时写的实验版本，代码有很多问题, 需要大量修改,不过就看看如何使用异步 hiredis 来访问redis就可以了

build:
1. no deps(hiredis is in ./lib and ./redis)
2. vim CMakeLists.txt to change SYNC_SERVER_NET_PATH 
(for example, this project is at /home/yourname/Server/src/qb/sync_server  so the SYNC_SERVER_NET_PATH is /home/yourname/Server/src/qb/)
4. change redis  password in sync_server/eventloop/RedisChannel.cpp
3. cd build; cmake ..; make

easy test:
1. in redis:  hset hloginuser:wx123456 eggs 7 age 7  
2. in redis:  zadd testkey 37 wx123456
3. run sync_server (./sync_server)

result:
holding on.....
1. wx123456 will be del &hloginuser:wx123456 will be getall & hloginuser:wx123456 will be expired for 20 seconds 




编译:
1. 没有依赖(hiredis 提前放在了 ./lib and ./redis)
2. 把CMakeLists.txt 的SYNC_SERVER_NET_PATH改成sync_server的上级目录 & 在sync_server/eventloop/RedisChannel.cpp修改密码
3. 在build中,cmake..;make

简单测试:
1. 在redis中:  hset hloginuser:wx123456 eggs 7 age 7 
2. 在redis中: zadd testkey 37 wx123456
3. 运行 sync_server  (./sync_server)

结果:
稍微等待一下.....
1. wx123456 将被删除 &hloginuser:wx123456 被getall &hloginuser:wx123456 被设置成20秒过期 

