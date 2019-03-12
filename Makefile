CC=gcc
objects = main.o backup.o lock_files.o logger.o
headers = backup.h lock_files.h logger.h

backupDaemon: $(objects)
	     $(CC) -o backupDaemon $(objects) -lm

main.o: main.c $(headers)
	$(CC) -c main.c

backup.o: backup.c
	  $(CC) -c backup.c

lock_files.o: lock_files.c
	$(CC) -c lock_files.c

logger.o: logger.c
	$(CC) -c logger.c
