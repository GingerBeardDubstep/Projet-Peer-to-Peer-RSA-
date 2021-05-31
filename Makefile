all : clean client serveur final_clean

clean : 
	@rm -rf *.o client serveur

final_clean:

	@rm -f *.o

srcDir := ./src

mdir:
	mkdir -p serverSaves

MyP2P_util.o:
	gcc -Wall -Wextra -Wformat -c $(srcDir)/MyP2P_util.c

serveur : MyP2P_util.o mdir
	gcc -Wall -Wextra -Wformat -pthread -o serveur MyP2P_util.o $(srcDir)/serveur.c

client: $(srcDir)/main_client.c client.o MyP2P_util.o
	gcc -Wall -Wextra -Wformat -o  client MyP2P_util.o client.o $(srcDir)/main_client.c 

	
client.o : MyP2P_util.o
	gcc -Wall -Wextra -Wformat -c  $(srcDir)/client.c

main : 
	gcc -Wall -Wformat -o main $(srcDir)/main.c

utils:
	gcc -Wall -Wformat -o MyP2P_util $(srcDir)/MyP2P_util.c

