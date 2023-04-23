#include <stdint.h>

struct BigString {
    uint64_t flag;
    size_t len;
    char * str;
};

struct SmallString {
    uint8_t len;
    char str[23];
};

union InnerString {
    struct SmallString small;
    struct BigString big;
};

typedef struct String {
    union InnerString inner;
} String;

typedef struct ParserResult {
    struct String user_agent_string;
    struct String user_agent_info_string;
    struct String accept_string;
    struct String mime_type_string;
} ParserResult;
