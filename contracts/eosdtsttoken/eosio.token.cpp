#include "eosio.token.hpp"






namespace eosio {

void token::create( ds_account issuer,
                    asset maximum_supply )
{
    require_auth( _self );

    auto sym = maximum_supply.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( maximum_supply.is_valid(), "invalid supply");
    check( maximum_supply.amount > 0, "max-supply must be positive");

    stats statstable( _self, sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing == statstable.end(), "token with symbol already exists" );

    statstable.emplace( _self, [&]( auto& s ) {
       s.supply.symbol = maximum_supply.symbol;
       s.max_supply = maximum_supply;
       s.issuer = issuer;
    });
}


void token::issue( ds_account to, asset quantity, ds_string memo )
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
    check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify( st, ds_account(0), [&]( auto& s ) {
       s.supply += quantity;
    });

    add_balance( st.issuer, quantity, st.issuer );

    if( to != st.issuer ) {
       SEND_INLINE_ACTION( *this, transfer, {st.issuer,"active"_n}, {st.issuer, to, quantity, memo} );
    }
}

void token::transfer( ds_account from,
                      ds_account to,
                      asset quantity,
                      ds_string memo )
{
    check( from != to, "cannot transfer to self" );
    require_auth( from );

    check( is_account( to ), "to account does not exist");
    auto sym = quantity.symbol.code().raw();
    stats statstable( _self, sym );
    const auto& st = statstable.get( sym );

    require_recipient( from );
    require_recipient( to );

    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must transfer positive quantity" );
    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    auto payer = has_auth( to ) ? to : from;

    sub_balance( from, quantity );
    add_balance( to, quantity, payer );
}

void token::sub_balance( ds_account owner, asset value ) {
   accounts from_acnts( _self, owner.value );
   const auto& from = from_acnts.get( value.symbol.code().raw(), "no balance object found" );

   if (from.balance.amount < value.amount) {
       print("\r\nsub_balance(EOSDT) { owner: ");
       print(owner);
       print(", value: ");
       print(value);
       print(", balance: ");
       print(from.balance);
       print(" }.");
   }
   check( from.balance.amount >= value.amount, "overdrawn EOSDT balance" );


   if( from.balance.amount == value.amount ) {
      from_acnts.erase( from );
   } else {
      from_acnts.modify( from, owner, [&]( auto& a ) {
          a.balance -= value;
      });
   }
}

void token::add_balance( ds_account owner, asset value, ds_account ram_payer )
{
   accounts to_acnts( _self, owner.value );
   auto to = to_acnts.find( value.symbol.code().raw() );
   if( to == to_acnts.end() ) {
      to_acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = value;
      });
   } else {
      to_acnts.modify( to, ds_account(0), [&]( auto& a ) {
        a.balance += value;
      });
   }
}

}
