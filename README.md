# Telnet-Implementation
## About
* Simple telnet client which helps you connect a telnet server.
* It is a basic version which responds with `DO` to all the `WILL` requests and with `WON'T` to all the `DO` requests.

##Installation and Execution
* Compile the c program using : 
	* `gcc -o telnet telnet.c`
* Run the telnet client using : 
	* `./telnet <ip-address / hostname> <port-number>`
