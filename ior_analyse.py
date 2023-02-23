import csv
import sys
import datetime
from statistics import mean

if len(sys.argv) != 3:
  print("usage: python %s csv_name time_in_seconds" % sys.argv[0])
  exit(0)
time_in_seconds = float(sys.argv[2])
print("%s in %f seconds" % (sys.argv[1], time_in_seconds))
csv_reader = csv.reader(open(sys.argv[1]))
tme = []
read_mb = []
write_mb = []

i = 0
for line in csv_reader:
  i = i + 1
  if i >= 3:
    tme.append(line[1])
    read_mb.append(float(line[-3]))
    write_mb.append(float(line[-2]))
end_time = datetime.datetime.strptime(tme[-1], "%H:%M:%S.%f")
i = 0
print((end_time - datetime.datetime.strptime(tme[i], "%H:%M:%S.%f")).total_seconds())
while (end_time - datetime.datetime.strptime(tme[i], "%H:%M:%S.%f")).total_seconds() > time_in_seconds:
  i = i + 1

print("Read %f MB/s" % mean(read_mb[i:]))
print("Write %f MB/s" % mean(write_mb[i:]))

