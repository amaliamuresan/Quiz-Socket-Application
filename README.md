# Quiz-Socket-Application
## Simple console based quiz aplication made in C using BCD sockets.

How to run the server:
Navigate to Server folder
compile server.c using: gcc -pthread -o server server.c
run program using: ./server

### Priority tasks

- [X] Server setup routine - Theo
- [X] Server listener thread (listen for incoming connections from clients) - Theo
- [ ] Server client handler thread (unique thread for each connected client) - Dorian
	- [ ] Client communication (receive)
- [ ] Server client transmission (data transmission) - Vlad


- [X] Client initialization routine - Amalia
- [X] Client connect to server routine - Amalia
- [ ] Client receive thread (server -> client) - Dorian
- [ ] Client transmission function (client -> server) - Vlad



### Server:
- [ ] Server setup routine
- [ ] Server file management functions (user identify by nickname, questions and answers, ...)
	- [ ] Read from files
	- [ ] Modify files (eg: a client answers a question, and his answer is saved)
- [ ] Server listener thread (listen for incoming connections from clients)
- [ ] Server client handler thread (unique thread for each connected client)
	- [ ] Client identification (eg: nickname -> client socket)
	- [ ] Client communication (recieve)
- [ ] Client keyword interpretation
- [ ] Server client transmission (data transmission)
	- eg: send_data_function(data, client_socket)
- [ ] Optional Client status check thread
	- if a client stopped responding or failed the conenction, we cand disconnect it

- Server threads: 
	- Main thread (file management + client transmission) (can be broken down in 2 threads)
	- Listener thread -> will start a handler thread after an incoming connection
	- Unique client handler thread for each connected client
	- Optional Client status check thread

### Client:
- [ ] Client initialization routine
- [ ] Client connect to server routine
- [ ] Client receive thread (server -> client)
- [ ] Client transmission function (client -> server)
- [ ] Client set nickname
- [ ] Client question list menu
- [ ] Client answer question menu
	- [ ] Question answered
	- [ ] Display your answer, others answer and the correct/suggested answer
	- [ ] Possible return to menu
- [ ] Optional disconnect from server
	- Using keyword "Exit"
	- After disconnect from server, program ends, ? console will close

- Client threads:
	- Main thread (most probably everything except receive)
	- Receive thread

