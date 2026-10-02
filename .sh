find include src -type f -exec sh -c 'for file do
	printf "// %s\n" "$file"
	cat "$file"
	printf "\n"
done' sh {} + > reflect.txt