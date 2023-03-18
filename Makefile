# Remove all columns according to specification in columns.pruned.tsv
languages.pruned.tsv: languages.original.tsv columns.user.tsv
	bash -c "cut -f`grep '^y' columns.user.tsv | cut -f2 | paste -sd ','` $< >$@"

# Allow user to mark in which columns is interested
columns.user.tsv: columns.pruned.tsv
	if [ -f $@ ]; then touch $@; else awk 'NR==1{ printf("Keep\tType\t"); print } NR>1{printf("n\tstr\t"); print}' $< >$@; fi

# Prune columns that are not needed to create specification
columns.pruned.tsv: columns.original.tsv
	cut --complement -f3,4,5,7,8,9,10 $< >$@

# Change data to TSV format since it is easier to process using standard UNIX tools
%.tsv: %.csv
	go run ./csv2tsv.go <$< >$@

# Check while downloading that file is as expected.
# Otherwise automatic filter mechanism wouldn't work.
# If hashes differ then user of this repo must migrate columns.user.tsv to a new format
columns.original.csv:
	wget 'https://pldb.com/columns.csv' -O $@
	sha256sum -c checksums.sha256

languages.original.csv: columns.original.csv
	wget 'https://pldb.com/languages.csv' -O $@

clean:
	rm -f languages.*.tsv languages.*.csv columns.original.tsv columns.*.csv columns.pruned.tsv

.PHONY: clean
