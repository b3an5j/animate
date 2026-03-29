#include "misc_helper.h"

const char* const dbg_strings[] = {
    [ERR_MALLOC] = "Failed to allocate memory for %s.\n",
    [ERR_MALLOC_WSIZE] = "Failed to allocate memory for %s %ux%u.\n",
    [ERR_IO] = "Failed to open %s.\n",
    [ERR_BM_HEADER] = "Failed to read bitmap header of %s.\n",
    [ERR_BM_V5] = "Failed to read v5 header of %s.\n",
    [FREED] = "%s is freed.\n",
    [INVALID_ARG] = "Invalid argument %s.\n",
    [INVALID_DIM] = "Invalid dimension, H = %zu W = %zu.\n",
    [TOOBIG] = "%s is too big.\n",
    [FAIL] = "%s is not successful.\n",
    [SUCCESS] = "%s is successful.\n"
};

#ifdef DEBUG
void dbg_print_helper(ErrorType code, ...)
{
    va_list args;
    va_start(args, code);

    fprintf(stderr, "[DEBUG] ");
    if ((code) == CUSTOM) {
        const char* str = va_arg(args, const char*);
        vfprintf(stderr, str, args);
        fprintf(stderr, "\n");
    }
    else {
        vfprintf(stderr, dbg_strings[code], args);
    }
}
#endif

bool validate_dimension(size_t height, size_t width)
{
    // a bit dangerous since height and width are size_t, safety check?
    if (!height || !width || height > UINT32_MAX || width > UINT32_MAX) {
        DBG_PRINT(INVALID_DIM, height, width);
        return false;
    }
    return true;
}