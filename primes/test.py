#!/usr/bin/env python3

from subprocess import check_output

K = 5000000

# test_M = [1, 2, 3, K // 4, K // 2]
# test_M = [4, 5, 6, 7, 8]
# test_M = [10, 50, 100, 1000]
# test_M = [5000, 10000, 50000, 100000]
# test_M = [500000, 1000000]
# test_M = [300000, K]
test_M = [1, 2, 4, K // 10, K // 4, K // 2]

results = {}

for M in test_M:
  times = []
  for i in range(5):
    out = check_output(['primes', '+p3', str(K), str(M)]).decode('utf-8')
    time_line = [l for l in out.splitlines() if l.startswith('time=')][0]
    time = float(time_line.split('time=')[1])
    times.append(time)

  avgtime = sum(times) / len(times)
  results[M] = (times, avgtime)

print(results)
