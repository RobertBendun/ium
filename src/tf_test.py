from tf_train import *
import numpy as np

def test():
    global model, le
    test_x, test_y, _ = load_data('./stop_times.test.tsv', le)
    test_x = tf.convert_to_tensor(test_x, dtype=tf.float32)
    test_y = tf.convert_to_tensor(test_y)

    model = tf.keras.models.load_model('model.keras')
    pd.DataFrame(model.predict(test_x), columns=le.classes_).to_csv('stop_times.predictions.tsv', sep='\t')


if __name__ == "__main__":
    test()
