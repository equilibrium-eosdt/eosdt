#include "eosdtorclize.hpp"

namespace eosdt {

    void eosdtorclize::create_tran(const ds_symbol &symbol, const ds_symbol &base, const ds_account &action, const ds_int &interval) {
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
                std::make_tuple(symbol, base)
        );

        auto id = (((uint128_t) symbol.raw()) << 64) | (((uint128_t) action.value) ^ ((uint128_t) base.raw()));
        auto deleted = cancel_deferred(id);
        ds_print("\r\ncancel: %, create: %, action: %, interval: %, delay: %, symbol: %, base: %.",
                 deleted, id, action, interval, (ds_uint) t.delay_sec, symbol, base);
        t.send(id, _self);
    }

    void eosdtorclize::cancel_tran(const ds_symbol &symbol, const ds_symbol &base, const ds_account &action) {
        auto id = (((uint128_t) symbol.raw()) << 64) | (((uint128_t) action.value) ^ ((uint128_t) base.raw()));
        auto deleted = cancel_deferred(id);
        ds_print("\r\ncancel: %, create: %, action: %, symbol: %,base: %", deleted, id, action, symbol, base);
    }

    void eosdtorclize::comonrefresh(const ds_symbol &symbol, const ds_symbol &base, const ds_account &action) {
        PRINT_STARTED("comonrefresh"_n)
        auto time = time_get();
        auto settings = orasetting_get();
        orarates_table orarates(_self, _self.value);
        auto index = orarates.template get_index<"ratebase"_n>();
        auto itr = index.find(compress_key(symbol.code().raw(), base.code().raw()));
        if (itr == index.end()
            || (itr->provablecb1a_update < time - settings.rate_timeout && !is_query_running(symbol, ORACLESOURCE, base))) {
            refreshrates(symbol, ORACLESOURCE, base);
        }
        if (itr->provablecb1a_update != STOP_REFRESH) {
            create_tran(symbol, base, "masterefresh"_n, settings.provablecb1a_interval);
        }
        PRINT_FINISHED("comonrefresh"_n)
    }

    void eosdtorclize::refreshutil(const ds_symbol &symbol, const ds_symbol &base) {
        PRINT_STARTED("refreshutil"_n)
        comonrefresh(symbol, base, "refreshutil"_n);
        PRINT_FINISHED("refreshutil"_n)
    }

    void eosdtorclize::masterefresh(const ds_symbol &symbol, const ds_symbol &base) {
        PRINT_STARTED("masterefresh"_n)
        comonrefresh(symbol, base, "masterefresh"_n);
        PRINT_FINISHED("masterefresh"_n)
    }


    void eosdtorclize::stoprefresh(const ds_symbol &symbol, const ds_symbol &base) {
        PRINT_STARTED("stoprefresh"_n)
        require_auth(_self);
        orarates_table orarates(_self, _self.value);
        auto index = orarates.template get_index<"ratebase"_n>();
        auto rate_itr = index.find(compress_key(symbol.code().raw(), base.code().raw()));
        ds_assert(rate_itr != index.end(), "rates does not exists for symbol: '%'.", symbol);
        index.modify(rate_itr, ds_account(0), [&](auto &o) {
            o.provablecb1a_update = STOP_REFRESH;
            o.delphioracle_update = STOP_REFRESH;
            o.equilibriumdsp_update = STOP_REFRESH;
        });
        cancel_tran(symbol, base, "masterefresh"_n);
        PRINT_FINISHED("stoprefresh"_n)
    }

    void eosdtorclize::startrefresh(const ds_symbol &symbol, const ds_symbol &base) {
        PRINT_STARTED("startrefresh")
        require_auth(_self);
        orarates_table orarates(_self, _self.value);
        auto index = orarates.template get_index<"ratebase"_n>();
        auto rate_itr = index.find(compress_key(symbol.code().raw(), base.code().raw()));
        ds_assert(rate_itr != index.end(), "rates does not exists for symbol: '%'.", symbol);
        auto time = time_get();
        auto settings = orasetting_get();
        time -= settings.rate_timeout;
        index.modify(rate_itr, ds_account(0), [&](auto &o) {
            o.provablecb1a_update = time;
            o.delphioracle_update = time;
            o.equilibriumdsp_update = time;
        });
        comonrefresh(symbol, base, "startrefresh"_n);
        PRINT_FINISHED("startrefresh")
    }
}
