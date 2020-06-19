#define TYPES_HPP

#include "eosdtsttoken.hpp"
#include "eosio.token.cpp"
#include "issuers.cpp"

using eosio::check;

namespace eosdt {

    void eosdtsttoken::retire(ds_account from,
                                ds_asset quantity,
                                ds_string memo) {

        auto sym = quantity.symbol;
        check( sym.is_valid(), "invalid symbol name" );
        check( memo.size() <= 256, "memo has more than 256 bytes" );

        auto sym_name = sym.code().raw();
        stats statstable(_self, sym_name);
        const auto &st = statstable.get(sym_name);

        require_auth(from);
        check(quantity.is_valid(), "invalid quantity");
        check(quantity.amount > 0, "must retire positive quantity");
        check(quantity.amount <= st.supply.amount, "quantity exceeds available supply");
        check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");

        statstable.modify(st, SAME_PAYER, [&](auto &s) {
            s.supply -= quantity;
        });

        sub_balance(from, quantity);
        issuer_retire(from, quantity);
    }

};
