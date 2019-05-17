#include "settings.hpp"

class oracle : public settings {

private:
    typedef eosio::multi_index<ORACLERATES, oracle_rate> oracle_rates_type;

protected:
    auto oracle_get_rate(const ds_symbol &token_symbol) {
        auto oraclize_account = settings_get().oraclize_account;
        oracle_rates_type oracle_rates(oraclize_account, oraclize_account.value);
        auto itr = oracle_rates.find(token_symbol.raw());
        ds_assert(itr != oracle_rates.end(), "oracle rate % not found.", token_symbol);
        return itr->rate;
    }

public:
    oracle(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) :
            settings(receiver, code, ds) {
    }


};
