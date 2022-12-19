FROM debian:buster

EXPOSE 80

COPY . /gdb-frontend-live
COPY ./container/nginx/gflive.conf /etc/nginx/sites-available/gflive.conf

RUN apt update -y && \
    apt upgrade -y && \
    apt install -y gcc \
                build-essential \
                libssl-dev \
                uuid-dev \
                zlib1g-dev \
                make \
                gdb \
                tmux \
                procps \
                python3 \
                nginx

RUN adduser gflive

RUN rm /etc/nginx/sites-enabled/default
RUN ln -s /etc/nginx/sites-available/gflive.conf /etc/nginx/sites-enabled/gflive.conf

WORKDIR /gdb-frontend-live

RUN make

CMD ./container/startup.sh