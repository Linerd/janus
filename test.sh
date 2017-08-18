#!/bin/bash

qq() {
  $1 ../lib/data/test_input_$2.dat | grep -v '\-\-\-\-' | cut -d' ' -f3 | cut -d'/' -f1 -f2 | sed -e 's/\// /'| sort -n -k2,2 -k1,1
}


output() {
  id=$1
  paste <(qq ../lib/maxmin $id) <(qq ../old/maxmin $id) | awk '{print $1, $3, ($1 - $3)/($1+1), ($2-$4)/($2+1), $1/($2+1), $3/($4+1)}'
}

tmpFile=$(mktemp)
output $1 > $tmpFile

echo -ne "Erroneous lines:\n"
echo -ne "\033[1;31m"
cat $tmpFile | awk '{if ($5 > 1) print $0}'
echo -ne "\033[0m"

echo -ne "Greeneries:\n"
echo -ne "\033[1;92m"
cat $tmpFile | awk '{if ($1 != $2) print $0}'
echo -ne "\033[0m"