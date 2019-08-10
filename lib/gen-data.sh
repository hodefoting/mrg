#!/bin/bash

echo "typedef struct MrgData{const char *path;"
echo "int length; "
echo "const char *data;}MrgData;"
echo "static MrgData mrg_data[]={"

for a in $1/*;do
   b=`echo $a|sed s:data/::`;
   echo "{\"$b\", `wc -c $a|cut -f 1 -d ' '`,";
   cat $a | sed 's/\\/\\\\/g' |\
            sed 's/\r/a/'     |\
            sed 's/"/\\"/g'   |\
            sed 's/^/"/'      |\
            sed 's/$/\\n"/'
            echo "},"
done
echo "{\"\", 6, \"fnord\"}," ;
echo "{NULL,0,NULL}};";

