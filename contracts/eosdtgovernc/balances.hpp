#include "oracle.hpp"
class balances : public oracle {
private:
    typedef eosio::multi_index<"accounts"_n, account> accounts_table;
    typedef eosio::multi_index<"stat"_n, currency_stats> currency_stats_table;
    typedef eosio::multi_index<"voters"_n, voter> voters_table;
protected:
    auto ctract_by_symbol(const ds_symbol &symbol) {
        if (symbol == UTILITY_SYMBOL) {
            return settings_get().nutoken_account;
        }
        return EOSCTRACT;
    }
    auto supply_get(const ds_symbol &symbol)
    {
        auto id = symbol.code().raw();
        currency_stats_table currencies(ctract_by_symbol(symbol), id);
        auto itr = currencies.find(id);
        if (itr == currencies.end()) {
            return ds_asset(0ll, symbol);
        }
        return itr->supply;
    }
    auto balance_get(const ds_account &account, const ds_symbol &symbol) {
        accounts_table accounts(ctract_by_symbol(symbol), account.value);
        auto itr = accounts.find(symbol.code().raw());
        if (itr == accounts.end()) {
            return ds_asset(0ll, symbol);
        }
        return itr->balance;
    }
    ds_asset voting_amount_get(const ds_account &owner, const ds_symbol &sym) const {
        voters_table voters(_self, owner.value);
        const auto itr = voters.find(sym.code().raw());
        return itr == voters.end() ? ds_asset(0, sym) : itr->voting_amount;
    }
    void voting_amount_sub(ds_account owner, ds_asset value) {
        voters_table voters(_self, owner.value);
        const auto &from = voters.get(value.symbol.code().raw(), "no balance object found");
        ds_assert(from.voting_amount >= value, "overdrawn balance");
        if (from.voting_amount == value) {
            voters.erase(from);
        } else {
            voters.modify(from, owner, [&](auto &a) {
                a.voting_amount -= value;
            });
        }
    }
    void voting_amount_add(ds_account owner, ds_asset value, ds_account ram_payer) {
        voters_table voters(_self, owner.value);
        auto to = voters.find(value.symbol.code().raw());
        if (to == voters.end()) {
            voters.emplace(ram_payer, [&](auto &a) {
                a.voting_amount = value;
            });
        } else {
            voters.modify(to, ds_account(0), [&](auto &a) {
                a.voting_amount += value;
            });
        }
    }
    void trans(const ds_account &to, const ds_asset &quantity, const ds_string &memo) {
        auto ctract = ctract_by_symbol(quantity.symbol);
        auto balance = balance_get(_self, quantity.symbol);
        ds_print("\r\ntrans: {from: %, to: %, quantity: % ,by: %, memo: '%', before: %, after: %}",
                 _self, to, quantity, ctract, memo, balance, balance - quantity);
        if (quantity.amount <= 0) {
            return;
        }
        eosio::action(
                eosio::permission_level{_self, "active"_n},
                ctract,
                "transfer"_n,
                std::make_tuple(_self, to, quantity, memo)
        ).send();
    }
public:
    balances(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) :
            oracle(receiver, code, ds) {
    }
    void transfer(const ds_account &from,
                  const ds_account &to,
                  ds_asset &quantity,
                  const ds_string &memo) {
        PRINT_STARTED("transfer"_n)
        if (to != _self)
            return;
        ds_assert(quantity.symbol == UTILITY_SYMBOL, "wrong quantity: % expected:  %", quantity, UTILITY_SYMBOL);
        auto ctract_for_symbol = ctract_by_symbol(quantity.symbol);
        ds_assert(ctract_for_symbol == _code, "wrong contract % for asset: % expected: %.", _code, quantity,
                  ctract_for_symbol);
        voting_amount_add(from, quantity, _self);
        PRINT_FINISHED("transfer"_n)
    }
};
