#pragma once
#define EOSDTCURRENT "eosdtorclize"_n

#include "info.hpp"
#include "../../eosdt/eosdt.hpp"


namespace eosdt {

    class [[eosio::contract("eosdtorclize")]] eosdtorclize : public eosdt::contract {

    public:
        using eosdt::contract::contract;

        void currentver();

        void getcurrent();

        void settingset(const ds_int &rate_timeout, const ds_int &query_timeout, const ds_int &provablecb1a_interval,
                        const ds_int &delphioracle_interval, const ds_int &equilibriumdsp_interval);

        void setlistdate(const ds_time &utility_listing_date);

        void transfer(const ds_account &from, const ds_account &to, ds_asset &quantity, const ds_string &memo);

        void unsubscribe(const ds_account &contract);

        void ratechanged(const ds_account &contract, const ds_time &update, const ds_asset &rate, const ds_symbol &base);

        void queryadd(const ds_symbol &symbol, const ds_account &source_contract, const ds_string &query,
                const uint8_t price_type, const ds_symbol &base);

        void querydel(const ds_symbol &symbol, const ds_account &source_contract, const ds_symbol &base);

        void callback(const eosio::checksum256 &query_id, const std::vector<unsigned char> &result,
                      const std::vector<unsigned char> &proof);

        void comonrefresh(const ds_symbol &symbol, const ds_symbol &base, const ds_account &action);

        void refreshutil(const ds_symbol &symbol, const ds_symbol &base);

        void masterefresh(const ds_symbol &symbol, const ds_symbol &base);

        void stoprefresh(const ds_symbol &symbol, const ds_symbol &base);

        void startrefresh(const ds_symbol &symbol, const ds_symbol &base);

        void delphirefres();

    private:
        void on_rate_changed(const ds_time &update, const ds_asset &rate, const ds_symbol &base);

        auto parse_price(const ds_symbol &symbol, const char *data);

        auto parse_rate(const ds_symbol &token_symbol, const ds_string &data);

        auto get_usd_rate();

        void set_median(const ds_int &rate_timeout, orarate &orarate);
        void rate_set(const ds_symbol &token_symbol, const source_type &source, const price_type &price_type,
                const ds_string &data);
        void rate_set(const ds_symbol &token_symbol, const source_type &source, const price_type &price_type,
                      const ds_symbol &base, const ds_string &data);
        void rate_set(const source_type &source, const price_type &price_type, const ds_asset &data);
        void rate_set(const source_type &source, const price_type &price_type, const ds_symbol &base,
                const ds_asset &data);

        void updtoldrates(const ds_time &time, const ds_asset &rate, const ds_symbol &base);

        bool is_query_running(const ds_symbol &symbol, const ds_account &source_contract, const ds_symbol &base);

        void refreshrates(const ds_symbol &symbol, const ds_account &source_contract, const ds_symbol &base);

        void create_tran(const ds_symbol &symbol, const ds_symbol &base, const ds_account &action, const ds_int &interval);

        void cancel_tran(const ds_symbol &symbol, const ds_symbol &base, const ds_account &action);

    };
} /// namespace eosdt

