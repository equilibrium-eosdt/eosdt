#ifndef EOSIO_TYPES_HPP
#define EOSIO_TYPES_HPP

#ifdef COMMON

#include <eosio/testing/tester.hpp>
#define ds_time eosio::chain::time_point_sec
#define ds_account eosio::chain::name
#define ds_asset eosio::chain::asset
#define ds_symbol eosio::chain::symbol
#define ds_checksum eosio::chain::checksum256_type
#define ds_stamp eosio::chain::block_timestamp_type

#else
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>
#include <eosio/eosio.hpp>
#include <eosio/print.hpp>
#include <eosio/time.hpp>
#include <eosio/symbol.hpp>
#include <eosio/asset.hpp>
#include <stdio.h>
#include <vector>
#include <cmath>


#define ds_time eosio::time_point_sec
#define ds_account eosio::name
#define ds_symbol eosio::symbol
#define ds_asset eosio::asset
#define ds_checksum eosio::checksum256
#define ds_stamp eosio::block_timestamp
#define N(X) name{#X}
#endif

#ifdef DEBUG
#define PRINT_STARTED(ACTION) ds_print("\r\n[%] % started.", __LINE__,ACTION);
#define PRINT_FINISHED(ACTION) ds_print("\r\n[%] % finished.", __LINE__,ACTION);
#else
#define PRINT_STARTED(ACTION)
#define PRINT_FINISHED(ACTION)
#endif

#define STOP_REFRESH ds_time(0)
#define BEFORE_LISTING_RATE 30.0L

#define ds_int int
#define ds_uint unsigned int
#define ds_ulong unsigned long long
#define ds_long long long
#define ds_llong __int128_t
#define ds_string std::string
#define ds_nullptr std::nullptr_t

#define EOS_SYMBOL_STR "EOS"
#define EOS_SYMBOL_DECIMAL 4
#define USD_SYMBOL_STR "USD"
#define USD_SYMBOL_DECIMAL 4
#define STABLE_SYMBOL_STR "EOSDT"
#define STABLE_SYMBOL_PAD_STR " " STABLE_SYMBOL_STR
#define STABLE_SYMBOL_DECIMAL 9
#define UTILITY_SYMBOL_STR "NUT"
#define UTILITY_SYMBOL_PAD_STR " " UTILITY_SYMBOL_STR
#define UTILITY_SYMBOL_DECIMAL 9
#define DAPP_SYMBOL_STR "DAPP"
#define DAPP_SYMBOL_PAD_STR " " DAPP_SYMBOL_STR
#define DAPP_SYMBOL_DECIMAL 4

#ifdef COMMON
#define EOSDTCNTRACT N(eosdtcntract)
#define EOSDTORCLIZE N(eosdtorclize)
#define ORACLERATES N(orarates)
#define EOS_SYMBOL ds_symbol(EOS_SYMBOL_DECIMAL,EOS_SYMBOL_STR)
#define EOS_SYMBOL_VALUE ::eosio::chain::string_to_symbol_c(EOS_SYMBOL_DECIMAL,EOS_SYMBOL_STR)
#define USD_SYMBOL ds_symbol(USD_SYMBOL_DECIMAL,USD_SYMBOL_STR)
#define USD_SYMBOL_VALUE ::eosio::chain::string_to_symbol_c(USD_SYMBOL_DECIMAL,USD_SYMBOL_STR)
#define STABLE_SYMBOL ds_symbol(STABLE_SYMBOL_DECIMAL,STABLE_SYMBOL_STR)
#define STABLE_SYMBOL_VALUE ::eosio::chain::string_to_symbol_c(STABLE_SYMBOL_DECIMAL,STABLE_SYMBOL_STR)
#define UTILITY_SYMBOL ds_symbol(UTILITY_SYMBOL_DECIMAL,UTILITY_SYMBOL_STR)
#define UTILITY_SYMBOL_VALUE ::eosio::chain::string_to_symbol_c(UTILITY_SYMBOL_DECIMAL,UTILITY_SYMBOL_STR)
#else
#define EOSDTCNTRACT "eosdtcntract"_n
#define EOSDTORCLIZE "eosdtorclize"_n
#define ORACLERATES "orarates"_n
#define ORACLEREFRESH "refreshutil"_n
#define EOSDTFORUM "eosdtgovernc"_n
#define EOS_SYMBOL ds_symbol(EOS_SYMBOL_STR,EOS_SYMBOL_DECIMAL)
#define EOS_SYMBOL_VALUE EOS_SYMBOL.value()
#define USD_SYMBOL ds_symbol(USD_SYMBOL_STR,USD_SYMBOL_DECIMAL)
#define USD_SYMBOL_VALUE USD_SYMBOL.value()
#define STABLE_SYMBOL ds_symbol(STABLE_SYMBOL_STR,STABLE_SYMBOL_DECIMAL)
#define STABLE_SYMBOL_VALUE STABLE_SYMBOL.value()
#define UTILITY_SYMBOL ds_symbol(UTILITY_SYMBOL_STR,UTILITY_SYMBOL_DECIMAL)
#define UTILITY_SYMBOL_VALUE UTILITY_SYMBOL.value()
#define DAPP_SYMBOL ds_symbol(DAPP_SYMBOL_STR,DAPP_SYMBOL_DECIMAL)
#define DAPP_SYMBOL_VALUE DAPP_SYMBOL.value()
#endif
#define EMPTY_SYMBOL (ds_symbol(0ull))


#define DOES_NOT_EXIST "does not exist"

#define ORASETTINGS "oracle settings"
struct oracle_settings
{
    ds_ulong id;
    ds_time utility_listing_date;
    ds_int rate_timeout;
    ds_int query_timeout;
    ds_int provablecb1a_interval;
    ds_int eosnationdsp_interval;
    ds_int equilibriumdsp_interval;

    ds_ulong primary_key() const { return id; }
};

#ifdef COMMON
FC_REFLECT(oracle_settings, (id)(utility_listing_date)(rate_timeout)(query_timeout)(provablecb1a_interval)(eosnationdsp_interval)(equilibriumdsp_interval));
#endif

struct oraqueries
{
    ds_symbol asset_symbol;
    ds_string query;
    ds_int price_type;
    ds_time query_updated_at;
    ds_time query_executed_at;
    ds_checksum checksumm;

    uint64_t primary_key() const {
#ifdef COMMON
        return asset_symbol.value();
#else
        return asset_symbol.raw();
#endif
    }
};

#ifdef COMMON
FC_REFLECT(oraqueries, (asset_symbol)(query)(price_type)(query_updated_at)(query_executed_at)(checksumm));
#endif

enum class price_type : ds_int {
    SYMBOL_TO_EOS,
    EOS_TO_SYMBOL,
    SYMBOL_TO_USD
};

enum class source_type : ds_int {
    provablecb1a,
    eosnationdsp,
    equilibriumdsp
};

auto to_string(const source_type &source)
{
    switch (source) {
        case source_type::provablecb1a:
            return "provablecb1a";
        case source_type::eosnationdsp:
            return "eosnationdsp";
        case source_type::equilibriumdsp:
            return "equilibriumdsp";
        default:
            return std::to_string((ds_int)source).c_str();
    }
}

struct oracle_rate
{
    ds_asset rate;
    ds_time update;
    ds_asset provablecb1a_price;
    ds_time provablecb1a_update;
    ds_asset eosnationdsp_price;
    ds_time eosnationdsp_update;
    ds_asset equilibriumdsp_price;
    ds_time equilibriumdsp_update;

    uint64_t primary_key() const {

#ifdef COMMON
        return rate.get_symbol().value();
#else
        return rate.symbol.raw();
#endif
    }
};

#ifdef COMMON
FC_REFLECT(oracle_rate, (rate)(update)(provablecb1a_price)(provablecb1a_update)(eosnationdsp_price)(eosnationdsp_update)(equilibriumdsp_price)(equilibriumdsp_update));
#endif

#ifndef ORACLIZEAPI_H
struct cbaddr
{
    ds_account sender;

    uint64_t primary_key() const { return sender.value; }
};

#endif
#ifdef COMMON
FC_REFLECT(cbaddr, (sender) );
#endif

struct ctrsetting {
    ds_ulong setting_id;
    uint8_t global_lock;
    ds_long time_shift;
    ds_account liquidator_account;
    ds_account oraclize_account;
    ds_account sttoken_account;
    ds_account nutoken_account;
    double governance_fee;
    double stability_fee;
    double critical_ltv;
    double liquidation_penalty;
    double liquidator_discount;
    ds_asset liquidation_price;
    double nut_auct_ratio;
    double nut_discount;
    double profit_factor;
    ds_uint vote_period;
    ds_uint stake_period;
    double reserve_ratio;
    double staking_weight;
    ds_account bpproxy_account;
    ds_account governc_account;

    ds_ulong primary_key() const { return setting_id; }
};
#ifdef COMMON
FC_REFLECT(ctrsetting, (setting_id)
        (global_lock)(time_shift)
        (liquidator_account)(oraclize_account)(sttoken_account)(nutoken_account)
        (governance_fee)(stability_fee)(critical_ltv)
        (liquidation_penalty)(liquidator_discount)(liquidation_price)(nut_auct_ratio)(nut_discount)(profit_factor)
        (vote_period)(stake_period)(reserve_ratio)(staking_weight)(bpproxy_account))
#endif

struct subscribe
{
    ds_account contract;
    ds_asset quantity;
    ds_uint callcount;
    ds_time lastcall;
    ds_time withdrawal_date;

    ds_ulong primary_key() const { return contract.value; }
};
#ifdef COMMON
FC_REFLECT(subscribe, (contract)(quantity)(callcount)(lastcall)(withdrawal_date))
#endif



struct record
{
    ds_ulong record_id;
    ds_time time;

    ds_account receiver;
    ds_account code;
    ds_account action;
    ds_string data;


    uint64_t primary_key() const { return record_id; }
};
#ifdef COMMON
FC_REFLECT(record, (record_id)(time)(receiver)(code)(action)(data) );
#endif

struct global {
    uint64_t max_block_net_usage;
    uint32_t target_block_net_usage_pct;
    uint32_t max_transaction_net_usage;
    uint32_t base_per_transaction_net_usage;
    uint32_t net_usage_leeway;
    uint32_t context_free_discount_net_usage_num;
    uint32_t context_free_discount_net_usage_den;
    uint32_t max_block_cpu_usage;
    uint32_t target_block_cpu_usage_pct;
    uint32_t max_transaction_cpu_usage;
    uint32_t min_transaction_cpu_usage;
    uint32_t max_transaction_lifetime;
    uint32_t deferred_trx_expiration_window;
    uint32_t max_transaction_delay;
    uint32_t max_inline_action_size;
    uint16_t max_inline_action_depth;
    uint16_t max_authority_depth;
    uint64_t max_ram_size;
    uint64_t total_ram_bytes_reserved;
    int64_t total_ram_stake;
    ds_stamp last_producer_schedule_update;
    uint64_t last_pervote_bucket_fill;
    int64_t pervote_bucket;
    int64_t perblock_bucket;
    uint32_t total_unpaid_blocks;
    int64_t total_activated_stake;
    uint64_t thresh_activated_stake_time;
    uint16_t last_producer_schedule_size;
    double total_producer_vote_weight;
    ds_stamp last_name_close;

};

#ifdef COMMON
FC_REFLECT(global, (max_ram_size)(total_ram_bytes_reserved)(total_ram_stake)
            (last_producer_schedule_update)(last_pervote_bucket_fill)
            (pervote_bucket)(perblock_bucket)(total_unpaid_blocks)(total_activated_stake)(thresh_activated_stake_time)
            (last_producer_schedule_size)(total_producer_vote_weight)(last_name_close) );
#endif

inline uint32_t now() {
    return current_time_point().sec_since_epoch();
}

#endif
