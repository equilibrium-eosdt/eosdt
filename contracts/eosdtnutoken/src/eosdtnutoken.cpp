#define TYPES_HPP

#include "eosdtnutoken.hpp"
#include "eosio.token.cpp"
#include "params.cpp"
#include "stables.cpp"

using eosio::check;

namespace eosdt {

    void eosdtnutoken::issue( ds_account to, ds_asset quantity, ds_string memo )
    {
        auto sym = quantity.symbol;
        check( sym.is_valid(), "invalid symbol name" );
        check( memo.size() <= 256, "memo has more than 256 bytes" );

        auto sym_name = sym.code().raw();
        stats statstable( _self, sym_name );
        auto existing = statstable.find( sym_name );
        check( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
        const auto& st = *existing;
        check( to == st.issuer, "nutokens can only be issued to issuer account" );

        require_auth( st.issuer );
        check( quantity.is_valid(), "invalid quantity" );
        check( quantity.amount > 0, "must issue positive quantity" );

        check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

        auto available = st.max_supply - st.supply;
        if (quantity.symbol == NUT_SYMBOL) {
            nutparams_table nutparams(_self, _self.value);
            auto param_itr = nutparams.find(0);
            if (param_itr != nutparams.end()) {
                available -= param_itr->burnt_supply;
            }
        }
        check( quantity.amount <= available.amount, "quantity exceeds available supply");

        statstable.modify( st, SAME_PAYER, [&]( auto& s ) {
            s.supply += quantity;
        });

        nutparams_table nutparams(_self, _self.value);
        if(nutparams.begin() == nutparams.end())
        {
            nutparams.emplace( _self, [&]( auto& s ) {
                s.param_id = SINGLETON_ID;
                s.burnt_supply.symbol = quantity.symbol;
                s.locked_supply.symbol = quantity.symbol;
            });
        }

        add_balance( st.issuer, quantity, st.issuer );
    }

    void eosdtnutoken::burn(ds_account from,
                       ds_asset        quantity,
                       ds_string       memo){

        auto sym = quantity.symbol;
        check( sym.is_valid(), "invalid symbol name" );
        check( memo.size() <= 256, "memo has more than 256 bytes" );

        auto sym_name = sym.code().raw();
        stats statstable( _self, sym_name );
        auto existing = statstable.find( sym_name );
        check( existing != statstable.end(), "token with symbol does not exist" );
        const auto& st = *existing;

        require_auth( from );
        check( quantity.is_valid(), "invalid quantity" );
        check( quantity.amount > 0, "must retire positive quantity" );
        check( quantity.amount <= st.supply.amount, "quantity exceeds available supply");
        check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

        statstable.modify( st, SAME_PAYER, [&]( auto& s ) {
            s.supply -= quantity;
        });

        sub_balance( from, quantity );

        if (quantity.symbol == NUT_SYMBOL) {
            nutparams_table nutparams(_self, _self.value);
            auto param_itr = nutparams.find(0);
            nutparams.modify(param_itr, SAME_PAYER, [&](auto &s) {
                s.burnt_supply += quantity;
            });
        }
    }


    void eosdtnutoken::receive(ds_account from,
                       ds_account to,
                       ds_asset        quantity,
                       ds_string       memo) {
        check( from != to, "cannot transfer to self" );
        require_auth( to );

        auto authorized = false;
        nutstables_table nutstables(_self,_self.value);
        for (auto stable_itr = nutstables.begin(); stable_itr != nutstables.end(); stable_itr++) {
            if (!stable_itr->is_own) {
                continue;
            }
            tokissuers_table tokissuers(stable_itr->account, stable_itr->account.value);
            auto issuer_itr = tokissuers.find(to.value);
            if (issuer_itr != tokissuers.end()) {
                authorized = true;
                break;
            }
        }
        check(authorized, "invalid receiver contract(via)");

        auto sym = quantity.symbol.code().raw();
        stats statstable( _self, sym );
        const auto& st = statstable.get( sym );

        require_recipient( from );
        require_recipient( to );

        check( quantity.is_valid(), "invalid quantity" );
        check( quantity.amount > 0, "must transfer positive quantity" );
        check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
        check( memo.size() <= 256, "memo has more than 256 bytes" );

        check(get_balance(from,quantity.symbol)>=quantity, "overdrawn NUT balance" );


        sub_balance( from, quantity );
        add_balance( to, quantity, to );

    }
}
