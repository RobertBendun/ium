# # Prune poorly filled or uninteresting columns
# languages.pruned1.tsv: languages.original.tsv
# 	cut --complement -f1,16-18,22,23,39,40,50,52,53,58-61,64-68,70-75,79,82,84-86,100,103-104,107-109,112,114,116-118,121-136,142,149,150,157,162,163,163,164,165,176,177,185,186,195, $< >$@

# Allow user to mark in which columns is interested
columns.user.tsv: columns.pruned.tsv
	awk 'NR==1{ printf("Keep\t"); print } NR>1{printf("n\t"); print}' $< >$@

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
