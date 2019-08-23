#include "positions.hpp"
#include "info.hpp"

class [[eosio::contract("eosdtreserve")]] eosdtreserve : public positions {

public:
    eosdtreserve(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) :
        positions(receiver, code, ds) {
    }

void currentver()
{
        ds_assert(false,GIT_VERSION);
}
};
extern "C" { void apply( ds_ulong receiver, ds_ulong code, ds_ulong action ) { if( code != receiver && action != "transfer"_n.value)return; switch( action ) {EOSIO_DISPATCH_HELPER( eosdtreserve, (currentver) (settingset) (transfer) (withdraw) (sendeos) )} } }
