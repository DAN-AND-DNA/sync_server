# sync_server

This project is an experimental version that was temporarily written for a tencent game. The code has many problems and needs a lot of modifications, but show you how to use async hiredis in your network libs how to use asynchronous hiredis to access redis.

这个项目其实是早期给腾讯做游戏的时候临时写的实验版本，代码有很多问题, 需要大量修改,不过就看看如何使用异步 hiredis 来访问redis就可以了

build:
1. no deps(hiredis is in ./lib and ./redis)
2. vim CMakeLists.txt to change SYNC_SERVER_NET_PATH & change password in sync_server/eventloop/RedisChannel.cpp
3. cd build; cmake ..; make

easy test:
1. in redis:  hset hloginuser:wx123456 eggs 7 age 37  
2. in redis:  zadd testkey 37 wx123456
3. run sync_server (./run sync_server)

result:
holding on.....
1. wx123456 will be del & hloginuser:wx123456 will be expired for 20 seconds 



这个项目其实是早期给腾讯做游戏的时候临时写的实验版本，代码有很多问题, 需要大量修改,不过就看看如何使用异步 hiredis 来访问redis就可以了
编译:
1. 没有依赖(hiredis 提前放在 ./lib and ./redis)

