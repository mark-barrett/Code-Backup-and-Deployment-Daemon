CC=gcc
objects = main.o backup.o lock_files.o logger.o update.c
headers = backup.h lock_files.h logger.h update.h

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

update.o: update.c
	$(CC) -c update.c
