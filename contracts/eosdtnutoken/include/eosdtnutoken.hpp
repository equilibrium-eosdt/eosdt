#pragma once
#include "defines.hpp"

namespace eosdt {

    class [[eosio::contract("eosdtnutoken")]] eosdtnutoken : public eosio::contract {

    public:
        eosdtnutoken(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) :
            eosio::contract(receiver, code, ds) {
        }

        void create(ds_account issuer,
                    ds_asset maximum_supply);

        void open(const ds_account &owner, const ds_symbol &symbol, const ds_account &ram_payer);
        void close(const ds_account &owner, const ds_symbol &symbol);

        void issue(ds_account to, ds_asset quantity, ds_string memo);

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

        ds_asset hash_by_nutlocks_balance();

    public:
        void burn(ds_account from,
                  ds_asset quantity,
                  ds_string memo);

        void receive(ds_account from,
                     ds_account via,
                     ds_account to,
                     ds_asset quantity,
                     ds_string memo);

        void lockacc(std::vector <ds_account> &lock_list);
        void unlockacc(std::vector <ds_account> &unlock_list);

        void paraminit(const ds_asset &checksumm);

        struct nutlock
        {
            ds_account account;

            uint64_t
            primary_key() const { return account.value; }
        };
        struct nutparam {
            uint64_t param_id;
            ds_asset burnt_supply;
            ds_asset locked_supply;

            uint64_t primary_key() const { return param_id; }
        };
        typedef eosio::multi_index<"nutlocks"_n, nutlock> nutlocks_table;
        typedef eosio::multi_index<"nutparams"_n, nutparam> nutparams_table;
        struct nutstable {
            ds_account account;
            uint8_t is_own;
            ds_string peg;

            uint64_t
            primary_key() const { return account.value; }
        };
        typedef eosio::multi_index<"nutstables"_n, nutstable> nutstables_table;
        void stableadd(ds_account account, uint8_t is_own, ds_string peg);
        void stablerem(ds_account account);
    };
}
