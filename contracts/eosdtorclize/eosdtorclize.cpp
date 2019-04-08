#include "settings.hpp"
class [[eosio::contract("eosdtorclize")]] eosdtorclize : public settings {
private:
    typedef eosio::multi_index<"queries"_n, oracle_query> oracle_queries_type;
    typedef const oracle_queries_type::const_iterator &oracle_queries_iterator;
    typedef multi_index<"oracle.rates"_n, oracle_rate> oracle_rates_type;
    typedef const oracle_rates_type::const_iterator &oracle_rates_iterator;
protected:
    auto time_get() {
        auto time = now();
        return time_point_sec(time);
    }
    void rates_set(const ds_string &data) {
        oracle_rates_type oracle_rates(_self, _self.value);
        for (auto rate_itr = oracle_rates.begin(); rate_itr != oracle_rates.end(); rate_itr++) {
            auto rate = parse_rate(rate_itr->rate.symbol, data);
            ds_print("\r\nread rates: %", rate);
            if (rate.amount != 0) {
                auto set = [&](auto &row) {
                    row.last_update = time_get();
                    row.rate = rate;
                };
                oracle_rates.modify(rate_itr, _self, set);
                ds_print("\r\nchange rates: % => %", rate_itr->rate, rate);
            }
        }
    }
    void rate_set(const ds_symbol &token_symbol, const ds_string &data) {
        PRINT_STARTED("rateset"_n)
        oracle_rates_type oracle_rates(_self, _self.value);
        auto rate = parse_price(token_symbol, data.c_str());
        ds_print("\r\nread rate: %", rate);
        if (rate.amount <= 0) {
            ds_print("\r\nrateset hasn't succeded");
            return;
        }
        auto set = [&](auto &row) {
            row.last_update = time_get();
            row.rate = rate;
        };
        auto rate_itr = oracle_rates.find(token_symbol.raw());
        if (token_symbol != UTILITY_SYMBOL || time_get() >= setting_get().utility_listing_date) {
            if (rate_itr == oracle_rates.end()) {
                ds_print("\r\nemplace rate: %", rate);
                oracle_rates.emplace(_self, set);
            } else {
                ds_print("\r\nchange rate: % => %", rate_itr->rate, rate);
                oracle_rates.modify(rate_itr, _self, set);
            }
        }
        if (token_symbol == USD_SYMBOL && time_get() < setting_get().utility_listing_date) {
            rate = ds_asset(pow(10.0, UTILITY_SYMBOL_DECIMAL) * to_ldouble(rate) / BEFORE_LISTING_RATE, UTILITY_SYMBOL);
            rate_itr = oracle_rates.find(UTILITY_SYMBOL.raw());
            if (rate_itr == oracle_rates.end()) {
                ds_print("\r\nemplace rate: %", rate);
                oracle_rates.emplace(_self, set);
            } else {
                ds_print("\r\nchange rate: % => %", rate_itr->rate, rate);
                oracle_rates.modify(rate_itr, _self, set);
            }
        }
        PRINT_FINISHED("rateset"_n)
    }
    bool has_recent_delete_old(const ds_symbol &symbol) {
        oracle_queries_type oraclqueries(_self, _self.value);
        auto result = false;
        auto itr = oraclqueries.find(symbol.raw());
        if (itr != oraclqueries.end()) {
            ds_print("\r\nasset_symbol: %, payer: %, moment: %", itr->asset_symbol, itr->payer, itr->moment);
            auto settings = setting_get();
            if ((time_get() - itr->moment).to_seconds() >= settings.query_timeout) {
                oraclqueries.erase(itr);
            } else {
                result = true;
            }
        }
        ds_print("\r\nhas_recent_delete_old finished %", result);
        return result;
    }
    static auto equal(ds_checksum l, ds_checksum r) {
        for (auto i = 0; i < 32; i++) {
            if (l.hash[i] != r.hash[i]) {
                return false;
            }
        }
        return true;
    }
    void create_tran(const ds_symbol &symbol, const ds_account &action, const ds_int &interval) {
        eosio::transaction t;
        t.delay_sec = interval - now() % interval;
        t.delay_sec = (ds_uint)t.delay_sec/2*2 + (action == "slaverefresh"_n ? 1U:0U);
        t.expiration = (ds_time)(now() + (ds_uint) t.delay_sec + 60);
        t.actions.emplace_back(
                eosio::permission_level(_self, "active"_n),
                EOSDTORCLIZE,
                action,
                std::make_tuple(symbol));
        auto id = (((uint128_t) symbol.raw()) << 64) | (((uint128_t) action.value));
        auto deleted = cancel_deferred(id);
        ds_print("\r\ncancel: %, create: %, action: %, interval: %", deleted, id, action, interval);
        t.send(id, _self);
    }
    void cancel_tran(const ds_symbol &symbol, const ds_account &action) {
        auto id = (((uint128_t) symbol.raw()) << 64) | (((uint128_t) action.value));
        auto deleted = cancel_deferred(id);
        ds_print("\r\ncancel: %, create: %, action: %, symbol: %", deleted, id, action, symbol);
    }
    void comonrefresh(const ds_symbol &symbol, const ds_account &action) {
        PRINT_STARTED("comonrefresh"_n)
        oracle_rates_type oracle_rates(_self, _self.value);
        auto itr = oracle_rates.find(symbol.raw());
        if (itr == oracle_rates.end()) {
            return;
        }
        auto time = ds_time(now());
        auto settings = setting_get();
        if (itr->last_update < time - settings.rate_timeout && !has_recent_delete_old(symbol)) {
            refreshutil(_self, symbol);
        }
        if(itr->master_update != STOP_REFRESH)
        {
            create_tran(symbol, "masterefresh"_n, settings.master_interval);
        }
        if(itr->slave_update != STOP_REFRESH)
        {
            create_tran(symbol, "slaverefresh"_n, settings.slave_interval);
        }
        oracle_rates.modify(itr, ds_account(0), [&](auto &o) {
            if (action == "masterefresh"_n && o.master_update != STOP_REFRESH) {
                o.master_update = time;
            } else if (action == "slaverefresh"_n && o.slave_update != STOP_REFRESH) {
                o.slave_update = time;
            }
            else if (action == "onerror"_n ) {
                o.onerror_update = time;
            }
        });
        PRINT_FINISHED("comonrefresh"_n)
    }
public:
    eosdtorclize(ds_account receiver, ds_account code, datastream<const char *> ds) : settings(receiver, code, ds) {
    }
    ACTION refreshutil(const ds_account &payer, const ds_symbol &symbol) {
        PRINT_STARTED("refreshutil"_n)
        require_auth(payer);
        ds_string query;
        if (symbol == USD_SYMBOL) {
            query = "json(https://min-api.cryptocompare.com/data/price?fsym=EOS&tsyms=USD).USD";
        } else if (symbol == UTILITY_SYMBOL) {
            query = "json(https://api.hitbtc.com/api/2/public/ticker/EOSNUT).last";
        } else {
            return;
        }
        if (!has_recent_delete_old(symbol)) {
            auto query_checksumm = oraclize_query("URL", query, proofType_TLSNotary | proofStorage_IPFS);
            oracle_queries_type oraclqueries(_self, _self.value);
            ds_print("\r\nquery_checksum: %", query_checksumm);
            oraclqueries.emplace(payer, [&](auto &o) {
                o.asset_symbol = symbol;
                o.checksumm = query_checksumm;
                o.payer = payer;
                o.moment = time_get();
            });
            comonrefresh(symbol, "refreshutil"_n);
        } else {
            ds_print("please retry later");
        }
        PRINT_FINISHED("refreshutil finished")
    }
    ACTION
    callback(const capi_checksum256 &query_checksumm, const std::vector<unsigned char> &result,
             const std::vector<unsigned char> &proof) {
        PRINT_STARTED("callback"_n)
        {
            require_auth(oraclize_cbAddress());
        }
        oracle_queries_type oraclqueries(_self, _self.value);
        ds_print("callback: %", query_checksumm);
        auto itr = oraclqueries.begin();
        for (; itr != oraclqueries.end() && !equal(itr->checksumm, query_checksumm); itr++) {
            ds_print("\r\noraclqueries: {checksum: %,asset_symbol: %,payer: %,moment: %}.",
                     itr->checksumm, itr->asset_symbol, itr->payer, itr->moment);
        }
        ds_assert(itr != oraclqueries.end(), "checksumm % is not found", query_checksumm);
        oraclqueries.erase(itr);
        auto result_str = vector_to_string(result);
        if (itr->asset_symbol == EMPTY_SYMBOL) {
            rates_set(result_str);
        } else {
            rate_set(itr->asset_symbol, result_str);
        }
        PRINT_FINISHED("callback"_n)
    }
    ACTION queriesdel(const ds_symbol &token_symbol) {
        has_recent_delete_old(token_symbol);
    }
    ACTION masterefresh(const ds_symbol &symbol) {
        PRINT_STARTED("masterefresh"_n)
        comonrefresh(symbol, "masterefresh"_n);
        PRINT_FINISHED("masterefresh"_n)
    }
    ACTION slaverefresh(const ds_symbol &symbol) {
        PRINT_STARTED("slaverefresh"_n)
        comonrefresh(symbol, "slaverefresh"_n);
        PRINT_FINISHED("slaverefresh"_n)
    }
    ACTION onerror(const uint128_t &sender_id, const std::vector<char> &sent_trx) {
        PRINT_STARTED("onerror"_n)
        ds_print("\r\n sender_id:%", sender_id);
        auto tran = unpack<transaction>(sent_trx);
        ds_print(
                "\r\n{expiration: %, ref_block_num: %, ref_block_prefix: %, max_net_usage_words: %, max_cpu_usage_ms: %, delay_sec: %}",
                tran.expiration, (ds_long) tran.ref_block_num, (ds_long) tran.ref_block_prefix,
                (ds_long) tran.max_net_usage_words, (ds_long) tran.max_cpu_usage_ms, (ds_long) tran.delay_sec);
        for (auto act : tran.actions) {
            ds_print("\r\naccount: %, name: %", act.account, act.name);
            if (act.account == EOSDTORCLIZE && act.name == ORACLEREFRESH) {
                auto param = unpack < tuple < ds_account, ds_symbol>>(act.data);
                comonrefresh(std::get<1>(param),"onerror"_n);
                ds_print("param1: %", std::get<1>(param));
            }
            for (auto auth:act.authorization) {
                ds_print("\r\nauth:{actor: %, permission: %}.", auth.actor, auth.permission);
            }
        }
        PRINT_FINISHED("onerror"_n)
    }
    ACTION stoprefresh(const ds_symbol &symbol) {
        PRINT_STARTED("stoprefresh"_n)
        require_auth(_self);
        oracle_rates_type oracle_rates(_self, _self.value);
        auto itr = oracle_rates.find(symbol.raw());
        ds_assert(itr != oracle_rates.end(), "rates does not exists for symbol: '%'.", symbol);
        oracle_rates.modify(itr, ds_account(0), [&](auto &o) {
            o.master_update = o.slave_update = STOP_REFRESH;
        });
        cancel_tran(symbol, "masterefresh"_n);
        cancel_tran(symbol, "slaverefresh"_n);
        PRINT_FINISHED("stoprefresh"_n)
    }
    ACTION startrefresh(const ds_symbol &symbol) {
        PRINT_STARTED("startrefresh")
        require_auth(_self);
        oracle_rates_type oracle_rates(_self, _self.value);
        auto itr = oracle_rates.find(symbol.raw());
        ds_assert(itr != oracle_rates.end(), "rates does not exists for symbol: '%'.", symbol);
        oracle_rates.modify(itr, ds_account(0), [&](auto &o) {
            o.master_update = o.slave_update = ds_time(now());
        });
        comonrefresh(symbol,"startrefresh"_n);
        PRINT_FINISHED("startrefresh")
    }
};
extern "C" { void apply( ds_ulong receiver, ds_ulong code, ds_ulong action ) { if( code != receiver && action != "onerror"_n.value) return; switch( action ) {EOSIO_DISPATCH_HELPER( eosdtorclize, (onerror)(settingset)(setlistdate) (refreshutil) (callback)(queriesdel) (masterefresh)(slaverefresh) (startrefresh)(stoprefresh) )} } }
