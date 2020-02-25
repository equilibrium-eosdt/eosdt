#include "eosdtorclize.hpp"

namespace eosdt {

    void eosdtorclize::create_tran(const ds_symbol &symbol, const ds_account &action, const ds_int &interval) {
        if (interval <= 0) {
            return;
        }
        auto time = time_get().sec_since_epoch();
        eosio::transaction t;
        t.delay_sec = interval - time % interval;
        if (t.delay_sec < 15U) {
            t.delay_sec = (ds_int) t.delay_sec + interval;
        }
        t.expiration = (ds_time)(time + (ds_uint) t.delay_sec + interval);
        t.actions.emplace_back(
                eosio::permission_level(_self, "active"_n),
                EOSDTORCLIZE,
                action,
                std::make_tuple(symbol)
        );

        auto id = (((uint128_t) symbol.raw()) << 64) | (((uint128_t) action.value));
        auto deleted = cancel_deferred(id);
        ds_print("\r\ncancel: %, create: %, action: %, interval: %, delay: %.",
                 deleted, id, action, interval, (ds_uint) t.delay_sec);
        t.send(id, _self);
    }

    void eosdtorclize::cancel_tran(const ds_symbol &symbol, const ds_account &action) {
        auto id = (((uint128_t) symbol.raw()) << 64) | (((uint128_t) action.value));
        auto deleted = cancel_deferred(id);
        ds_print("\r\ncancel: %, create: %, action: %, symbol: %", deleted, id, action, symbol);
    }

    void eosdtorclize::comonrefresh(const ds_symbol &symbol, const ds_account &action) {
        PRINT_STARTED("comonrefresh"_n)
        auto time = time_get();
        auto settings = orasetting_get();
        orarates_table orarates(_self, _self.value);
        auto itr = orarates.find(symbol.raw());
        if (itr == orarates.end()
            || (itr->provablecb1a_update < time - settings.rate_timeout && !is_query_running(symbol))) {
            refreshrates(symbol);
        }
        if (itr->provablecb1a_update != STOP_REFRESH) {
            create_tran(symbol, "masterefresh"_n, settings.provablecb1a_interval);
        }
        PRINT_FINISHED("comonrefresh"_n)
    }

    void eosdtorclize::refreshutil(const ds_symbol &symbol) {
        PRINT_STARTED("refreshutil"_n)
        comonrefresh(symbol, "refreshutil"_n);
        PRINT_FINISHED("refreshutil"_n)
    }

    void eosdtorclize::masterefresh(const ds_symbol &token_symbol) {
        PRINT_STARTED("masterefresh"_n)
        comonrefresh(token_symbol, "masterefresh"_n);
        PRINT_FINISHED("masterefresh"_n)
    }


    void eosdtorclize::stoprefresh(const ds_symbol &symbol) {
        PRINT_STARTED("stoprefresh"_n)
        require_auth(_self);
        orarates_table orarates(_self, _self.value);
        auto itr = orarates.find(symbol.raw());
        ds_assert(itr != orarates.end(), "rates does not exists for symbol: '%'.", symbol);
        orarates.modify(itr, ds_account(0), [&](auto &o) {
            o.provablecb1a_update = STOP_REFRESH;
            o.delphioracle_update = STOP_REFRESH;
            o.equilibriumdsp_update = STOP_REFRESH;
        });
        cancel_tran(symbol, "masterefresh"_n);
        PRINT_FINISHED("stoprefresh"_n)
    }

    void eosdtorclize::startrefresh(const ds_symbol &token_symbol) {
        PRINT_STARTED("startrefresh")
        require_auth(_self);
        orarates_table orarates(_self, _self.value);
        auto itr = orarates.find(token_symbol.raw());
        ds_assert(itr != orarates.end(), "rates does not exists for symbol: '%'.", token_symbol);
        auto time = time_get();
        auto settings = orasetting_get();
        time -= settings.rate_timeout;
        orarates.modify(itr, ds_account(0), [&](auto &o) {
            o.provablecb1a_update = time;
            o.delphioracle_update = time;
            o.equilibriumdsp_update = time;
        });
        comonrefresh(token_symbol, "startrefresh"_n);
        PRINT_FINISHED("startrefresh")
    }
}
