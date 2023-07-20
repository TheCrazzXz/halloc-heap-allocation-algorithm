#/bin/bash

if [ "$#" -lt 2 ]; then
    echo "Usage : bash compile.sh <SOURCE.c> <OUTPUT EXECUTABLE> [GCC FLAGS] "
    exit
fi

gcc heap/*.c $1 -o $2 $3