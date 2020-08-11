#ifndef STORAGE_H_
#define STORAGE_H_

#include "KVStore.h"
#include "kvstore_global_api.h"
#include "mbed.h"
#include <string>

class KvStore
{
public:
    template<typename T> void store(const std::string key, T value);
    template<typename T> T restore(const std::string key);
private:
    kv_info_t info;
};

#endif /* STORAGE_H_ */