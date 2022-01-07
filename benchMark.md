# 测试工具

“wrk”和“[WebBench](https://github.com/linyacool/WebBench)”

# 物理环境（虚拟机环境）
- CPU: Intel(R) Xeon(R) Silver 4214Y CPU @ 2.20GHz
- Core Number: 8 Core
- Memory: 6G
- OS: Ubuntu 20.04.1 LTS
- VMware Workstation 16 Player

# 测试场景

模拟1000条TCP连接，持续时间60s

# 测试结果

## 1个工作线程
### wrk
`./wrk -c 1000 -t 10 --latency -d 60 http://127.0.0.1:12345/index.html`
```
Running 1m test @ http://127.0.0.1:12345/index.html
  10 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   108.84ms   19.17ms 528.86ms   90.09%
    Req/Sec     0.88k   166.73     3.79k    90.09%
  Latency Distribution
     50%  110.90ms
     75%  113.51ms
     90%  117.27ms
     99%  139.39ms
  523985 requests in 1.00m, 324.81MB read
  Socket errors: connect 0, read 523985, write 0, timeout 0
Requests/sec:   8720.60
Transfer/sec:      5.41MB
```

### WebBench
`./bin/webbench -t 60 -c 1000 -2 --get http://127.0.0.1:12345/index.html`
```
Speed=257668 pages/min, 2791395 bytes/sec.
Requests: 257668 susceed, 0 failed.
```

## 4个工作线程
### wrk
`./wrk -c 1000 -t 10 --latency -d 60 http://127.0.0.1:12345/index.html`
```
Running 1m test @ http://127.0.0.1:12345/index.html
  10 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    17.58ms   76.07ms   1.73s    95.94%
    Req/Sec     3.07k     2.06k   14.61k    71.22%
  Latency Distribution
     50%    3.83ms
     75%    7.86ms
     90%   15.43ms
     99%  351.65ms
  1730668 requests in 1.00m, 1.05GB read
  Socket errors: connect 0, read 1730670, write 0, timeout 39
Requests/sec:  28798.39
Transfer/sec:     17.85MB
```

### WebBench
`./bin/webbench -t 60 -c 1000 -2 --get http://127.0.0.1:12345/index.html`
```
Speed=1546722 pages/min, 16756124 bytes/sec.
Requests: 1546722 susceed, 0 failed.
```

## 16个工作线程（超过了CPU核心数）
### wrk
`./wrk -c 1000 -t 10 --latency -d 60 http://127.0.0.1:12345/index.html`
```
Running 1m test @ http://127.0.0.1:12345/index.html
  10 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    10.48ms   73.52ms   1.70s    96.87%
    Req/Sec     2.64k     1.80k   15.51k    71.87%
  Latency Distribution
     50%  665.00us
     75%    1.00ms
     90%    1.73ms
     99%  234.82ms
  1538646 requests in 1.00m, 0.93GB read
  Socket errors: connect 0, read 1538644, write 0, timeout 59
Requests/sec:  25603.50
Transfer/sec:     15.87MB
```

### WebBench
`./bin/webbench -t 60 -c 1000 -2 --get http://127.0.0.1:12345/index.html`
```
Speed=1563203 pages/min, 16934658 bytes/sec.
Requests: 1563203 susceed, 0 failed.
```
