#ifndef STORAGE_H_
#define STORAGE_H_

#include "KVStore.h"
#include "kvstore_global_api.h"
#include "mbed.h"
#include <string>

class KvStore
{
public:
    KvStore(KvStore const&) = delete;       // do not allow copy constructor of a singleton
    void operator=(KvStore const&) = delete;
    static KvStore& getInstance();
    template<typename T> void store(const std::string key, T value)
    {
        int result = kv_set(key.c_str(), &value, sizeof(T), 0);
        if(result)
        {
            printf("Parameter '%s' store error %d\n", key.c_str(), MBED_GET_ERROR_CODE(result));
        }
    }

    template<typename T> T restore(const std::string key, const T defaultValue)
    {
        T value;
        bool error = false;
        int result = kv_get_info(key.c_str(), &info);
        if(result)
        {
            error = true;
            printf("Parameter '%s' get info error %d\n", key.c_str(), MBED_GET_ERROR_CODE(result));
        }
        else
        {
            size_t actualSize;
            result = kv_get(key.c_str(), &value, info.size, &actualSize);
            if(result)
            {
                error = true;
                printf("Parameter '%s' restore error %d\n", key.c_str(), MBED_GET_ERROR_CODE(result));
            }
        }

        if(error)
        {
            value = defaultValue;
            store<T>(key, value);
        }

        return value;
    }
private:
    KvStore();
    kv_info_t info;
};

#endif /* STORAGE_H_ */
