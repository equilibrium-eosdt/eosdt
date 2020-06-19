using eosio::check;

namespace eosdt {

    void eosdtsttoken::create(ds_account issuer,
                              ds_asset maximum_supply) {
        require_auth(_self);

        auto sym = maximum_supply.symbol;
        check(sym.is_valid(), "invalid symbol name");
        check(maximum_supply.is_valid(), "invalid supply");
        check(maximum_supply.amount > 0, "max-supply must be positive");

        stats statstable(_self, sym.code().raw());
        auto existing = statstable.find(sym.code().raw());
        check(existing == statstable.end(), "token with symbol already exists");

        statstable.emplace(_self, [&](auto &s) {
            s.supply.symbol = maximum_supply.symbol;
            s.max_supply.symbol = maximum_supply.symbol;
            s.issuer = _self;
        });
    }

    void eosdtsttoken::issue(ds_account to, ds_asset quantity, ds_string memo) {
        auto sym = quantity.symbol;
        check(sym.is_valid(), "invalid symbol name");
        check(memo.size() <= 256, "memo has more than 256 bytes");

        auto sym_name = sym.code().raw();
        stats statstable(_self, sym_name);
        auto existing = statstable.find(sym_name);
        check(existing != statstable.end(), "token with symbol does not exist, create token before issue");
        const auto &st = *existing;

        require_auth(to);
        check(quantity.is_valid(), "invalid quantity");
        check(quantity.amount > 0, "must issue positive quantity");

        check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
        check(quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

        statstable.modify(st, SAME_PAYER, [&](auto &s) {
            s.supply += quantity;
        });
        
        add_balance(to, quantity, to);
        issuer_issue(to, quantity);
    }

    void eosdtsttoken::transfer(ds_account from,
                                ds_account to,
                                ds_asset quantity,
                                ds_string memo) {
        check(from != to, "cannot transfer to self");
        require_auth(from);

        if (!is_account( to )) {
            check(false, "to account does not exist");
        }
        auto sym = quantity.symbol.code().raw();
        stats statstable(_self, sym);
        const auto &st = statstable.get(sym);

        require_recipient(from);
        require_recipient(to);

        check(quantity.is_valid(), "invalid quantity");
        check(quantity.amount > 0, "must transfer positive quantity");
        check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
        check(memo.size() <= 256, "memo has more than 256 bytes");

        auto payer = has_auth(to) ? to : from;

        sub_balance(from, quantity);
        add_balance(to, quantity, payer);
    }

    void eosdtsttoken::sub_balance(ds_account owner, ds_asset value) {
        accounts from_acnts(_self, owner.value);
        const auto &from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
        check(from.balance.amount >= value.amount, "overdrawn EOSDT balance");

        if (from.balance.amount == value.amount) {
            from_acnts.erase(from);
        } else {
            from_acnts.modify(from, SAME_PAYER, [&](auto &a) {
                a.balance -= value;
            });
        }
    }

    void eosdtsttoken::add_balance(ds_account owner, ds_asset value, ds_account ram_payer) {
        accounts to_acnts(_self, owner.value);
        auto to = to_acnts.find(value.symbol.code().raw());
        if (to == to_acnts.end()) {
            to_acnts.emplace(ram_payer, [&](auto &a) {
                a.balance = value;
            });
        } else {
            to_acnts.modify(to, SAME_PAYER, [&](auto &a) {
                a.balance += value;
            });
        }
    }

    ds_asset eosdtsttoken::get_supply(ds_symbol sym) const {
        stats statstable(_self, sym.code().raw());
        const auto &st = statstable.get(sym.code().raw());
        return st.supply;
    }

    ds_asset eosdtsttoken::get_balance(ds_account owner, ds_symbol sym) const {
        accounts accountstable(_self, owner.value);
        const auto &ac = accountstable.get(sym.code().raw());
        return ac.balance;
    }

    void eosdtsttoken::open(const ds_account &owner, const ds_symbol &symbol, const ds_account &ram_payer) {
        require_auth(ram_payer);

        check(is_account(owner), "owner account does not exist");

        auto sym_code_raw = symbol.code().raw();
        stats statstable(get_self(), sym_code_raw);
        const auto &st = statstable.get(sym_code_raw, "symbol does not exist");
        check(st.supply.symbol == symbol, "symbol precision mismatch");

        accounts acnts(get_self(), owner.value);
        auto it = acnts.find(sym_code_raw);
        if (it == acnts.end()) {
            acnts.emplace(ram_payer, [&](auto &a) {
                a.balance.symbol = symbol;
            });
        }
    }

    void eosdtsttoken::close(const ds_account &owner, const ds_symbol &symbol) {
        require_auth(owner);
        accounts acnts(get_self(), owner.value);
        auto it = acnts.find(symbol.code().raw());
        check(it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect.");
        check(it->balance.amount == 0, "Cannot close because the balance is not zero.");
        acnts.erase(it);
    }

} /// namespace eosio

#ifndef TYPES_HPP
EOSIO_DISPATCH( eosio::token, (create)(issue)(transfer) )
#endif
