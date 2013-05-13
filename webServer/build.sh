rm -rf *~ client server webserver.o &&
gcc -o client client.c &&
gcc -o webserver.o -c webserver.c &&
gcc -o server main.c webserver.o -lpthread &&
sync
sync
