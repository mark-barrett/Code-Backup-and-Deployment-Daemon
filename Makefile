CC=gcc
objects = main.o backup.o lock_files.o
headers = backup.h lock_files.h

backupDaemon: $(objects)
	     $(CC) -o backupDaemon $(objects) -lm

main.o: main.c $(headers)
	$(CC) -c main.c

backup.o: backup.c
	  $(CC) -c backup.c

lock_files.o: lock_files.c
	$(CC) -c lock_files.c
