#include "eosdtreserve.hpp"

namespace eosdt {

    void eosdtreserve::paraminit() {
        PRINT_STARTED("paraminit"_n)
        require_auth(_self);
        resparams_table resparams(_self, _self.value);
        auto itr = resparams.find(0);
        ds_assert(itr == resparams.end(), "% resparams.", ALREADY_STORED);
        const auto set = [&](auto &row) {
            row.param_id = 0;
            row.funds_sent = 0;
        };
        resparams.emplace(_self, set);
        PRINT_FINISHED("paraminit"_n)
    }

    void eosdtreserve::fundssent() {
        PRINT_STARTED("fundssent"_n)
        resparams_table resparams(_self, _self.value);
        auto itr = resparams.find(0);
        ds_assert(itr != resparams.end(), "% resparams.", NEED_TO_SET_UP);
        const auto set = [&](auto &row) {
            row.param_id = 0;
            row.funds_sent = 1;
        };
        resparams.modify(itr, ds_account(0), set);
        PRINT_FINISHED("fundssent"_n)
    }

    uint8_t eosdtreserve::fundssentget() {
        resparams_table resparams(_self, _self.value);
        auto itr = resparams.find(0);
        ds_assert(itr != resparams.end(), "% resparams.", NEED_TO_SET_UP);
        return itr->funds_sent;
    }
}
