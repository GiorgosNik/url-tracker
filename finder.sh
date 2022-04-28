#!/bin/bash

for var in "$@"
do
    counter=0
    for i in /tmp/snifferOut/*.out; do
        while IFS= read -r line
        do
            arrIN=(${line//./ })
            tld=${arrIN[${#arrIN[@]}-2]}     
            num=${arrIN[${#arrIN[@]}-1]}
            if [ "$tld" = "$var" ]; then
                counter=$((counter+ num))
            fi  
        done < "$i"

    done
    echo "$var $counter"
done

