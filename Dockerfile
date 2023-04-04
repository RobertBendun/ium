FROM ubuntu:22.04

RUN apt update && apt install -y vim make python3 python3-pip python-is-python3 gcc g++ golang wget unzip
RUN pip install pandas matplotlib scikit-learn
CMD "bash"
