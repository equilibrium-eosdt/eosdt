#include "applicator.hpp"
class [[eosio::contract("eosdtgovernc")]] eosdtgovernc : public appliator {
public:
    eosdtgovernc(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) :
            appliator(receiver, code, ds) {
    }
};
extern "C" { void apply( ds_ulong receiver, ds_ulong code, ds_ulong action ) { if( code != receiver && action != "transfer"_n.value) return; switch( action ) {EOSIO_DISPATCH_HELPER( eosdtgovernc, (settingset) (propose) (transfer) (vote)(unvote) (unstake) (expire) (apply) (clnproposal) )} } }
