#include "liquid.hpp"

class [[eosio::contract("eosdtorclize")]] eosdtorclize : public liquid {
public:
    eosdtorclize(ds_account receiver, ds_account code, datastream<const char *> ds) : liquid(receiver, code, ds) {
    }

    void currentver() {
        ds_assert(false, GIT_VERSION);
    }
};
extern "C" { void apply( ds_ulong receiver, ds_ulong code, ds_ulong action ) { if( code != receiver && action != "transfer"_n.value) return; switch( action ) {EOSIO_DISPATCH_HELPER( eosdtorclize, (settingset)(setlistdate) (refreshutil) (callback) (masterefresh) (startrefresh)(stoprefresh) (queryadd)(querydel) (currentver) (transfer)(unsubscribe)(ratechanged) )} } }
