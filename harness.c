#include <stdint.h>
#include <stddef.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "liblinkparser.so.h"

const ptrdiff_t ADDR_JNI_ONLOAD = 0x0000000000011640;
const ptrdiff_t ADDR_PARSE_LINK = 0x000000000002F870;
const ptrdiff_t ADDR_COPY_JNI_STRING_FROM_STR = 0x0000000000011160;

typedef struct Functions
{
  void *(*parse_link)(struct ParserResult *, struct String *);
  void *(*copy_jni_string_from_str)(struct String *, const char *);
} Functions;

int afl_libfuzzer_init()
{
  return 0;
}

struct Functions *load_function()
{
  void *p_lib = dlopen("liblinkparser.so", RTLD_NOW | RTLD_GLOBAL);
  if (p_lib != NULL)
  {
    int (*JNI_OnLoad)(void *, void *) = dlsym(p_lib, "JNI_OnLoad");
    if (JNI_OnLoad != NULL)
    {
      Dl_info jni_on_load_info;
      dladdr(JNI_OnLoad, &jni_on_load_info);
      size_t jni_on_load_addr = (size_t)jni_on_load_info.dli_saddr;
      int diff_parse_link = ADDR_PARSE_LINK - ADDR_JNI_ONLOAD;
      int diff_copy_jni_string_from_str = ADDR_COPY_JNI_STRING_FROM_STR - ADDR_JNI_ONLOAD;
      size_t parse_link_addr = jni_on_load_addr + diff_parse_link;
      size_t copy_jni_string_from_str_addr = diff_copy_jni_string_from_str + jni_on_load_addr;
      printf("[i] parse_link_addr: %zX\n", parse_link_addr);
      printf("[i] copy_jni_string_from_str_addr: %zX\n", copy_jni_string_from_str_addr);
      void *(*parse_link)(ParserResult *, String *) = (void *(*)(ParserResult *, String *))(parse_link_addr);
      void *(*copy_jni_string_from_str)(String *, const char *) = (void *(*)(String *, const char *))(copy_jni_string_from_str_addr);
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
      perror("[-] Can't find JNI_OnLoad function (dlsym)");
      return NULL;
    }
    dlclose(p_lib);
  }
  else
  {
    fprintf(stderr, "[-] %s\n", dlerror());
    return NULL;
  }
}

#define TRIAGE
#ifdef TRIAGE
int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("Usage: %s URL\n", argv[0]);
    exit(1);
  }
  Functions *functions = load_function();
  printf("[+] Functions loaded\n");
  ParserResult *parser_result = (ParserResult *)malloc(sizeof(ParserResult));
  String *url = (String *)malloc(sizeof(String));
  functions->copy_jni_string_from_str(url, argv[1]);
  functions->parse_link(parser_result, url);
  return 0;
}
#else  // TRIAGE
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  return 0;
}
#endif // TRIAGE
