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
#define ds_public_key eosio::chain::public_key_type

#else

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/transaction.hpp>
#include <eosio/print.hpp>
#include <eosio/system.hpp>
#include <eosio/crypto.hpp>

#define ds_time eosio::time_point_sec
#define ds_account eosio::name
#define ds_symbol eosio::symbol
#define ds_asset eosio::asset
#define ds_checksum capi_checksum256
#define ds_public_key eosio::public_key
#define N(X) name{#X}
#endif

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
#define REX_SYMBOL_STR "REX"
#define REX_SYMBOL_DECIMAL 4
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
#define ORACLERATES_STR "orarates"
#define EOSDTLIQDATR_STR "eosdtliqdatr"
#define EOSDTNUTOKEN_STR "eosdtnutoken"
#define EOSDTSTTOKEN_STR "eosdtsttoken"

#ifdef COMMON
#define EOSCTRACT N(eosio.token)
#define EOSDTCNTRACT N(eosdtcntract)
#define EOSDTORCLIZE N(eosdtorclize)
#define ORACLERATES N(orarates)
#define EOSDTLIQDATR N(eosdtliqdatr)
#define EOSDTNUTOKEN N(eosdtnutoken)
#define EOSDTSTTOKEN N(eosdtsttoken)
#define EOSDTFORUM N(eosdtgovernc)
#define EOS_SYMBOL ds_symbol(EOS_SYMBOL_DECIMAL,EOS_SYMBOL_STR)
#define EOS_SYMBOL_VALUE ::eosio::chain::string_to_symbol_c(EOS_SYMBOL_DECIMAL,EOS_SYMBOL_STR)
#define REX_SYMBOL ds_symbol(REX_SYMBOL_DECIMAL,REX_SYMBOL_STR)
#define REX_SYMBOL_VALUE ::eosio::chain::string_to_symbol_c(REX_SYMBOL_DECIMAL,REX_SYMBOL_STR)
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
#define ORACLERATES "orarates"_n
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

#define SETTINGS "govsettings"
#define blockproduceproposal "blockproduce"_n

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


struct govsetting {
    ds_ulong setting_id;
    ds_account eosdtcntract_account;
    ds_asset min_proposal_weight;
    ds_uint freeze_period;
    double min_participation;
    double success_margin;
    ds_uint top_holders_amount;
    ds_uint max_bp_count;
    ds_uint max_bp_votes;
    ds_asset min_vote_stake;
    ds_uint unstake_period;

    ds_ulong primary_key() const { return setting_id; }
};

#ifdef COMMON
FC_REFLECT(govsetting, (setting_id)(eosdtcntract_account)
(min_proposal_weight)(freeze_period)(min_participation)(success_margin)(top_holders_amount)
(max_bp_count)(max_bp_votes)(min_vote_stake)(unstake_period));
#endif

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

constexpr static ds_uint SIX_MONTHS_IN_SECONDS = (ds_uint) (6 * (365.25 / 12) * 24 * 60 * 60);

#define PROPOSALS "proposals"

struct proposal {
    ds_account proposal_name;
    ds_account proposer;
    ds_string title;
    ds_string proposal_json;
    ds_time created_at;
    ds_time expires_at;
    uint8_t proposal_type;

    auto primary_key() const { return proposal_name.value; }

    uint64_t by_proposer() const { return proposer.value; }

};

#ifdef COMMON
FC_REFLECT(proposal, (proposal_name)(proposer)(title)(proposal_json)(created_at)(expires_at)(proposal_type));
#endif

struct voter {
    ds_asset voting_amount;
    ds_time withdrawal_date;

    ds_ulong primary_key() const {
#ifdef COMMON
        return voting_amount.get_symbol().to_symbol_code();
#else
        return voting_amount.symbol.code().raw();
#endif
    }
};

#ifdef COMMON
FC_REFLECT(voter, (voting_amount)(withdrawal_date));
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
    ds_string vote_json;

    auto primary_key() const { return id; }

    __uint128_t by_proposal() const { return compress_key(proposal_name.value, voter.value); }

    __uint128_t by_voter() const { return compress_key(voter.value, proposal_name.value); }
};

#ifdef COMMON
FC_REFLECT(vote, (id)(proposal_name)(voter)(vote)(updated_at)(vote_json));
#endif


struct producer_info {
    ds_account          owner;
    double              total_votes;
    ds_public_key   producer_key;
    bool                is_active;
    ds_string           url;
    ds_uint             unpaid_blocks;
    ds_ulong            last_claim_time;
    uint16_t            location;
    auto primary_key() const { return owner.value; }
};
#ifdef COMMON
FC_REFLECT( producer_info, (owner)(total_votes)(producer_key)(is_active)(url)
        (unpaid_blocks)(last_claim_time)(location))
#endif

struct bpvote
{
    ds_account producer;
    ds_asset votes;

    uint64_t primary_key() const { return producer.value; }

#ifdef COMMON
    uint64_t byvotes() const { return votes.get_amount(); }
#else
    uint64_t byvotes() const { return votes.amount; }
#endif
};

#ifdef COMMON
FC_REFLECT(bpvote, (producer)(votes));
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
#endif
