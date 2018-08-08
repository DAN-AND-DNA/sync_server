# sync_server
this project is just show you how to use async hiredis in your network libs

build:
1. no deps(hiredis is ./lib and ./redis)
2. vim CMakeLists.txt to change SYNC_SERVER_NET_PATH & change password in sync_server/eventloop/RedisChannel.cpp
3. cd build; cmake ..; make

easy test:
1. in redis:  hset hloginuser:wx123456 eggs 7 age 37  
2. in redis:  zadd testkey 37 wx123456
3. run sync_server (./run sync_server)

result:
1. wx123456 will be del & hloginuser:wx123456 will be expired for 20 seconds 

