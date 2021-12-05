#ifndef STORAGE_H_
#define STORAGE_H_

#include "Console.h"
#include "KVStore.h"
#include "Logger.h"
#include "kvstore_global_api.h"
#include "mbed.h"
#include <string>

/*
class of flash disk storage for program parameters
*/
class KvStore
{
public:
    KvStore(KvStore const&) = delete;       // do not allow copy constructor of a singleton
    void operator=(KvStore const&) = delete;
    KvStore(KvStore&&) = delete;
    void operator=(KvStore&&) = delete;
    static KvStore& getInstance();
    
    template<typename T> void store(const std::string& key, T value)     // store key-value pair in memory
    {
        int result = kv_set(key.c_str(), &value, sizeof(T), 0);
        if(result)
        {
            LOG_ERROR("Parameter " << key << " store error " << MBED_GET_ERROR_CODE(result));   //NOLINT(hicpp-signed-bitwise)
        }
    }

    /*
    restore value from the given key
    if key not found, create the parameter with default value
    */
    template<typename T> T restore(const std::string& key, T defaultValue)
    {
        T value;
        bool error = false;
        int result = kv_get_info(key.c_str(), &info);
        if(result)
        {
            error = true;
            LOG_ERROR("Parameter " << key << " get info error " << MBED_GET_ERROR_CODE(result));    //NOLINT(hicpp-signed-bitwise)
        }
        else
        {
            size_t actualSize{0};
            result = kv_get(key.c_str(), &value, info.size, &actualSize);
            if(result)
            {
                error = true;
                LOG_ERROR("Parameter " << key << " restore error " << MBED_GET_ERROR_CODE(result));     //NOLINT(hicpp-signed-bitwise)
            }
        }

        if(error)
        {
            value = defaultValue;
            store<T>(key, value);
        }

        return value;
    }

    /*
    restore value from the given key using min-max limits
    if key not found, create the parameter with default value
    */
    template<typename T> T restore(const std::string key, T defaultValue, T minValue, T maxValue)
    {
        T value = restore<T>(key, defaultValue);
        if(value > maxValue)
        {
            value = maxValue;
        }
        else if(value < minValue)
        {
            value = minValue;
        }

        return value;
    }

    static void list(CommandVector& cv);
    static void clear(CommandVector& cv);
private:
    KvStore();
    ~KvStore() = default;
    kv_info_t info{0};
};

#endif /* STORAGE_H_ */
