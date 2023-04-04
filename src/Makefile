normalize: normalize.cc
	g++ -std=c++20 -Wall -Wextra -O3 -o $@ $<

csv2tsv/csv2tsv: csv2tsv/csv2tsv.go
	cd csv2tsv; go build

clean:
	rm -f *.csv *.tsv stop*.txt trips.txt
