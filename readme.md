# cryptowatch 0.0.1 by [b](https://github.com/ebrian)
the mildy useful crypto ticker

cryptowatch is a simple cryptocurrency price ticker. i wrote it in c because if i didn't, linus torvalds would be upset with me.

![](https://i.imgur.com/XBcnvHl.png)

### install dependencies

__# ubuntu__
```sh
sudo apt-get install build-essential pkg-config libjson-c-dev libcurl4-openssl-dev
```

__# rhel__
```sh
yum groupinstall "Development tools"
yum install json-c-devel libcurl-devel
```

__# macos__
```sh
brew install gcc json-c pkg-config curl
```

### clone & compile (linux & macos)

```sh
git clone https://github.com/ebrian/cryptowatch
cd cryptowatch
make
./cryptowatch
```

### clone, compile and run (docker)

```sh
git clone https://github.com/ebrian/cryptowatch
cd cryptowatch
```

__# centos__
```sh
docker build -f docker/Dockerfile.centos7 -t cryptowatch .
docker run -ti --rm cryptowatch
```
or
```sh
docker build -f docker/Dockerfile.centos7 -t cryptowatch .
alias cryptowatch='docker run -ti --rm cryptowatch'
cryptowatch
```

__# ubuntu__
```sh
docker build -f docker/Dockerfile.ubuntu -t cryptowatch .
docker run -ti --rm cryptowatch
```
or
```sh
docker build -f docker/Dockerfile.ubuntu -t cryptowatch .
alias cryptowatch='docker run -ti --rm cryptowatch'
cryptowatch
```
