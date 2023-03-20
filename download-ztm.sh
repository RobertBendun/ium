#!/usr/bin/env bash

set -e -o pipefail

keep=(stops.txt trips.txt stop_times.txt)

mkdir -p data && cd data
parallel -j 8 -- wget --no-verbose --no-clobber <../ztm-data.txt

for file in $(find . -name 'index*.zip'); do
	dir="${file##*=}"
	dir="${dir%.zip}"
	if [ ! -d "$dir" ]; then
		mkdir "$dir"
		unzip "$file" -d "$dir" "${keep[@]}"
	fi
done

cd ..

for k in "${keep[@]}"; do
	cat $(find data -name "$k") > "$k"
done
