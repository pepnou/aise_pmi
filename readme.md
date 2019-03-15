# PROJET SERVEUR PMI AISE MASTER 1 CHPS SEMESTRE 2

Voici le projet de
PEPIN Thibaut et EMERY Antoine

compilation et execution :
	git clone https://github.com/pepnou/aise_pmi
	cd aise_pmi
	make

-> lancer le serveur :
	sudo ./pmiserver [numero du port >= 1024]
-> lancer les processus clients :
	sudo ./helper/pmirun [ip du serveur]:[numero du port serveur] [nombre de processus du client] [programme a lancer]
	/!\ toute ip commençant par 127.0.0 est pour le local, distant autrement

/!\ Apres l'execution du programme, supprimer le repertoire map pour eviter de vous retrouver enfouis sous des fichier de 16Mb.
/!\ Chaque programme lancé avec pmirun nécessite 32Mb de mémoire RAM.


LISTE DES FONCTIONNALITEES :

A1
A2
A3
A4

B5
B7
Pour des raisons qui nous échappent, lors de la lecture de valeur dans le segment shm, des valeures héronées peuvent être lues, malgrés la mise en place de mlock et msync.
Nous avons modifié le programme de test pour que celui-ci ne s'arrete pas a la lecture d'une valeure fausse, ce qui nous a permis de constater que sur 300 000 clé-valeur écrite nous rencontrions moins de 3 fautes. L'utilisation des sockets ne génére aucune erreur dans les tests (voir b8 pour leurs utilisation).
B8
Le changement de la couche de transport en fonction de la localité des processus est naturellement utilisé. I est cependant possible de forcer l'utilisation des sockets en spécifiant la variable d'environement DYNAMIC_COMM au lancement, ce qui donne la commande suivante : 
	sudo DYNAMIC_COMM=n ./helper/pmirun [ip du serveur]:[numero du port serveur] [nombre de processus du client] [programme a lancer]
B9
lors des test d'utilisation en local avec sockets , écriture de clé : 4 usec et lecture : 10 usec
lors des test d'utilisation en local avec shm, écriture de clé : 2 usec et lecture : 8 usec

C11 :
- amélioration de performance en passant par une table de hachage pour les clés
- support multijob et multiprocess pour le serveur
- support de n'importe quel type d'envoi a condition que le type soit contigu en mémoire






# AISE PMI SERVER

To build simply run make, feel free to fork this version for your work !


## Using PMIRUN

```
pmirun [PMI SERVER] [PROCESS] [CMD] [ARGS..]
```
- PMI SERVER: a field exported as PMI_SERVER in the env
- PROCESS: the number of processes to be launched
- CMD: the command
- ARGS: optionnal arguments

example:
```
pmirun 127.0.0.1:9000 16 ./a.out
```

You may change the PMI server to match your implementation.

## PMI Test

Used to validate that your PMI runtime works.

```
make
pmirun 127.0.0.1:9000 16 ./test_value
```

## PMI Perf

Used to validate the performance of your PMI runtime.
Only runs on two processes.

```
make
pmirun 127.0.0.1:9000 2 ./test_perf
```


