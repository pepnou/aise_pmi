# PROJET SERVEUR PMI AISE MASTER 1 CHPS SEMESTRE 2

Voici le projet de
PEPIN Thibaut et EMERY Antoine

compilation et execution :
	git clone https://github.com/pepnou/aise_pmi
	cd aise_pmi
	make

-> lancer le serveur :
	./pmiserver [numero du port >= 1024]
-> lancer les processus clients :
	./helper/pmirun 127.0.0.1:[numero du port serveur] [nombre de processus du client] ./[programme a lancer]
	/!\ 127.0.0.1 est pour le local, autrement, utilisé l'ip du serveur distant


LISTE DES FONCTIONNALITEES :

A1
A2
A3
A4
B5
B7
B8
B9 (lors des test d'utilisation en local, écriture de clé : ~ 3 usec et lecture : ~ 100 usec)
C11 :
- amélioration de performance en passant par une table de hachage pour les clées
- support multijob et multiprocess
- type d'envoi non limitant a condition que ce soit contigu en mémoire











SUJER :














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


