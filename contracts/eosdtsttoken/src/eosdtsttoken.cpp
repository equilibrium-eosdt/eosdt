#define TYPES_HPP
#define EOSDTCNTRACT "eosdtcntract"_n

#include "eosdtsttoken.hpp"
#include "eosio.token.cpp"
#include "issuers.cpp"

using eosio::check;

namespace eosdt {
    ACTION eosdtsttoken::retire(ds_account from,
                                ds_asset quantity,
                                ds_string memo) {
        if (memo.find("via:") != 0) {
            retirefrom(findissuer(), from, quantity, memo);
        }else{
            retirefrom(ds_account(memo.c_str()+4), from, quantity, memo);
        }
    }

    ACTION eosdtsttoken::retirefrom(ds_account issuer, ds_account from,
                                    ds_asset quantity,
                                    ds_string memo) {
        eosio::print("\r\nretire(EOSDT) { from: ");
        eosio::print(from);
        eosio::print(", issuer: ");
        eosio::print(issuer);
        eosio::print(", quantity: ");
        eosio::print(quantity);
        eosio::print(", memo: ");
        eosio::print(memo);
        eosio::print(" }.");

        require_auth(issuer);
        if (issuer != from) {
            require_auth(from);
        }
        auto sym = quantity.symbol.code().raw();
        stats statstable(_self, sym);
        const auto &st = statstable.get(sym);

        require_recipient(from);

        check(quantity.is_valid(), "invalid quantity");
        check(quantity.amount > 0, "must retire positive quantity");
        check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
        check(memo.size() <= 256, "memo has more than 256 bytes");

        check(quantity.amount <= st.supply.amount, "quantity exceeds available supply");

        sub_balance(from, quantity);

        statstable.modify(st, ds_account(0), [&](auto &s) {
            s.supply -= quantity;
        });
        eosio::print("\r\n sub_balance finished");
        issuer_retire(issuer, quantity);
        eosio::print("\r\n retire finished");
    }

    ACTION eosdtsttoken::fixstat(ds_symbol symbol) {
        require_auth(_self);

        stats statstable(_self, symbol.code().raw());
        auto itr = statstable.find(symbol.code().raw());
        check(itr != statstable.end(), "token with symbol does not exists");

        statstable.modify(itr, _self, [&](auto &s) {
            s.issuer = _self;
        });
    }
};

EOSIO_DISPATCH(eosdt::eosdtsttoken,
(create)(open)(close)(issue)(issuefrom)(transfer)(retire)(retirefrom)(fixstat)
(issueradd)(issuerrem)
)
