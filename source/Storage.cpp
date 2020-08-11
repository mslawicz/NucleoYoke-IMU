#include "Storage.h"

template<typename T> void KvStore::store(const std::string key, T value)
{
    int result = kv_set(key.c_str(), &value, sizeof(T), 0);
    if(result)
    {
        printf("Parameter '%s' store error %d\n", key.c_str(), MBED_GET_ERROR_CODE(result));
    }
}

template<typename T> T KvStore::restore(const std::string key)
{
    T value;
    int result = kv_get_info(key.c_str(), &info);
    if(result)
    {
        printf("Parameter '%s' get info error %d\n", key.c_str(), MBED_GET_ERROR_CODE(result));
    }
    else
    {
        result = kv_get(key.c_str(), &value, info.size);
        if(result)
        {
            printf("Parameter '%s' restore error %d\n", key.c_str(), MBED_GET_ERROR_CODE(result));
        }
    }
    return value;
}