FROM ubuntu:latest

#set DEBIAN_FRONTEND=noninteractive to ignore user prompt
ENV DEBIAN_FRONTEND=noninteractive

# installing basic dependecies
RUN apt-get -y update \
    && apt-get -y install wget \
                          curl \
    && rm -r /var/lib/apt/lists/*

#installing latest cmake
RUN wget https://github.com/Kitware/CMake/releases/download/v3.22.3/cmake-3.22.3-linux-x86_64.sh \
      -q -O /tmp/cmake-install.sh \
      && chmod u+x /tmp/cmake-install.sh \
      && mkdir /usr/bin/cmake \
      && /tmp/cmake-install.sh --skip-license --prefix=/usr/bin/cmake \
      && rm /tmp/cmake-install.sh

ENV PATH="/usr/bin/cmake/bin:${PATH}"

#installing dependicies for mongo-c mongo-cxx and documentdb-odbc
RUN apt-get -y update \
    && apt-get -y install libcurl4-openssl-dev \
                          libssl-dev \
                          uuid-dev \
                          zlib1g-dev \
                          libpulse-dev \
                          gcc \
                          gcc-multilib  \
                          g++ \
                          g++-multilib \
                          build-essential \
                          valgrind \
                          libboost-all-dev \
                          libsasl2-dev \
                          libbson-dev \
                          lcov \
                          git \
                          unixodbc-dev \
                          gdb \
    && rm -r /var/lib/apt/lists/*

# compile and install mongoc
RUN MONGO_C_VERSION=1.21.1 \
    && cd /tmp \
    && curl -OL https://github.com/mongodb/mongo-c-driver/releases/download/$MONGO_C_VERSION/mongo-c-driver-$MONGO_C_VERSION.tar.gz \
    && tar xzf "mongo-c-driver-$MONGO_C_VERSION.tar.gz" \
    && rm "mongo-c-driver-$MONGO_C_VERSION.tar.gz" \
    && cd "mongo-c-driver-$MONGO_C_VERSION" \
    && mkdir cmake-build \
    && cd cmake-build \
    && cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF .. \
    && make \
    && make install \
    && rm -r /tmp/*

# compile and install mongocxx
RUN MONGO_CXX_VERSION=3.6.6 \
    && cd /tmp \
    && curl -OL https://github.com/mongodb/mongo-cxx-driver/releases/download/r$MONGO_CXX_VERSION/mongo-cxx-driver-r$MONGO_CXX_VERSION.tar.gz \
    && tar -xzf "mongo-cxx-driver-r$MONGO_CXX_VERSION.tar.gz" \
    && rm "mongo-cxx-driver-r$MONGO_CXX_VERSION.tar.gz" \
    && cd "mongo-cxx-driver-r$MONGO_CXX_VERSION/build" \
    && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr/local -DBSONCXX_POLY_USE_MNMLSTC=1 .. \
    && make \
    && make install \
    && rm -r /tmp/*

#installing dependecies for JDK
RUN apt-get -y update \
    && apt-get -y install software-properties-common \
    && rm -r /var/lib/apt/lists/*

#setup JDK
RUN  wget -O- https://apt.corretto.aws/corretto.key | apt-key add - \
     && apt-add-repository 'deb https://apt.corretto.aws stable main' \
     && apt-get update \
     && apt-get install -y java-17-amazon-corretto-jdk \
     && rm -r /var/lib/apt/lists/*

ENV JAVA_HOME /usr/lib/jvm/java-17-amazon-corretto/
ENV DOCUMENTDB_HOME /documentdb-odbc/build/odbc/bin/
ENV ODBC_LIB_PATH /documentdb-odbc/build/odbc/lib/

WORKDIR /documentdb-odbc
