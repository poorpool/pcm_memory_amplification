#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <malloc.h>
#include <mpi/mpi.h>
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
  MPI_Init(&argc, &argv);
  int myid;
  int numprocs;
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  if (argc != 3) {
    printf("Usage: %s read_file_path transfer_size_in_kb\n", argv[0]);
    return 0;
  }
  char read_file_path[105];
  sprintf(read_file_path, "%s/%d/ior_file_easy.%08d", argv[1], myid, myid);
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
  MPI_Finalize();
  return 0;
}