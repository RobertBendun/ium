#!/bin/sh

cd /ium/src
python3 tf_train.py "$1"
cp /ium/src/model.keras /github/workspace/
