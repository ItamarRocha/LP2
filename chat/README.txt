Compile:
```shell
gcc server.c -pthread -o server
```
```shell
gcc client.c -pthread -o client
```  
> or just type make

Executing:

* Server:
```shell    
./chat [port] [chat_name]
```
* Client:     
```shell
./chat [ip] [port] [username]
```
Messages will be stored at [chat_name].txt in the same folder
