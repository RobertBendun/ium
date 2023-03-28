#!/usr/bin/env bash

set -xe -o pipefail

# Disable to allow to work in bare jenkins
# make normalize csv2tsv/csv2tsv

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

k=$keep
csv="${k%.txt}.csv"
cat $(find data -name "$k") | shuf > "$csv"
train_size=$(( $(wc -l "$csv" | cut -f1 -d' ') * 8 / 10 ))
echo $train_size

head -n $train_size  $csv >train.csv
tail -n +$train_size $csv >test.csv

# Disable to allow to work in bare jenkins
# for k in "${keep[@]}"; do
# 	csv="${k%.txt}.csv"
# 	tsv="${k%.txt}.tsv"
# 	if [ ! -f "$tsv" ]; then
# 		cat $(find data -name "$k") > "$csv"
# 		csv2tsv/csv2tsv <"$csv" >"$tsv"
# 	fi
# done
#
# if [ ! -f "stop_times.normalized.tsv" ]; then
# 	./normalize <stop_times.tsv >stop_times.normalized.tsv
# 	./split_train_valid_test.py
# fi
# ./stats.py
