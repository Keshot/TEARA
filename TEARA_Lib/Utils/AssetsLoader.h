#ifndef _TEARA_LIB_UTILS_ASSETS_LOADER_H_
#define _TEARA_LIB_UTILS_ASSETS_LOADER_H_

#include "TEARA_3RDPARTY/stb/stb_image.h"

void AssetsLoaderInit()
{
    stbi_set_flip_vertically_on_load_thread(1);
}

#endif