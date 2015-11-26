#/bin/bash
ECHO="echo -e"

#DIR1=/home/astrol/work/rootfs/hi3515/BVS-W4020-VC-H.filesys.3515
#DIR2=/home/astrol/work/rootfs/hi3515/BVS-W4020-VC-H.filesys.3515
DIR1=$1
DIR2=$2

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

if [ $# -ne 2 ]; then
	printcw "Usage: $0 + dir1 + dir2";	
	exit -1
fi

function list_file()
{
	for file in $1/*
	do 
		if [ -f $file ]; then
			$ECHO "$file is a file";
		fi

		if [ -d $file ]; then
			$ECHO "$file is a direction";
			list_file $file;
		fi
	done
}

#list_file $DIR1;
#list_file $DIR2;

function compare_file()
{
	for file1 in $1/*
	do
		if [ -f $file1 ]; then		# file
			for file2 in $2/*
			do
				FILE1=`basename $file1`;
				FILE2=`basename $file2`;
				if [ -f $file2 ] && [ $FILE1 == $FILE2 ]; then
					cmp -s $file1 $file2
					if [ $? != 0 ]; then
						$ECHO "$FILE1 is differ"	
					else
						$ECHO "$FILE1 is same"
					fi
					break;
				fi
			done
		fi

		if [ -d $file1 ]; then 	# direction
			compare_file $file1 $2/`basename $file1`
		fi	
	done
}

compare_file $DIR1 $DIR2;

