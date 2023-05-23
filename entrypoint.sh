#!/bin/sh

cd /ium/src

if [ -f /github/workspace/model.keras ]; then
	# Since model exists we already did the training
	cp /github/workspace/model.keras /ium/src/
	python3 tf_test.py "$1"
else
	python3 tf_train.py "$1"
	cp /ium/src/model.keras /github/workspace/
fi
