#!/bin/bash
file1=$1
file2=$2
limit=$3
duplicates=$4
rm -f inputFile
touch inputFile
output='inputFile'
#printf "virusesFile=$file1\n"
#printf "countriesFile=$file2\n"
#printf "numlines=$limit\n"
#printf "duplicatesAllowed=$duplicates\n"
i=0

declare -A dupl_track
value=1

#FILES LINES
c1="$(echo $(wc -l $file1)  | cut -d " " -f1)"
t=$(($limit / 4))
if (( $t < $c1 )) && (( $t > 0 ))
then
    c1=$t
fi
c2="$(echo $(wc -l $file2)  | cut -d " " -f1)"
d=0
v=0

for (( l=1; l<=$limit; l++))
do
    d=0
    v=0
    #RANDOM ID
    randomID=$(($RANDOM % 9999))
    while [[ ${dupl_track[$randomID]} ]] && [[ $duplicates == 0 ]]
    do
        randomID=$(($RANDOM % 9999))
    done
    if (( $duplicates == 1 )) && (( RANDOM % $limit > $limit/2+$limit/4))
    then
        randomID=$(($RANDOM % 9999))
        for i in "${!dupl_track[@]}"
        do
          if (( RANDOM % $limit > $limit/2));
          then
          randomID=$i
          d=1
          break
          fi
        done
    elif (( RANDOM % $limit > $limit/2+$limit/4))
    then
        randomID=$(($RANDOM % 9999))
        for i in "${!dupl_track[@]}"
        do
          if (( RANDOM % $limit > $limit/2));
          then
          randomID=$i
          v=1
          break
          fi
        done
    fi
    dupl_track[$randomID]=1
    printf "$randomID " >> $output
    if [ $d == 1 ] || [ $v == 1 ]
    then
        dupline=$(grep -n "$randomID" inputFile)
        a=( $dupline )
    fi
    #RANDOM NAME
    if [ $d == 0 ] && [ $v == 0 ]
    then
        chars=$(($RANDOM % 9 + 3))
        for (( i=0; i<$chars; i++ ))
        do
            if [ "$i" == "0" ]
            then
                ch=$(($RANDOM % 26 + 65))
                printf "\\$(printf %o $ch)" >> $output
            else
                ch=$(($RANDOM % 26 + 97))
                printf "\\$(printf %o $ch)" >> $output
            fi
        done
    else
        printf "${a[1]}" >> $output
    fi
    printf " " >> $output
    #RANDOM SURNAME
    if [ $d == 0 ] && [ $v == 0 ]
    then
        chars=$(($RANDOM % 9 + 3))
        for (( i=0; i<$chars; i++ ))
        do
            if [ "$i" == "0" ]
            then
                ch=$(($RANDOM % 26 + 65))
                printf "\\$(printf %o $ch)" >> $output
            else
                ch=$(($RANDOM % 26 + 97))
                printf "\\$(printf %o $ch)" >> $output
            fi
        done
    else
        printf "${a[2]}" >> $output
    fi
    printf " " >> $output
    #RANDOM COUNTRY
    if [ $d == 0 ] && [ $v == 0 ]
    then
        country=$(sed -n $(($RANDOM % $c2 + 1))p $file2)
        printf "$country " >> $output
    else
        printf "${a[3]} " >> $output
    fi
    #RANDOM AGE
    if [ $d == 0 ] && [ $v == 0 ]
    then
        age=$(($RANDOM % 120 + 1))
        printf "$age " >> $output
    else
        printf "${a[4]} " >> $output
    fi
    #RANDOM VIRUS
    if [ $d == 0 ] && [ $v == 0 ]
    then
        virus=$(sed -n $(($RANDOM % $c1 + 1))p $file1)
        printf "$virus " >> $output
    elif [ $d == 1 ]
    then
        printf "${a[5]} " >> $output
    elif [ $v == 1 ]
    then
        virus=$(sed -n $(($RANDOM % $c1 + 1))p $file1)
        while [[ $virus == ${a[5]} ]]
        do
            virus=$(sed -n $(($RANDOM % $c1 + 1))p $file1)
        done
        printf "$virus " >> $output
    fi
    #VACCINATED OR NOT
    if [ $d == 0 ] && [ $v == 0 ]
    then
        vac=$(($RANDOM % 4))
        if [ "$vac" == "0" ]
        then
            printf "NO " >> $output
        else
            printf "YES " >> $output
        fi
    elif [ $d == 1 ]
    then
        if [ "${a[6]}" == "NO" ]
        then
            printf "YES " >> $output
            vac=1
        else
            printf "NO " >> $output
            vac=0
        fi
    elif [ $v == 1 ]
    then
        vac=$(($RANDOM % 4))
        if [ "$vac" == "0" ]
        then
            printf "NO " >> $output
        else
            printf "YES " >> $output
        fi
    fi
    #RANDOM DATE OF VACCINATION
    if [ "$vac" != "0" ]
    then
        d=$(($RANDOM % 29 + 1))
        m=$(($RANDOM % 11 + 1))
        y=$(($RANDOM % 31 + 1980))
        printf "$d-$m-$y " >> $output
    fi
    printf "\n" >> $output
done

#printf "\n$(sed -n 1p $output) \n" >> $output
#dup=1
#printf "yo\n">> $output
#awk '{if(( NR == "$dup" )) print $0}' $output >> $output
#printf "yo\n">> $output
#reading each line
#printf "$((1 + $RANDOM % 9999)) $line \n" >> inputFile
#awk '{if(NR==1) print $0}' $file1

#while IFS= read -r line1 && IFS= read -r line2 <&3; do
#  echo "File 1: $line1"
#  echo "File 2: $line2"
#done < $file1 3< $file2

#head -n 1 $file1 | tail -n + 1
#i=$((i+1))
