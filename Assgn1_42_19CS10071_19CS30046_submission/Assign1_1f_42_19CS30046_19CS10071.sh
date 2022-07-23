
awk -F" " '{print $col}' col=$2 $1 \
    | tr '[:upper:]' '[:lower:]'  \
	| sort  \
	| uniq -c \
	| awk '{print $2"\t"$1}' \
	| sort -k 2 -nr > "1c_output_$2_column.freq"
