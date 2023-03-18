package main

import (
	"encoding/csv"
	"fmt"
	"io"
	"os"
)

func main() {
	reader := csv.NewReader(os.Stdin)
	reader.FieldsPerRecord = -1
	reader.ReuseRecord = true

loop:
	for lineno := 0; ; lineno++ {
		record, err := reader.Read()
		switch err {
		case io.EOF:
			break loop
		default:
			fmt.Fprintf(os.Stderr, "[WARNING] Couldn't read line %d: %v\n", lineno, err)
		case nil:
		}
		for i, entry := range record {
			if i != 0 {
				fmt.Print("\t")
			}
			fmt.Print(entry)
		}
		fmt.Println()
	}
}
