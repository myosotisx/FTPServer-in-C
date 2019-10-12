server: server.o server_pi.o server_cm.o server_util.o
	gcc -o server server.o server_pi.o server_cm.o server_util.o 

server.o: server_util.h server_pi.h server_cm.h server.c
	gcc -c server.c

server_pi.o: server_util.h server_pi.c
	gcc -c server_pi.c

server_cm.o: server_util.h server_cm.h server_cm.c
	gcc -c server_cm.c

server_util.o: server_util.c
	gcc -c server_util.c

clean:
	rm *.o