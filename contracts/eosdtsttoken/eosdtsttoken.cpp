#include "eosio.token.cpp"


class eosdtsttoken :public eosio::token {

public:
    eosdtsttoken(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) :
            token(receiver, code, ds) {
    }

    ACTION burn(ds_account from,
                ds_asset quantity,
                ds_string memo){
        require_auth( from );
        auto sym = quantity.symbol.code().raw();
        stats statstable( _self, sym );
        const auto& st = statstable.get( sym );

        require_recipient( from );

        eosio_assert( quantity.is_valid(), "invalid quantity" );
        eosio_assert( quantity.amount > 0, "must transfer positive quantity" );
        eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
        eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

        eosio_assert( quantity.amount <= st.supply.amount, "quantity exceeds available supply");

        sub_balance( from, quantity );

        statstable.modify( st, ds_account(0), [&]( auto& s ) {
            s.supply -= quantity;
        });
    }

};

EOSIO_DISPATCH(eosdtsttoken,
(create)(issue)(transfer)(burn))
