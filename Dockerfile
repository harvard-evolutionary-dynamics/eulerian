FROM ubuntu:16.04

WORKDIR /harvard-evolutionary-dynamics/eulerian

RUN apt-get -y update    \
 && apt-get -y install   \
      bash               \
      build-essential    \
      git                \
      python3            \
      python3-dev        \
      python3-pip        \
      time               \
 && python3 -m pip install --upgrade pip \
 && python3 -m pip install   \
      cmake==3.17.3          \
      setuptools==46.4.0     \
      twine==1.15.0

COPY . .

RUN rm -rf cmake-build-{debug,release} dist eulerian.egg-info build
RUN python3 setup.py install --verbose
