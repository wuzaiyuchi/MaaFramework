#pragma once

#include "MaaDef.h"
#include "MaaPort.h"

#ifdef __cplusplus
extern "C"
{
#endif

    struct MAA_API MaaRect
    {
        int32_t x;
        int32_t y;
        int32_t width;
        int32_t height;
    };

    struct MAA_API MaaImage
    {
        int32_t rows;
        int32_t cols;
        int32_t type;
        void* data;
    };

#define MaaRecognitionResultDetailBuffSize 16384

    struct MAA_API MaaRecognitionResult
    {
        MaaRect box;
        void* detail_buff; // size = MaaRecognitionResultDetailBuffSize
    };

    struct MAA_API MaaCustomRecognizerAPI
    {
        MaaBool (*analyze)(const MaaImage* image, MaaJsonString custom_recognition_param,
                           /*out*/ MaaRecognitionResult* result);
    };

#ifdef __cplusplus
}
#endif