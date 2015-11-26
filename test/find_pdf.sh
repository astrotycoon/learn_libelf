#/bin/bash

ECHO="echo -e"

# blue
function printc()
{
    $ECHO "\t \033[1;36m $1 \033[0m"
}
## red
function printcw()
{
    $ECHO "\t \033[1;35m $1 \033[0m"
}


find . -name "*.pdf" | while read line
do 
	$ECHO $line
#	mkdir -p /tftpboot`dirname $line`;
done
