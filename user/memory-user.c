#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc != 4) {
    printf("Usage: memory-user <start> <limit> <increment>, ...to allocate which is then incremented up to limit mebibytes\n");
    exit(-1);
  }

  uint start = atoi(argv[1]);
  uint limit = atoi(argv[2]);
  uint increment = atoi(argv[3]);
  uint i, j;

  int *array;

  for (i = start; i <= limit; i += increment) {
    uint bytes = i * 1024 * 1024;

    printf("allocating %d mebibytes\n", i);
    array = (int *)malloc(bytes);
    if (array == 0) {
      printf("malloc failed at %d MiB\n", i);
      exit(-1);
    }
    printf("malloc returned %p\n", array);

    //
    // CASE 1: do not touch the allocated memory
    //
    // (Leave this commented out when you want to test lazy allocation: free memory should not drop.)
    //
    // sleep(50);


    //
    // CASE 2: touch every page
    // Use this for Task 3/5 when you want to force page faults
    //
    for (j = 0; j < bytes / sizeof(int); j += 1024) {
      // write roughly once per 4 KB (one int per page)
      array[j] = j;
    }
    printf("Touched all pages in %d MiB\n", i);


    //
    // CASE 3: touch 1 out of every 16 pages (~6% of total)
    // Uncomment this instead of CASE 2 if you want that behavior later.
    //
    // for (j = 0; j < bytes / sizeof(int); j += 1024 * 16) {
    //   array[j] = j;
    // }
    // printf("Touched ~1/16 of pages in %d MiB\n", i);


    sleep(50);
    printf("freeing %d mebibytes\n", i);
    free(array);
    sleep(50);
  }

  exit(0);
}
