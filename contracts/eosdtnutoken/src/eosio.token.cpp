using eosio::check;

namespace eosdt {

    void eosdtnutoken::create(ds_account issuer,
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
            s.max_supply = maximum_supply;
            s.issuer = issuer;
        });
    }

    void eosdtnutoken::transfer(ds_account from,
                                ds_account to,
                                ds_asset quantity,
                                ds_string memo) {
        check(from != to, "cannot transfer to self");
        require_auth(from);

        check(is_account(to), "to account does not exist");
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

    ds_asset eosdtnutoken::get_supply(ds_symbol sym) const {
        stats statstable(_self, sym.code().raw());
        const auto &st = statstable.get(sym.code().raw());
        return st.supply;
    }

    ds_asset eosdtnutoken::get_balance(ds_account owner, ds_symbol sym) const {
        accounts accountstable(_self, owner.value);
        const auto &ac = accountstable.get(sym.code().raw());
        return ac.balance;
    }

    void eosdtnutoken::open(const ds_account &owner, const ds_symbol &symbol, const ds_account &ram_payer) {
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

    void eosdtnutoken::close(const ds_account &owner, const ds_symbol &symbol) {
        require_auth(owner);
        accounts acnts(get_self(), owner.value);
        auto it = acnts.find(symbol.code().raw());
        check(it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect.");
        check(it->balance.amount == 0, "Cannot close because the balance is not zero.");
        acnts.erase(it);
    }
} /// namespace eosdt

