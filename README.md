# Quiz-Socket-Application
## Simple console based quiz aplication made in C using BCD sockets.

### How to run the server:
- Navigate to Server folder
- compile server.c using: gcc -pthread -o server server.c
- run program using: ./server



### Server:
- [X] Server setup routine
- [X] Server file management functions (user identify by nickname, questions and answers, ...)
	- [X] Read from files
	- [X] Modify files (eg: a client answers a question, and his answer is saved)
- [X] Server listener thread (listen for incoming connections from clients)
- [X] Server client handler thread (unique thread for each connected client)
	- [X] Client identification (eg: nickname -> client socket)
	- [X] Client communication (recieve)
- [X] Client keyword interpretation
- [X] Server client transmission (data transmission)
	- eg: send_data_function(data, client_socket)
- [ ] Optional Client status check thread
	- if a client stopped responding or failed the conenction, we cand disconnect it

- Server threads: 
	- Main thread (file management + client transmission) (can be broken down in 2 threads)
	- Listener thread -> will start a handler thread after an incoming connection
	- Unique client handler thread for each connected client
	- Optional Client status check thread

### Client:
- [X] Client initialization routine
- [X] Client connect to server routine
- [X] Client receive thread (server -> client)
- [X] Client transmission function (client -> server)
- [X] Client set nickname
- [X] Client question list menu
- [X] Client answer question menu
	- [X] Question answered
	- [X] Display your answer, others answer and the correct/suggested answer
	- [X] Possible return to menu
- [ ] Optional disconnect from server
	- Using keyword "Exit"
	- After disconnect from server, program ends, ? console will close

- Client threads:
	- Main thread (most probably everything except receive)
	- Receive thread

