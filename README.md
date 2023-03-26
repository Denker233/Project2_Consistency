
run server \
argv[1] : "primary" if running a primary server; anything else would be replica\
argv[2] : {"local_write", "primary", "quorum"}\
argv[3] : port 4000\
```
./server <primary/replica> <strategy> <port>
```

run client
ip: not useful now, can be any number
```
./client <ip> <port> <option:post/read/choose/reply> <title> <content>
```

example:
```
./server primary local_write 4000
./server 1 local_write 4001
./client 1 4000 Post abc cba
```
