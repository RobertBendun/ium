#!/bin/sh

set -xe

docker build -t ium .
docker run -v .:/ium/ -it ium
