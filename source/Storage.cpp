#include "Storage.h"

KvStore::KvStore()
{
    int result = kv_reset("/kv/");
    if(result)
    {
        printf("Storage resetting error %d\n", MBED_GET_ERROR_CODE(result));
    }
}