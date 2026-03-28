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
    [TOOSMALL] = "%s is too small.\n"
};

bool validate_dimension(size_t height, size_t width)
{
    // a bit dangerous since height and width are size_t, safety check?
    if (!height || !width || height > UINT32_MAX || width > UINT32_MAX) {
        DBG_PRINT(INVALID_DIM, height, width);
        return false;
    }
    return true;
}