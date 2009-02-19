#!/bin/bash

wavefile=out.wav
soxdfile=sox.dat
gnupfile=gnu.dat

lowerboundlevel=-0.3
upperboundlevel=0.3

deltalongtime=0.005
deltashorttime=0.0009

sox $wavefile $soxdfile

tail -n +3 $soxdfile > $gnupfile

plotheader="# gnuplot file\n
set title 'SoX'\n
set grid xtics ytics\n
set key off\n
set style data points\n
plot '$gnupfile'\n"

# echo -e $plotheader | gnuplot

declare -a timearray
declare -a amplitude
declare -a code

echo "Read file $gnupfile"
timearray=($(cat "$gnupfile" | tr -s [:blank:] \\t | cut -f 2))
amplitude=($(cat "$gnupfile" | tr -s [:blank:] \\t | cut -f 3))


element_count_timearray=${#timearray[*]}
element_count_amplitude=${#amplitude[*]}

echo -e "Compute data..."
if [ $element_count_timearray -ne $element_count_amplitude ]
then
  echo fileformat error
  exit 1
fi

x=0
y=0
z=0upperboundlevel
j=0
k=0
countrange=0
let element_count_timearray=element_count_timearray-2
for i in `seq 0 $element_count_timearray`
do
  rangelag="$(echo "((${amplitude[$i]})<=($upperboundlevel))&&((${amplitude[$i]})>=($lowerboundlevel))" | bc -l 2>/dev/null)"
  if [ $rangelag -ne 0 ] 2>/dev/null
  then
    let countrange=$countrange+1
  else
    if [ $countrange -eq 0 ]
    then
      let j=i
    fi
    if [ $countrange -gt 70 ]
    then
      z=$( echo "${timearray[$i]}-${timearray[$j]}"|bc )
      y=$( echo "${timearray[$j]}-${timearray[$k]}-$z"|bc )
      flag=$(echo "$y<$deltashorttime"|bc)
      if [ $flag -ne 0 ]
      then
	code[$x]=0
	echo -n ${code[$x]}
	let dx=j-countrange/2
	plotheader=$(echo -e "$plotheader\nset label '${code[$x]}' at ${timearray[$dx]},0.95\n")
	let x=x+1
      else
	flag=$(echo "$y<$deltalongtime"|bc)
	if [ $flag -ne 0 ]
	then
# 	  code[$x]=1
# 	  echo -n ${code[$x]}
	  plotheader=$(echo -e "$plotheader\nset label '1' at ${timearray[$j]},0.9\n")
# 	  let x=x+1
	fi
      fi
      echo -e "\n${timearray[$i]} $y $z $countrange"
      let k=j
      code[$x]=1
      echo -n ${code[$x]}
      let dx=j+countrange/2
      plotheader=$(echo -e "$plotheader\nset label '${code[$x]}' at ${timearray[$dx]},0.95\n")
      let x=x+1
    elif [ $countrange -gt 30 ]
    then
      z=$( echo "${timearray[$i]}-${timearray[$j]}"|bc )
      y=$( echo "${timearray[$j]}-${timearray[$k]}-$z"|bc )
      flag=$(echo "$y<$deltashorttime"|bc)
      if [ $flag -ne 0 ]
      then
	code[$x]=0
	echo -n ${code[$x]}
	plotheader=$(echo -e "$plotheader\nset label '0' at ${timearray[$j]},0.8\n")
	let x=x+1
      else
	flag=$(echo "$y<$deltalongtime"|bc)
	if [ $flag -ne 0 ]
	then
	  code[$x]=1
	  echo -n ${code[$x]}
	  let dx=j-countrange
	  plotheader=$(echo -e "$plotheader\nset label '${code[$x]}' at ${timearray[$dx]},0.95\n")
	  let x=x+1
	fi
      fi
      echo -e "\n${timearray[$i]} $y $z $countrange"
      let k=j
      code[$x]=0
      echo -n ${code[$x]}
      let dx=j+countrange/2
      plotheader=$(echo -e "$plotheader\nset label '${code[$x]}' at ${timearray[$j]},0.95\n")
      let x=x+1
    fi
    countrange=0
    if [ $x -eq 1 ]
    then
      if [ $i -gt 500 ]
      then
	let startcnt=i-500
      else
	let startcnt=i
      fi
      starttime=${timearray[$startcnt]}
    fi
    let endcnt=k
    endtime=${timearray[$endcnt]}
  fi
  
done


element_count_codearray=${#code[*]}
echo -e "\nCodes:"
BIN=""
for i in `seq 0 7`;
do
  BIN=$(echo $BIN${code[i]})
done
DEC1=$(echo "ibase=2; $BIN" | bc)
echo "bin $BIN = dec $DEC1"

BIN=""
for i in `seq 8 15`;
do
  BIN=$(echo $BIN${code[i]})
done
DEC1=$(echo "ibase=2; $BIN" | bc)
echo "bin $BIN = dec $DEC1"

BIN=""
for i in `seq 16 23`;
do
  BIN=$(echo $BIN${code[i]})
done
DEC1=$(echo "ibase=2; $BIN" | bc)
echo "bin $BIN = dec $DEC1"


BIN=""
for i in `seq 25 32`;
do
  BIN=$(echo $BIN${code[i]})
done
DEC1=$(echo "ibase=2; $BIN" | bc)
echo "bin $BIN = dec $DEC1"

BIN=""
for i in `seq 33 40`;
do
  BIN=$(echo $BIN${code[i]})
done
DEC1=$(echo "ibase=2; $BIN" | bc)
echo "bin $BIN = dec $DEC1"


echo -e "$plotheader\n
replot\n
pause -1 'Hit return to continue'" > gnuplot.in #| gnuplot
echo -e "call:\n gnuplot gnuplot.in"
echo $starttime $startcnt
let endcnt=endcnt-startcnt+500
echo $endtime $endcnt
# tail -n +$startcnt $soxdfile | head -n $endcnt > $gnupfile
