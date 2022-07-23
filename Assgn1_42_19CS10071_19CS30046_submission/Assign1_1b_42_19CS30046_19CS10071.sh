#cd 1.b.files
mkdir 1.b.files.out
file_res="1.b.out.txt"
for file in 1.b.files/*.txt
do
	cat $file | sort -n > 1.b.files.out/$(basename $file);
	cat $file >> $file_res;
done
	sort < $file_res \
	| uniq -c \
	| awk '{print $2"\t"$1}' \
	| sort -k 1 -n > $(basename $file_res)
