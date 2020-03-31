#include "eosdtorclize.hpp"

#include "settings.cpp"
#include "subscribers.cpp"
#include "queries.cpp"
#include "rates.cpp"
#include "orclize.cpp"
#include "defer.cpp"
#include "liquid.cpp"
#include "delphi.cpp"

namespace eosdt {
}/// namespace eosdt



EOSIO_DISPATCH_EX(eosdt::eosdtorclize, (currentver)(getcurrent)
        (settingset)(setlistdate)
        (transfer)(unsubscribe)(ratechanged)
        (queryadd)(querydel)(callback)
        (refreshutil)(masterefresh)
        (startrefresh)(stoprefresh)
        (delphirefres)
)
