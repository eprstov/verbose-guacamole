# coinbase sample feeder
#### producer
- connects to the [Coinbase websocket ticker channel](https://docs.cloud.coinbase.com/exchange/docs/websocket-channels#ticker-channel)
- subscribes to a specific symbol
- sends ticks over the network to a specific UDP endpoint

#### consumer
- receives ticks from a specific UDP endpoint
- prints them out to stdout

### how to build
1. create a docker image using the Dockerfile provided
2. in the docker container from within the source directory run `cmake -B.make && make -C.make -j4`
3. the executables now are at `.make/producer` and `.make/consumer`

### how to run
#### producer
```
./producer BTC-USD 127.0.0.1:5555
```

#### consumer
```
./consumer 0.0.0.0:5555
```

