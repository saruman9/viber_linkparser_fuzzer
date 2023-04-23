#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

int afl_libfuzzer_init()
{
  return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  return 0;
}
