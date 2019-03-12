CC=gcc
objects = main.o backup.o
headers = backup.h

backupDaemon: $(objects)
	     $(CC) -o backupDaemon $(objects) -lm

main.o: main.c $(headers)
	$(CC) -c main.c

backup.o: backup.c
	  $(CC) -c backup.c
