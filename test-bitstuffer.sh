#! /bin/bash
gcc -g -o bitstuff bitstuff.c
SAMPLE=sample-mod.txt
PERIOD=6
SUCCESS=0
echo Testing bitstuffer...
for (( i = 2; i < $PERIOD; i++ )); do
  t=mktemp;
  ./bitstuff -scp $i -f $SAMPLE | ./bitstuff -ucp $i > $t
  diff $SAMPLE $t
  SUCCESS=$?
  rm $t
done
(( !$SUCCESS )) && echo "Bitstuffer runs as expected."
(( $SUCCESS )) && echo "Test failed."


