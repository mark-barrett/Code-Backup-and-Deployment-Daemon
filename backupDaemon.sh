#! /bin/bash

### BEGIN INIT INFO
# Provides:          foo
# Required-Start:    $local_fs $network
# Required-Stop:     $local_fs
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: foo service
# Description:       Run Foo service
### END INIT INFO

# Carry out specific functions when asked to by the system
case "$1" in
  start)
    echo "Starting Code Backup and Deployment Daemon"
    sudo /usr/sbin/backupDaemon
    ;;
  stop)
    echo "Stopping Code Backup and Deployment Daemon"
    echo "Yuup"
    sleep 2
    ;;
  *)
    echo "Usage: /etc/init.d/backupDaemon {start|stop}"
    exit 1
    ;;
esac

exit 0

