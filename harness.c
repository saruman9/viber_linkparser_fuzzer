#include <stdint.h>
#include <stddef.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

#include "liblinkparser.so.h"
#include "utf8.h"

const ptrdiff_t ADDR_JNI_ONLOAD = 0x0000000000011640;
const ptrdiff_t ADDR_PARSE_LINK = 0x000000000002F870;
const ptrdiff_t ADDR_COPY_JNI_STRING_FROM_STR = 0x0000000000011160;
const ptrdiff_t ICU_SQLITE_ANDROID_BINDER__INIT = 0x0000714788297F90;
const ptrdiff_t g_ICU_VERSION = 0x00007147882A9000;
const uint32_t ICU_VERSION = 66;

static void *LIBC_SHARED = NULL;
static void *LIBICU_BINDER = NULL;
static void *LIBLINKPARSER = NULL;

bool is_ascii_str(char *str, size_t len)
{
  for (int i = 0; i < len; i++)
  {
    if (!isascii(str[i]))
    {
      return false;
    }
  }
  return true;
}

typedef struct Functions
{
  void (*parse_link)(struct ParserResult *, struct String *);
  void (*copy_jni_string_from_str)(struct String *, const char *);
} Functions;

int afl_libfuzzer_init()
{
  return 0;
}

void set_icu_version(ptrdiff_t binder_init_addr)
{
  ptrdiff_t diff = g_ICU_VERSION - ICU_SQLITE_ANDROID_BINDER__INIT;
  ptrdiff_t version_addr = binder_init_addr + diff;
#ifdef TRIAGE
  printf("[i] original ICU_VERSION = %X\n", *(uint32_t *)version_addr);
#endif
  *(uint32_t *)version_addr = ICU_VERSION;
  return;
}

struct Functions *load_functions()
{
  LIBC_SHARED = dlopen("/data/local/tmp/libc++_shared.so", RTLD_NOW | RTLD_GLOBAL);
  LIBICU_BINDER = dlopen("/data/local/tmp/libicuBinder.so", RTLD_NOW | RTLD_GLOBAL);
  LIBLINKPARSER = dlopen("/data/local/tmp/liblinkparser.so", RTLD_NOW | RTLD_GLOBAL);
  if (LIBLINKPARSER != NULL && LIBC_SHARED != NULL && LIBICU_BINDER != NULL)
  {
    int (*JNI_OnLoad)(void *, void *) = dlsym(LIBLINKPARSER, "JNI_OnLoad");
    void (*binder_init)() = dlsym(LIBICU_BINDER, "_ZN22IcuSqliteAndroidBinder4initEv");

    if (JNI_OnLoad != NULL && binder_init != NULL)
    {
      Dl_info jni_on_load_info;
      dladdr(JNI_OnLoad, &jni_on_load_info);
      size_t jni_on_load_addr = (size_t)jni_on_load_info.dli_saddr;

      Dl_info binder_init_info;
      dladdr(binder_init, &binder_init_info);
      size_t binder_init_addr = (size_t)binder_init_info.dli_saddr;
      set_icu_version(binder_init_addr);

      int diff_parse_link = ADDR_PARSE_LINK - ADDR_JNI_ONLOAD;
      int diff_copy_jni_string_from_str = ADDR_COPY_JNI_STRING_FROM_STR - ADDR_JNI_ONLOAD;
      size_t parse_link_addr = jni_on_load_addr + diff_parse_link;
      size_t copy_jni_string_from_str_addr = jni_on_load_addr + diff_copy_jni_string_from_str;
#ifdef TRIAGE
      printf("[i] parse_link_addr: %zX\n", parse_link_addr);
      printf("[i] copy_jni_string_from_str_addr: %zX\n", copy_jni_string_from_str_addr);
#endif
      void (*parse_link)(ParserResult *, String *) = (void (*)(ParserResult *, String *))(parse_link_addr);
      void (*copy_jni_string_from_str)(String *, const char *) = (void (*)(String *, const char *))(copy_jni_string_from_str_addr);
      if (parse_link != NULL && copy_jni_string_from_str != NULL)
      {
        Functions *functions = (Functions *)malloc(sizeof(Functions));
        functions->parse_link = parse_link;
        functions->copy_jni_string_from_str = copy_jni_string_from_str;
        return functions;
      }
      else
      {
        perror("[-] Can't find parse_link function");
        return NULL;
      }
    }
    else
    {
      perror("[-] Can't find symbols (dlsym)");
      return NULL;
    }
  }
  else
  {
    fprintf(stderr, "[-] %s\n", dlerror());
    return NULL;
  }
}

void close_libraries()
{
  dlclose(LIBLINKPARSER);
  dlclose(LIBICU_BINDER);
  dlclose(LIBC_SHARED);
}

int fuzz(const uint8_t *data, size_t size)
{
  char *input = (char *)malloc(size + 1);
  memcpy(input, data, size);
  input[size] = 0;

  if (!validate_utf8(input, size + 1))
  {
#ifdef TRIAGE
    fprintf(stderr, "[-] Invalid UTF-8 data\n");
#endif
    return 0;
  }
  else
  {
#ifdef TRIAGE
    printf("[i] Valid UTF-8 string\n");
#endif
  }

  Functions *functions = load_functions();
  if (functions != NULL)
  {
#ifdef TRIAGE
    printf("[+] Functions loaded\n");
#endif
  }
  else
  {
    exit(2);
  }
  ParserResult *parser_result = (ParserResult *)malloc(sizeof(ParserResult));
  String *url = (String *)malloc(sizeof(String));
  functions->copy_jni_string_from_str(url, input);
  functions->parse_link(parser_result, url);
  free(input);
  free(parser_result);
  free(url);
  free(functions);
  return 0;
}

#ifdef TRIAGE
int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("Usage: %s FILE\n", argv[0]);
    exit(1);
  }

  FILE *f = fopen(argv[1], "r");
  if (f == NULL)
  {
    fprintf(stderr, "FILE open error\n");
    exit(1);
  }
  struct stat sb;
  stat(argv[1], &sb);
  size_t size = sb.st_size;
  uint8_t *data = (uint8_t *)malloc(size);
  fread(data, size, 1, f);
  fclose(f);

  return fuzz(data, size);
}
#else  // TRIAGE
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  return fuzz(data, size);
}
#endif // TRIAGE
