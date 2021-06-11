#!/bin/bash
inputfile=$1
inputdir=$2
numfilesperdir=$3

#check if directory exists and print error
if [ -d "$inputdir" ];
then
  echo "\"$inputdir\" directory already exists."
  exit 1;
fi

#create parent directory
mkdir $inputdir

#declare associative arrays
declare -A countries
declare -A limit
declare -A counter
declare -A index

#first pass of inputFile
#create subdirectories and count the number of records for each country
while read line;
do
  a=( $line )
  mkdir -p $inputdir/${a[3]}
  if [[ -v "countries[${a[3]}]" ]];
  then
    let "countries[${a[3]}]=countries[${a[3]}]+1"
  else
    countries[${a[3]}]=1
  fi
done < $inputfile

#create #numFilesPerDir text files in the subdirectories
for j in $inputdir/*;
do
  for (( i=1; i<=$numfilesperdir; i++ ));
  do
    IFS='/' read -ra ADDR <<< "$j"
    touch "$j/${ADDR[1]}-$i.txt"
  done
done

#set how many records will each file have
for i in "${!countries[@]}";
do
  counter[$i]=0
  index[$i]=1
  limit[$i]=`expr ${countries[$i]} / $numfilesperdir`
done

#second pass of the inputFile
#fill the files with records
while read line;
do
  a=( $line )
  if [[ ${counter[${a[3]}]} -gt ${limit[${a[3]}]} ]];
  then
    let "index[${a[3]}]=index[${a[3]}]+1"
    counter[${a[3]}]=0
  else
    let "counter[${a[3]}]=counter[${a[3]}]+1"
  fi
  echo "$line" >> "$inputdir/${a[3]}/${a[3]}-${index[${a[3]}]}.txt"
done < $inputfile

#Cleaning empty files
for j in $inputdir/*;
do
  for k in $j/*;
  do
    if [ ! -s $k ];
    then
      rm -f $k
    fi
  done
done

#delete associative arrays
unset countries
unset limit
unset counter
unset index

#exit properly
exit 0;