#!/usr/bin/env python3
import pandas as pd
import tensorflow as tf
from sklearn.preprocessing import LabelEncoder

pd.set_option('display.float_format', lambda x: '%.5f' % x)

le = LabelEncoder()
le.fit([x.strip() for x in pd.read_csv('./stop_times.categories.tsv', sep='\t')['stop_headsign'].to_numpy()])

def load_data(path: str, le: LabelEncoder):
    train = pd.read_csv(path, sep='\t', dtype={ 'departure_time': float, 'stop_id': str, 'stop_headsign': str })

    departure_time = train['departure_time'].to_numpy()
    stop_id        = train['stop_id'].to_numpy()
    stop_headsign  = [x.strip() for x in train['stop_headsign'].to_numpy()]

    stop_headsign = le.transform(stop_headsign)

    x = [[d, int(s)] for d, s in zip(departure_time, stop_id)]
    return x, stop_headsign, le

num_classes = len(le.classes_)


def train(epochs: int):
    global le

    model = tf.keras.Sequential([
        tf.keras.layers.Input(shape=(2,)),
        tf.keras.layers.Dense(4 * num_classes, activation='relu'),
        tf.keras.layers.Dense(4 * num_classes, activation='relu'),
        tf.keras.layers.Dense(4 * num_classes, activation='relu'),
        tf.keras.layers.Dense(num_classes, activation='softmax')
    ])

    model.compile(optimizer='adam',
                  loss=tf.keras.losses.SparseCategoricalCrossentropy(),
                  metrics=['accuracy'])

    train_x, train_y, _ = load_data('./stop_times.train.tsv', le)
    train_x = tf.convert_to_tensor(train_x, dtype=tf.float32)
    train_y = tf.convert_to_tensor(train_y)

    valid_x, valid_y, _ = load_data('./stop_times.valid.tsv', le)
    valid_x = tf.convert_to_tensor(valid_x, dtype=tf.float32)
    valid_y = tf.convert_to_tensor(valid_y)

    model.fit(train_x, train_y, validation_data=(valid_x, valid_y), epochs=epochs, batch_size=1024)
    model.save('model.keras')

if __name__ == "__main__":
    import sys
    epochs = int('2' if len(sys.argv) != 2 else sys.argv[1])
    train(epochs)
