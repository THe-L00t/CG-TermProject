// stb_image.h - v2.28 - public domain image loader
// https://github.com/nothings/stb/blob/master/stb_image.h

#ifndef STBI_INCLUDE_STB_IMAGE_H
#define STBI_INCLUDE_STB_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char stbi_uc;

#ifndef STBIDEF
#ifdef STB_IMAGE_STATIC
#define STBIDEF static
#else
#define STBIDEF extern
#endif
#endif

STBIDEF stbi_uc *stbi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
STBIDEF void stbi_image_free(void *retval_from_stbi_load);
STBIDEF const char *stbi_failure_reason(void);

#ifdef __cplusplus
}
#endif

#endif // STBI_INCLUDE_STB_IMAGE_H

#ifdef STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Minimal PNG loader implementation
static stbi_uc *stbi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels)
{
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;

    // Read PNG signature
    unsigned char sig[8];
    fread(sig, 1, 8, f);

    // Simple PNG parser (minimal implementation)
    // This is a placeholder - you should use the full stb_image.h from GitHub
    fclose(f);
    return NULL;
}

static void stbi_image_free(void *retval_from_stbi_load)
{
    free(retval_from_stbi_load);
}

static const char *stbi_failure_reason(void)
{
    return "stb_image not fully implemented - please download full version";
}

#endif // STB_IMAGE_IMPLEMENTATION
