#define TYPES_HPP
#define EOSDTCNTRACT "eosdtcntract"_n

#include "eosdtnutoken.hpp"
#include "eosio.token.cpp"
#include "params.cpp"
#include "stables.cpp"

using eosio::check;

namespace eosdt {

    ACTION eosdtnutoken::issue( ds_account to, ds_asset quantity, ds_string memo )
    {
        auto sym = quantity.symbol;
        check( sym.is_valid(), "invalid symbol name" );
        check( memo.size() <= 256, "memo has more than 256 bytes" );

        auto sym_name = sym.code().raw();
        stats statstable( _self, sym_name );
        auto existing = statstable.find( sym_name );
        check( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
        const auto& st = *existing;

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
        eosio::print("\r\nissue{ to: ");
        eosio::print(to);
        eosio::print(", quantity: ");
        eosio::print(quantity);
        eosio::print(", max_supply: ");
        eosio::print(st.max_supply);
        eosio::print(", supply: ");
        eosio::print(st.supply);
        eosio::print(", available: ");
        eosio::print(available);
        eosio::print(" }");

        check( quantity.amount <= available.amount, "quantity exceeds available supply");

        statstable.modify( st, ds_account(0), [&]( auto& s ) {
            s.supply += quantity;
        });

        nutparams_table nutparams(_self, _self.value);
        if(nutparams.begin() == nutparams.end())
        {
            nutparams.emplace( _self, [&]( auto& s ) {
                s.param_id = 0;
                s.burnt_supply.symbol = quantity.symbol;
                s.locked_supply.symbol = quantity.symbol;
            });
        }

        add_balance( st.issuer, quantity, st.issuer );

        if( to != st.issuer ) {
            SEND_INLINE_ACTION( *this, transfer, {st.issuer,"active"_n}, {st.issuer, to, quantity, memo} );
        }
    }

    ACTION eosdtnutoken::burn(ds_account from,
                       ds_asset        quantity,
                       ds_string       memo){
        require_auth( from );
        auto sym = quantity.symbol.code().raw();
        stats statstable( _self, sym );
        const auto& st = statstable.get( sym );

        require_recipient( from );

        check( quantity.is_valid(), "invalid quantity" );
        check( quantity.amount > 0, "must transfer positive quantity" );
        check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
        check( memo.size() <= 256, "memo has more than 256 bytes" );

        check( quantity.amount <= st.supply.amount, "quantity exceeds available supply");

        sub_balance( from, quantity );

        statstable.modify( st, ds_account(0), [&]( auto& s ) {
            s.supply -= quantity;
        });

        if (quantity.symbol == NUT_SYMBOL) {
            nutparams_table nutparams(_self, _self.value);
            auto param_itr = nutparams.find(0);
            nutparams.modify(param_itr, ds_account(0), [&](auto &s) {
                s.burnt_supply += quantity;
            });
        }
    }


    ACTION eosdtnutoken::receive(ds_account from,
                       ds_account via,
                       ds_account to,
                       ds_asset        quantity,
                       ds_string       memo) {
        check( from != to, "cannot transfer to self" );
        require_auth( via );
        eosio::print("\r\nreceive(NUT) { from: ");
        eosio::print(from);
        eosio::print(" , via: ");
        eosio::print(via);
        eosio::print(" , to: ");
        eosio::print(to);
        eosio::print(" , quantity: ");
        eosio::print(quantity);
        eosio::print(" , memo: '");
        eosio::print(memo);
        eosio::print("'}");


        auto authorized = false;
        nutstables_table nutstables(_self,_self.value);
        for (auto stable_itr = nutstables.begin(); stable_itr != nutstables.end(); stable_itr++) {
            if (!stable_itr->is_own) {
                continue;
            }
            struct tokissuer
            {
                ds_account issuer;
                ds_asset max_supply;
                ds_asset supply;
                bool is_active;
                uint64_t primary_key() const { return issuer.value; }
            };
            eosio::multi_index<"tokissuers"_n, tokissuer> tokissuers(stable_itr->account, stable_itr->account.value);
            auto issuer_itr = tokissuers.find(via.value);
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


        add_balance( from, -quantity,to );
        add_balance( to, quantity, to );

    }
}

EOSIO_DISPATCH(eosdt::eosdtnutoken,
(create)(open)(close)(issue)(transfer)(burn)(receive)(lockacc)(unlockacc)(paraminit)(stableadd)(stablerem)
)
