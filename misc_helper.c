#include "misc_helper.h"

// typedef enum {
//     ERR_MALLOC,
//     ERR_MALLOC_WSIZE,
//     ERR_FOPEN,
//     ERR_BM_HEADER,
//     ERR_BM_V5,
//     ERR_CUSTOM,
// } ErrorType;

bool validate_dimension(size_t height, size_t width)
{
    // a bit dangerous since height and width are size_t, safety check?
    if (!height || !width || height > UINT32_MAX || width > UINT32_MAX) {
        DBG_PRINT("Invalid dimension, H = %zu W = %zu\n", height, width);
        return false;
    }
    return true;
}