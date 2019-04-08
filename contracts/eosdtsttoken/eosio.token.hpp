/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once


#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>

#include <string>


#ifndef ds_account
#define ds_account eosio::name
#endif

#ifndef ds_asset
#define ds_asset eosio::asset
#endif

#ifndef ds_symbol
#define ds_symbol eosio::symbol
#endif

#ifndef ds_string
#define ds_string std::string
#endif

namespace eosiosystem {
    class system_contract;
}

namespace eosio {

    using std::string;

    class token : public contract {
    public:
        token(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) :
                contract(receiver, code, ds) {
        }

        void create(ds_account issuer,
                    ds_asset maximum_supply);

        void issue(ds_account to, ds_asset quantity, string memo);

        void transfer(ds_account from,
                      ds_account to,
                      ds_asset quantity,
                      ds_string memo);

        inline ds_asset get_supply(ds_symbol sym) const;

        inline ds_asset get_balance(ds_account owner, ds_symbol sym) const;

        struct account {
            ds_asset balance;

            uint64_t primary_key() const { return balance.symbol.code().raw(); }
        };

        struct currency_stats {
            ds_asset supply;
            ds_asset max_supply;
            ds_account issuer;

            uint64_t primary_key() const { return supply.symbol.code().raw(); }
        };

        typedef eosio::multi_index<"accounts"_n, account> accounts;
        typedef eosio::multi_index<"stat"_n, currency_stats> stats;

    protected:
        void sub_balance(ds_account owner, ds_asset value);

        void add_balance(ds_account owner, ds_asset value, ds_account ram_payer);

    public:
        struct transfer_args {
            ds_account from;
            ds_account to;
            ds_asset quantity;
            string memo;
        };
    };

    ds_asset token::get_supply(ds_symbol sym) const {
        stats statstable(_self, sym.code().raw());
        const auto &st = statstable.get(sym.code().raw());
        return st.supply;
    }

    ds_asset token::get_balance(ds_account owner, ds_symbol sym) const {
        accounts accountstable(_self, owner.value);
        const auto &ac = accountstable.get(sym.code().raw());
        return ac.balance;
    }

} /// namespace eosio
