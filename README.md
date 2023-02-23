# 使用 pcm 测试 ior 的内存放大

## 测试目的

ior 在读的时候会产生读校验，产生对内存的访问。为了具体测量读校验对内存带宽的影响，看看读校验使用了多少内存带宽，特此测试。

## 测试方案

内存放大比：在 ior 程序读写期间，`内存读写的速率 / 磁盘IO的速率`。当然用总量也是可以的，但是不够直观，也不好测。

ior 会给出磁盘 IO 速率和读写所用时间。使用 pcm-memory 工具采样 ior 程序执行期间的内存读写速率，进行平均，得到内存读写的速率。

为了排除 VFS 的影响，需要

1. 使用ior easy write写文件
2. 测得ior easy read的内存放大比
3. 自己写一个无读校验的读文件程序（即每次从文件中读取 transferSize 大小的数据进入内存），测这个程序的内存放大比
4. 将ior easy read的内存放大比减去自己读文件程序的内存放大比，分析结果

注：TransferSize 可理解为 io buffer 的大小，blockSize 为一个 client 接触到文件的大小。详见 [ior说明](https://buildmedia.readthedocs.org/media/pdf/ior/latest/ior.pdf)

## pcm ior 安装

pcm：https://github.com/intel/pcm 下载 deb 包安装

ior：

```bash
wget https://github.com/hpc/ior/releases/download/3.3.0/ior-3.3.0.tar.gz
./configure
make -j16
sudo make install
```

## 单进程编程和测试（废弃）

IO 500 负载从 https://io500.org/storage/sc21/2021.11.01-15.13.47/ 扒拉下来

核心参数：`-t` 代表 transfer size；`-b` 代表 block size，必须为 transfer size 整数倍

### 单进程使用ior easy write写文件

transfer size 512 kB，文件大小 20g

```bash
mkdir -p build/file/ior-easy
ior -C -Q 1 -g -G 760427273 -k -e -o build/file/ior-easy/ior_file_easy -O stoneWallingStatusFile=build/file/ior-easy/stonewall -t 512k -b 20g -u -F -w -D 300 -O stoneWallingWearOut=300 -a POSIX
```

### 单进程使用ior easy read 读文件

采样间隔 0.01s

```bash
ior -C -Q 1 -g -G 760427273 -k -e -o build/file/ior-easy/ior_file_easy -O stoneWallingStatusFile=build/file/ior-easy/stonewall -t 512k -b 20g -u -F -r -R -a POSIX
```

带 pcm 监测版：

```
sudo pcm-memory 0.01 -csv=ior_easy_read_memory.csv -- ior -C -Q 1 -g -G 760427273 -k -e -o build/file/ior-easy/ior_file_easy -O stoneWallingStatusFile=build/file/ior-easy/stonewall -t 512k -b 20g -u -F -r -R -a POSIX
```

- IO 3962.35 MB/s，用时 5.17s
- 内存读 4558.805686 MB/s
- 内存写 306.817627 MB/s

内存放大 1.15 倍

### 单进程使用ior easy read 读文件（无读校验）

删掉 -R

```bash
ior -C -Q 1 -g -G 760427273 -k -e -o build/file/ior-easy/ior_file_easy -O stoneWallingStatusFile=build/file/ior-easy/stonewall -t 512k -b 20g -u -F -r -a POSIX
```

带 pcm 监测版：

```
sudo pcm-memory 0.01 -csv=ior_easy_read_no_check_memory.csv -- ior -C -Q 1 -g -G 760427273 -k -e -o build/file/ior-easy/ior_file_easy -O stoneWallingStatusFile=build/file/ior-easy/stonewall -t 512k -b 20g -u -F -r -a POSIX
```

- IO 6082.58 MB/s，用时 3.37s
- 内存读 6817.510542 MB/s
- 内存写 424.239940 MB/s

内存放大 1.13 倍

### 单进程使用自制程序读文件

编译：

```bash
g++ -o simple_read simple_read.cc -Wall -O3
```

```bash
./simple_read build/file/ior-easy/0/ior_file_easy.00000000 512
```

```bash
sudo pcm-memory 0.01 -csv=my_no_check_memory.csv -- ./simple_read build/file/ior-easy/0/ior_file_easy.00000000 512
```

- IO 6084.0586 MB/s，用时 3.3662s
- 内存读 6883.567108 MB/s
- 内存写 326.597349 MB/s

内存放大 1.13 倍

## 二十进程版测试

transfer size 5mb, file size 5 gb

### 写

```bash
mkdir -p build/file/ior-easy
mpirun -np 20 taskset -c 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20 ior -C -Q 1 -g -G 760427273 -k -e -o build/file/ior-easy/ior_file_easy -O stoneWallingStatusFile=build/file/ior-easy/stonewall -t 5m -b 5g -u -F -w -D 300 -O stoneWallingWearOut=300 -a POSIX
```

### ior 读校验读

```
sudo pcm-memory 0.01 -csv=ior_easy_read_memory.csv -- mpirun --allow-run-as-root -np 20 taskset -c 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20 ior -C -Q 1 -g -G 760427273 -k -e -o build/file/ior-easy/ior_file_easy -O stoneWallingStatusFile=build/file/ior-easy/stonewall -t 5m -b 5g -u -F -r -R -a POSIX
```

- IO 3005.86 MB/s，用时 34.06681s
- 内存读 12071.264334 MB/s
- 内存写 14779.540480 MB/s

内存读放大比 4.016

### ior 没有读校验读

```
sudo pcm-memory 0.01 -csv=ior_easy_read_no_check_memory.csv -- mpirun --allow-run-as-root -np 20 taskset -c 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20 ior -C -Q 1 -g -G 760427273 -k -e -o build/file/ior-easy/ior_file_easy -O stoneWallingStatusFile=build/file/ior-easy/stonewall -t 5m -b 5g -u -F -r -a POSIX
```

- IO 3049.66 MB/s，用时 33.57757s
- 内存读 5593.270511 MB/s
- 内存写 11028.040336 MB/s

内存放大比 1.834

### 使用自制程序读文件

```bash
mpicxx -o simple_read simple_read.cc -Wall -O3
```

```bash
sudo pcm-memory 0.01 -csv=my_no_check_memory.csv -- mpirun --allow-run-as-root -np 20 taskset -c 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20 ./simple_read build/file/ior-easy 5120
```

- IO 2465.60 MB/s，用时 41.5314s
- 内存读 5021.576476 MB/s
- 内存写 8952.389107 MB/s

内存放大比 2.03

## 结论

自己写测试程序，和去掉 ior 的 -R，差不多，反正没了读校验都差不多。

在 20 进程条件下，去除 VFS 开销，IOR 读校验内存放大开销 2 倍。在单进程条件，只有0.02倍。

