#ifndef TYPES_HPP
#define TYPES_HPP

#ifdef COMMON

#include <eosio/testing/tester.hpp>
#include <eosio/chain/resource_limits.hpp>
#define ds_time eosio::chain::time_point_sec
#define ds_account eosio::chain::name
#define ds_asset eosio::chain::asset
#define ds_symbol eosio::chain::symbol
#define ds_checksum eosio::chain::checksum256_type

#else

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/transaction.hpp>

#define ds_time eosio::time_point_sec
#define ds_account eosio::name
#define ds_symbol eosio::symbol
#define ds_asset eosio::asset
#define ds_checksum capi_checksum256
#define N(X) name{#X}
#endif

#define DELETEDATA
#define TESTNET
#define DEBUG

#ifdef DEBUG
#define PRINT_STARTED(ACTION) ds_print("\r\n[%] % started.", __LINE__,ACTION);
#define PRINT_FINISHED(ACTION) ds_print("\r\n[%] % finished.", __LINE__,ACTION);
#else
#define PRINT_STARTED(ACTION)
#define PRINT_FINISHED(ACTION)
#endif

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

#define EOSCTRACT_STR "eosio.token"
#define EOSDTCNTRACT_STR "eosdtcntract"
#define EOSDTORCLIZE_STR "eosdtorclize"
#define ORACLERATES_STR "oracle.rates"
#define EOSDTLIQDATR_STR "eosdtliqdatr"
#define EOSDTNUTOKEN_STR "eosdtnutoken"
#define EOSDTSTTOKEN_STR "eosdtsttoken"

#ifdef COMMON
#define EOSCTRACT N(eosio.token)
#define EOSDTCNTRACT N(eosdtcntract)
#define EOSDTORCLIZE N(eosdtorclize)
#define ORACLERATES N(oracle.rates)
#define EOSDTLIQDATR N(eosdtliqdatr)
#define EOSDTNUTOKEN N(eosdtnutoken)
#define EOSDTSTTOKEN N(eosdtsttoken)
#define EOSDTFORUM N(eosdtgovernc)
#define EOS_SYMBOL ds_symbol(EOS_SYMBOL_DECIMAL,EOS_SYMBOL_STR)
#define EOS_SYMBOL_VALUE ::eosio::chain::string_to_symbol_c(EOS_SYMBOL_DECIMAL,EOS_SYMBOL_STR)
#define USD_SYMBOL ds_symbol(USD_SYMBOL_DECIMAL,USD_SYMBOL_STR)
#define USD_SYMBOL_VALUE ::eosio::chain::string_to_symbol_c(USD_SYMBOL_DECIMAL,USD_SYMBOL_STR)
#define STABLE_SYMBOL ds_symbol(STABLE_SYMBOL_DECIMAL,STABLE_SYMBOL_STR)
#define STABLE_SYMBOL_VALUE ::eosio::chain::string_to_symbol_c(STABLE_SYMBOL_DECIMAL,STABLE_SYMBOL_STR)
#define UTILITY_SYMBOL ds_symbol(UTILITY_SYMBOL_DECIMAL,UTILITY_SYMBOL_STR)
#define UTILITY_SYMBOL_VALUE ::eosio::chain::string_to_symbol_c(UTILITY_SYMBOL_DECIMAL,UTILITY_SYMBOL_STR)
#else
#define EOSCTRACT "eosio.token"_n
#define EOSDTCNTRACT "eosdtcntract"_n
#define EOSDTORCLIZE "eosdtorclize"_n
#define ORACLERATES "oracle.rates"_n
#define ORACLEREFRESH "refreshutil"_n
#define EOSDTLIQDATR "eosdtliqdatr"_n
#define EOSDTNUTOKEN "eosdtnutoken"_n
#define EOSDTSTTOKEN "eosdtsttoken"_n
#define EOSDTFORUM "eosdtgovernc"_n
#define EOS_SYMBOL ds_symbol(EOS_SYMBOL_STR,EOS_SYMBOL_DECIMAL)
#define EOS_SYMBOL_VALUE EOS_SYMBOL.value()
#define USD_SYMBOL ds_symbol(USD_SYMBOL_STR,USD_SYMBOL_DECIMAL)
#define USD_SYMBOL_VALUE USD_SYMBOL.value()
#define STABLE_SYMBOL ds_symbol(STABLE_SYMBOL_STR,STABLE_SYMBOL_DECIMAL)
#define STABLE_SYMBOL_VALUE STABLE_SYMBOL.value()
#define UTILITY_SYMBOL ds_symbol(UTILITY_SYMBOL_STR,UTILITY_SYMBOL_DECIMAL)
#define UTILITY_SYMBOL_VALUE UTILITY_SYMBOL.value()
#endif
#define EMPTY_SYMBOL (ds_symbol(0ull))

#define NEW_LINE "\r\n"
#define NAME "name"
#define TIME "time"
#define EXIST "exist"
#define DOES_NOT_EXIST "does not exist"
#define IS_INVALID "is invalid"
#define ALREADY_STORED "already stored"
#define DOES_NOT_EMPLACE_IN_WHITELIST "does not emplace in whitelist"
#define HAS_WRONG_VALUE "has wrong value"
#define NAME_IS_EMPTY "name is empty"

#define ONLY_FOR_TEST "This function only for test purposes."
#define NEED_TO_SET_UP "need to set up(eosdtgovernc)"
#define SETTLEMENT_TIMING "settlement timing"

#define SETTINGS "settings"

struct account {
    ds_asset balance;

    ds_ulong primary_key() const {
#ifdef COMMON
        return balance.get_symbol().value();
#else
        return balance.symbol.raw();
#endif
    }
};

#ifdef COMMON
FC_REFLECT(account, (balance));
#endif

struct currency_stats {
    ds_asset supply;
    ds_asset max_supply;
    ds_account issuer;

    uint64_t primary_key() const {
#ifdef COMMON
        return supply.get_symbol().value();
#else
        return supply.symbol.code().raw();
#endif
    }
};
#ifdef COMMON
FC_REFLECT(currency_stats, (supply)(max_supply)(issuer));
#endif


struct setting {
    ds_ulong id;
    ds_long time_shift;
    ds_account eosdtcntract_account;
    ds_account liquidator_account;
    ds_account oraclize_account;
    ds_account nutoken_account;
    ds_asset min_proposal_weight;
    ds_uint freeze_period;
    double min_participation;
    double success_margin;
    ds_ulong top_holders_amount;

    ds_ulong primary_key() const { return id; }
};

#ifdef COMMON
FC_REFLECT(setting, (id)(time_shift)(eosdtcntract_account)(liquidator_account)(oraclize_account)(nutoken_account)(min_proposal_weight)(freeze_period)(min_participation)(success_margin)(top_holders_amount));
#endif

struct oracle_rate {
    ds_asset rate;
    ds_time last_update;
    ds_time master_update;
    ds_time slave_update;
    ds_time onerror_update;

    uint64_t primary_key() const {

#ifdef COMMON
        return rate.get_symbol().value();
#else
        return rate.symbol.raw();
#endif
    }
};

#ifdef COMMON
FC_REFLECT(oracle_rate, (rate)(last_update)(master_update)(slave_update)(onerror_update));
#endif

constexpr static ds_uint SIX_MONTHS_IN_SECONDS = (ds_uint) (6 * (365.25 / 12) * 24 * 60 * 60);

#define PROPOSALS "proposals"

struct proposal {
    ds_account proposal_name;
    ds_account proposer;
    ds_string title;
    ds_string proposal_json;
    ds_time created_at;
    ds_time expires_at;

    auto primary_key() const { return proposal_name.value; }

    uint64_t by_proposer() const { return proposer.value; }

};

#ifdef COMMON
FC_REFLECT(proposal, (proposal_name)(proposer)(title)(proposal_json)(created_at)(expires_at));
#endif

struct voter {
    ds_asset voting_amount;

    ds_ulong primary_key() const {
#ifdef COMMON
        return voting_amount.get_symbol().to_symbol_code();
#else
        return voting_amount.symbol.code().raw();
#endif
    }
};

#ifdef COMMON
FC_REFLECT(voter, (voting_amount));
#endif

static __uint128_t compress_key(const ds_ulong &left, const ds_ulong &right) {
    return ((__uint128_t) left) << 64 | right;
}

struct vote {
    ds_ulong id;
    ds_account proposal_name;
    ds_account voter;
    uint8_t vote;
    ds_time updated_at;
    ds_asset quantity;

    auto primary_key() const { return id; }

    __uint128_t by_proposal() const { return compress_key(proposal_name.value, voter.value); }

    __uint128_t by_voter() const { return compress_key(voter.value, proposal_name.value); }
};

#ifdef COMMON
FC_REFLECT(vote, (id)(proposal_name)(voter)(vote)(updated_at)(quantity));
#endif

#endif
