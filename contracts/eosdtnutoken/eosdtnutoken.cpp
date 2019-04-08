#include "eosio.token.cpp"
class eosdtnutoken :public eosio::token {
public:
    eosdtnutoken(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) :
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
    }
    ACTION receive(ds_account from,
                       ds_account to,
                       ds_asset quantity,
                       ds_string memo) {
        eosio_assert(to == "eosdtcntract"_n,"wrong contract");
        eosio_assert( from != to, "cannot transfer to self" );
        require_auth( to );
        eosio_assert( is_account( to ), "to account does not exist");
        auto sym = quantity.symbol.code().raw();
        stats statstable( _self, sym );
        const auto& st = statstable.get( sym );
        require_recipient( from );
        require_recipient( to );
        eosio_assert( quantity.is_valid(), "invalid quantity" );
        eosio_assert( quantity.amount > 0, "must transfer positive quantity" );
        eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
        eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );
        eosio_assert(get_balance(from,quantity.symbol)>=quantity, "overdrawn balance" );
        add_balance( from, -quantity,to );
        add_balance( to, quantity, to );
    }
};
EOSIO_DISPATCH(eosdtnutoken,
(create)(issue)(transfer)(burn)(receive))
