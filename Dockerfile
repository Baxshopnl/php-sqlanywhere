FROM        debian:latest as sqlanywhere
ENV         SQL_ANYWHERE_CLIENT="http://d5d4ifzqzkhwt.cloudfront.net/sqla17client/sqla17_client_linux_x86x64.tar.gz"
RUN         set -ex \
            && apt-get update && apt-get install -y curl \
            && curl -sL ${SQL_ANYWHERE_CLIENT} | tar xz -C /tmp \
            && cd /tmp/client17* \
            && ./setup -install sqlany_client64 -silent -nogui -I_accept_the_license_agreement

FROM php:8.0-cli

ENV         LD_LIBRARY_PATH="/opt/sqlanywhere17/lib64"
ENV         SQLANY17="/opt/sqlanywhere17"

COPY        --from=sqlanywhere  /opt                /opt

RUN apt-get update && apt-get install -y wget gcc make zip unzip libtool

COPY src /opt/sqlanywhere

RUN cd /opt/sqlanywhere && phpize && ./configure && make
RUN echo "extension=/opt/sqlanywhere/.libs/sqlanywhere.so" >> /usr/local/etc/php/conf.d/sqlanywhere.ini

COPY test ~

CMD ["php", "~/test.php"]
