#include "Storage.h"

KvStore::KvStore()
{
    //Console::getInstance().registerCommand("da", "display alarms", callback(this, &Alarm::display));
    //Console::getInstance().registerCommand("ca", "clear alarms", callback(this, &Alarm::clear));
}

KvStore& KvStore::getInstance()
{
    static KvStore instance;  // Guaranteed to be destroyed, instantiated on first use
    return instance;
}