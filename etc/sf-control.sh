#!/bin/bash

. /etc/smart-fridge.init

case "$1" in

start)
    sf_start
    ;;
stop)
    sf_stop
    ;;
reload)
    sf_stop
    sf_start
    ;;
*)
    echo "Usage: $0 (start | stop)"
    ;;
esac

exit 0

sf_stop () {
    pkill -f $DIRECTORY/bin/sf_reader
    pkill -f $DIRECTORY/bin/sf_db
}

sf_start () {
    $DIRECTORY/bin/sf_reader -s &
    $DIRECTORY/bin/sf_db &
}
