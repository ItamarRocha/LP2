Compile:
    ```
    gcc server.c -pthread -o server
    ```
    ```
    gcc client.c -pthread -o client
    ```  
    or just type make

Executing:
    Server:
    ```    
    ./chat [port] [chat_name]
    ```
    Client:     
    ```    
    ./chat [ip] [port] [username]
    ```
Messages will be stored at [chat_name].txt in the same folder
