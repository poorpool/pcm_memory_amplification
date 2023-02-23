#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

constexpr int kPageSize = 4096;

long get_us() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000 + tv.tv_usec;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s read_file_path transfer_size_in_kb\n", argv[0]);
    return 0;
  }
  char *read_file_path = argv[1];
  int transfer_size_in_kb = atoi(argv[2]);
  int read_buffer_size = transfer_size_in_kb * 1024;
  printf("Read file %s with transfer_size %dkB\n", read_file_path,
         transfer_size_in_kb);

  int fd = open(read_file_path, O_RDONLY | O_SYNC); // ior -e
  void *buffer = memalign(kPageSize, read_buffer_size);
  ssize_t total_size = 0;
  ssize_t len;

  long start_us = get_us();
  do {
    len = read(fd, buffer, read_buffer_size);
    if (len < 0) {
      printf("read len %ld err\n", len);
    }
    total_size += len;
  } while (len > 0);
  long diff_us = get_us() - start_us;

  printf("read %ld B (%.4f MB) in %.4fs, %.4f MB/s\n", total_size,
         total_size / 1024.0 / 1024.0, diff_us / 1000000.0,
         total_size / 1024.0 / 1024.0 / (diff_us / 1000000.0));

  free(buffer);
  close(fd);
  return 0;
}