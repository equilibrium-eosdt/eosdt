using eosio::check;

namespace eosdt {

    void eosdtsttoken::issueradd(ds_account issuer,
                                 ds_asset maximum_supply) {
        require_auth(_self);
        check(is_account(issuer), "issuer does not exists as an account.");
        auto sym = maximum_supply.symbol;
        check(sym.is_valid(), "invalid symbol name");
        check(maximum_supply.is_valid(), "invalid supply");
        check(maximum_supply.amount > 0, "max-supply must be positive");

        ds_asset supply(0,sym);

        stats statstable(_self, maximum_supply.symbol.code().raw());
        auto itr = statstable.find(maximum_supply.symbol.code().raw());
        check(itr != statstable.end(), "token with symbol does not exists");

        tokissuers_table tokissuers(_self, _self.value);
        if (tokissuers.begin() == tokissuers.end()
            && (itr->max_supply.amount != 0)) {
            maximum_supply = itr->max_supply;
            supply = itr->supply;
        } else {
            statstable.modify(itr, _self, [&](auto &s) {
                s.max_supply += maximum_supply;
            });
        }

        check(tokissuers.find(issuer.value) == tokissuers.end(), "issuer already exists as an issuer");
        tokissuers.emplace(_self, [&](auto &i) {
            i.issuer = issuer;
            i.supply = supply;
            i.max_supply = maximum_supply;
            i.is_active = true;
        });

    }

    ds_account eosdtsttoken::findissuer() {
        tokissuers_table tokissuers(_self, _self.value);
        for (auto itr = tokissuers.begin(); itr != tokissuers.end(); itr++) {
            if (has_auth(itr->issuer)) {
                check(itr->is_active, "issuer is disabled.");
                return itr->issuer;
            }
        }
        check(false, "issuer with auth did not found.");
        return ds_account();
    }

    void eosdtsttoken::issuer_issue(ds_account issuer, ds_asset quantity) {
        tokissuers_table tokissuers(_self, _self.value);
        auto itr = tokissuers.find(issuer.value);
        check(itr != tokissuers.end() && itr->is_active, "issuer does not exists as an issuer or disabled.");

        check(quantity.symbol == itr->supply.symbol, "symbol precision mismatch");
        check(quantity.amount <= itr->max_supply.amount - itr->supply.amount,
              "quantity exceeds available supply for issuer");

        tokissuers.modify(itr, _self, [&](auto &i) {
            i.supply += quantity;
        });

    }

    void eosdtsttoken::issuer_retire(ds_account issuer, ds_asset quantity) {
        tokissuers_table tokissuers(_self, _self.value);
        auto itr = tokissuers.find(issuer.value);
        check(itr != tokissuers.end(), "issuer does not exists as an issuer");

        check(quantity.symbol == itr->supply.symbol, "symbol precision mismatch");
        check(quantity.amount <= itr->supply.amount, "quantity exceeds available supply for an issuer");

        tokissuers.modify(itr, _self, [&](auto &i) {
            i.supply -= quantity;
        });

    }

    void eosdtsttoken::issuerrem(ds_account issuer) {
        require_auth(_self);

        tokissuers_table tokissuers(_self, _self.value);
        auto itr = tokissuers.find(issuer.value);
        check(itr != tokissuers.end(), "issuer does not exists as an issuer");
        check(itr->is_active, "issuer already deactivated");
        check(itr->supply.amount == 0l, "issuer hasn't burnt its supply");

        tokissuers.modify(itr, _self, [&](auto &i) {
            i.is_active = false;
        });

        stats statstable(_self, itr->max_supply.symbol.code().raw());
        auto stat_itr = statstable.find(itr->max_supply.symbol.code().raw());
        check(stat_itr != statstable.end(), "token with symbol does not exists");

        statstable.modify(stat_itr, _self, [&](auto &s) {
            s.max_supply -= itr->max_supply;
            check(s.max_supply.amount >= 0l, "cant decrease max_supply");
        });
    }

}
