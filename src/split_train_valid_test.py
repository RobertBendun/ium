#!/usr/bin/env python3
import pandas as pd
from sklearn.model_selection import train_test_split

data = pd.read_csv('./stop_times.normalized.tsv', sep='\t')

train, test = train_test_split(data, test_size=0.5)
valid, test = train_test_split(test, test_size=0.5)

train.to_csv('stop_times.train.tsv', sep='\t')
test.to_csv('stop_times.test.tsv', sep='\t')
valid.to_csv('stop_times.valid.tsv', sep='\t')
