from tf_train import *
import numpy as np
from sklearn.metrics import accuracy_score

def test():
    global model, le
    test_x, test_y, _ = load_data('./stop_times.test.tsv', le)
    test_x = tf.convert_to_tensor(test_x, dtype=tf.float32)
    test_y = tf.convert_to_tensor(test_y)

    model = tf.keras.models.load_model('model.keras')
    predictions = np.argmax(model.predict(test_x), 1)

    with open('stop_times.predictions.tsv', 'w') as f:
        f.write('stop_headsign\n')
        for x in le.inverse_transform(predictions):
            print(x, file=f)

    with open('stop_times.accuracy.tsv', 'a') as f:
        print(accuracy_score(test_y, predictions), file=f, sep='\t')


if __name__ == "__main__":
    test()
