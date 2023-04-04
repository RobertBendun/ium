#!/usr/bin/env python3
import math
import os
import pandas as pd
import contextlib

pd.set_option('display.float_format', lambda x: '%.5f' % x)

def float2time(d: float):
    hours = math.floor(d * 24)
    minutes = math.floor(d * 24 * 60 - hours * 60)
    return "%s:%s" % tuple(str(x).rjust(2,'0') for x in (hours, minutes))

data = pd.read_csv(f'./stop_times.normalized.tsv', sep='\t', dtype={ 'departure_time': float, 'stop_id': str, 'stop_headsign': str })

print("--- Pictures -------------------------------------------------")

with contextlib.suppress(Exception):
    os.mkdir("pics")

(data["departure_time"] * 24).plot(kind='hist', title="Częstotliwość czasu odjazdu").get_figure().savefig('pics/departure_time_frequency.png')
print("pics/departure_time_frequency.png")
data["stop_headsign"].value_counts().plot(kind='pie', title="Popularność celu").get_figure().savefig('pics/stop_headsign_popularity.png')
print("pics/stop_headsign_popularity.png")


print("--- Minmum departure time per stop headsign ------------------")
shgroup = data.groupby('stop_headsign').min(numeric_only=True)
shgroup["departure_time"] = shgroup["departure_time"].map(float2time)
print(shgroup)
print()

print("--- Maximum departure time per stop headsign -----------------")
shgroup = data.groupby('stop_headsign').max(numeric_only=True)
shgroup["departure_time"] = shgroup["departure_time"].map(float2time)
print(shgroup)
print()

print("--- Mean departure time per stop headsign --------------------")
shgroup = data.groupby('stop_headsign').mean(numeric_only=True)
shgroup["departure_time"] = shgroup["departure_time"].map(float2time)
print(shgroup)
print()

print("--- Normalized data statistics -------------------------------")
print(data.describe(include='all'))

for subset in ['train', 'valid', 'test']:
    print(f"--- {subset.title()} data statistics -------------------------------")
    data = pd.read_csv(f'./stop_times.{subset}.tsv', sep='\t', dtype={ 'departure_time': float, 'stop_id': str, 'stop_headsign': str })
    print(data.describe(include='all'))
