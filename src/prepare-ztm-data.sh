#!/usr/bin/env bash

set -xe -o pipefail

# Go requires presence of this variable with some sensible directory
export XDG_CACHE_HOME="/tmp/xdg_cache_home/"
mkdir -p "$XDG_CACHE_HOME"

make normalize csv2tsv/csv2tsv

keep=stop_times.txt

mkdir -p data && cd data
xargs -- wget --no-verbose --no-clobber <../ztm-data.txt

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
	csv="${k%.txt}.csv"
	tsv="${k%.txt}.tsv"
	if [ ! -f "$tsv" ]; then
		cat $(find data -name "$k") > "$csv"
		csv2tsv/csv2tsv <"$csv" >"$tsv"
	fi
done

if [ ! -f "stop_times.normalized.tsv" ]; then
	./normalize <stop_times.tsv >stop_times.normalized.tsv
fi

if [ ! \( -f "stop_times.train.tsv" -a -f "stop_times.test.tsv" -a -f "stop_times.valid.tsv" \) ]; then
	./split_train_valid_test.py
fi

cut -f3 stop_times.normalized.tsv | head -n1 > stop_times.categories.tsv
cut -f3 stop_times.normalized.tsv | tail -n +2 | uniq | sort | uniq >> stop_times.categories.tsv
