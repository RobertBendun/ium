FROM ubuntu:22.04

RUN apt update && apt install -y vim make python3 python3-pip python-is-python3 gcc g++ golang wget unzip git
RUN pip install pandas matplotlib scikit-learn tensorflow

RUN mkdir /ium
COPY . /ium

WORKDIR /ium/src/

RUN wget https://bendun.students.wmi.amu.edu.pl/stop_times.train.tsv
RUN wget https://bendun.students.wmi.amu.edu.pl/stop_times.valid.tsv
RUN wget https://bendun.students.wmi.amu.edu.pl/stop_times.test.tsv
RUN wget https://bendun.students.wmi.amu.edu.pl/stop_times.categories.tsv

VOLUME /github/workspace/

ENTRYPOINT ["/ium/entrypoint.sh"]
