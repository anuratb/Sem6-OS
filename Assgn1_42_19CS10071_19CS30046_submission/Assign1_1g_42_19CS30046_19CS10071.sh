
for i in $(seq 150);
do
    for i in $(seq 9);
    do
        echo -n "$RANDOM,"
    done
    echo "$RANDOM"
done |tr -s [:blank:] ',' > Data.csv

read -p "Name of file: " file;
read -p "Column number: " col;
read -p "Regular expression: " pattern;
flg=false
file=$(echo $file |sed -r "s/^\"\s*//"|sed -r "s/\s*\"$//" )
pattern=$(echo $pattern |sed -r "s/^\"\s*//"|sed -r "s/\s*\"$//" )
for i in $(awk    '$(col) ~ pattern  {system("echo true")}' FS=',' col="$col" pattern="$pattern"   $file);
do $i && flg=true
done
$flg && echo "YES" || echo "NO"