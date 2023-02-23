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

## 编程和测试

IO 500 负载从 https://io500.org/storage/sc21/2021.11.01-15.13.47/ 扒拉下来

核心参数：`-t` 代表 transfer size；`-b` 代表 block size，必须为 transfer size 整数倍

### 使用ior easy write写文件

```
mkdir -p build/file/ior-easy
ior -C -Q 1 -g -G 760427273 -k -e -o build/file/ior-easy/ior_file_easy -O stoneWallingStatusFile=build/file/ior-easy/stonewall -t 512k -b 10g -u -F -w -D 300 -O stoneWallingWearOut=300 -a POSIX
sudo pcm-memory 0.1 -csv=ior_easy_write_memory.csv -- ior -C -Q 1 -g -G 760427273 -k -e -o build/file/ior-easy/ior_file_easy -O stoneWallingStatusFile=build/file/ior-easy/stonewall -t 512k -b 8000m -u -F -w -D 300 -O stoneWallingWearOut=1 -a POSIX
```