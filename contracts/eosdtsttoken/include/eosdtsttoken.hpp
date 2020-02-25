#pragma once
#include "defines.hpp"

namespace eosdt {

    class [[eosio::contract("eosdtsttoken")]] eosdtsttoken :public eosio::contract {

    public:
        eosdtsttoken(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) :
                eosio::contract(receiver, code, ds) {
        }

        void create(ds_account issuer,ds_asset maximum_supply);

        void open(const ds_account &owner, const ds_symbol &symbol, const ds_account &ram_payer);
        void close(const ds_account &owner, const ds_symbol &symbol);

        void issue(ds_account to, ds_asset quantity, ds_string memo);
        void issuefrom(ds_account issuer,ds_account to, ds_asset quantity, ds_string memo);

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

        ACTION retire(ds_account from,
                                    ds_asset        quantity,
                                    ds_string       memo);
        ACTION retirefrom(ds_account issuer,ds_account from,
                                        ds_asset        quantity,
                                        ds_string       memo);

        ACTION fixstat(ds_symbol symbol);

        void issueradd(ds_account issuer, ds_asset maximum_supply);
        void issuerrem(ds_account issuer);
    protected:
        void sub_balance(ds_account owner, ds_asset value);

        void add_balance(ds_account owner, ds_asset value, ds_account ram_payer);
        ds_account findissuer();
        void issuer_issue(ds_account issuer, ds_asset quantity);
        void issuer_retire(ds_account issuer, ds_asset quantity);
    public:
        struct transfer_args {
            ds_account from;
            ds_account to;
            ds_asset quantity;
            ds_string memo;
        };

        struct tokissuer
        {
            ds_account issuer;
            ds_asset max_supply;
            ds_asset supply;
            bool is_active;
            uint64_t
            primary_key() const { return issuer.value; }
        };
        typedef eosio::multi_index<"tokissuers"_n, tokissuer> tokissuers_table;
    };

} /// namespace eosio
