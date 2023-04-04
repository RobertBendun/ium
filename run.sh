#!/bin/sh

set -xe

docker build -t ium .
docker run -it ium
