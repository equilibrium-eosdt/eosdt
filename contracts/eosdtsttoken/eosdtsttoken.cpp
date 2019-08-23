#include "eosio.token.cpp"


using eosio::check;

class eosdtsttoken :public eosio::token {

public:
    eosdtsttoken(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) :
            token(receiver, code, ds) {
    }

    ACTION retire(ds_account from,
                ds_asset quantity,
                ds_string memo){
        eosio::print("\r\nretire(EOSDT) { from: ");
        eosio::print(from);
        eosio::print(", quantity: ");
        eosio::print(quantity);
        eosio::print(", memo: ");
        eosio::print(memo);
        eosio::print(" }.");

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
    }

};

EOSIO_DISPATCH(eosdtsttoken,
(create)(issue)(transfer)(retire))
