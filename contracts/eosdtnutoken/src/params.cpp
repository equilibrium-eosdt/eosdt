#include "eosdtnutoken.hpp"
#include <eosio/crypto.hpp>

namespace eosdt {

    void eosdtnutoken::lockacc(std::vector <ds_account> &lock_list) {
        require_auth(_self);
        nutlocks_table nutlocks(_self, _self.value);
        nutparams_table nutparams(_self, _self.value);
        auto param_itr = nutparams.find(0);
        auto supply = ds_asset(0, NUT_SYMBOL);
        for (auto acc : lock_list) {
            auto value = acc.value;
            auto existing = nutlocks.find(value);
            check(existing == nutlocks.end(), "account lock already exists");
            nutlocks.emplace(_self, [&](auto &s) {
                s.account = acc;
            });

            accounts acnts(_self, value);
            auto acnts_itr = acnts.find(NUT_SYMBOL.code().raw());
            if (acnts_itr != acnts.end()) {
                supply += acnts_itr->balance;
            }
        }
        if (param_itr != nutparams.end()) {
            nutparams.modify(param_itr, SAME_PAYER, [&](auto &s) {
                s.locked_supply += supply;
                check(s.locked_supply.amount <= get_supply(NUT_SYMBOL).amount, "quantity exceeds available supply");
            });
        }
    }

    void eosdtnutoken::unlockacc(std::vector <ds_account> &unlock_list) {
        require_auth(_self);
        nutlocks_table nutlocks(_self, _self.value);
        nutparams_table nutparams(_self, _self.value);
        auto param_itr = nutparams.find(0);
        auto supply = ds_asset(0, NUT_SYMBOL);
        for (auto acc : unlock_list) {
            auto value = acc.value;
            auto existing = nutlocks.find(value);
            check(existing != nutlocks.end(), "account lock doesn't exist");
            nutlocks.erase(existing);

            accounts acnts(_self, value);
            auto acnts_itr = acnts.find(NUT_SYMBOL.code().raw());
            if (acnts_itr != acnts.end()) {
                supply += acnts_itr->balance;
            }
        }
        nutparams.modify(param_itr, SAME_PAYER, [&](auto &s) {
            s.locked_supply -= supply;
            check(s.locked_supply.amount >= 0l, "overdrawn NUT balance");
        });
    }

    ds_asset eosdtnutoken::hash_by_nutlocks_balance() {
        nutlocks_table nutlocks(_self, _self.value);
        ds_asset hash(0, NUT_SYMBOL);
        auto index = 0;
        auto supply = ds_asset(0, NUT_SYMBOL);
        for (auto itr = nutlocks.begin(); itr != nutlocks.end(); itr++) {
            accounts acnts(_self, itr->account.value);
            auto acnts_itr = acnts.find(NUT_SYMBOL.code().raw());
            if (acnts_itr != acnts.end()) {
                supply += acnts_itr->balance;
                hash += (++index) * acnts_itr->balance;
            }
        }
        return hash;
    }

    void eosdtnutoken::paraminit(const ds_asset &checksumm) {
        nutlocks_table nutlocks(_self, _self.value);
        nutparams_table nutparams(_self, _self.value);
        auto param_itr = nutparams.begin();
        check(param_itr == nutparams.end(), "nutparams already exist");

        auto hash = hash_by_nutlocks_balance();
        if (checksumm.amount != hash.amount) {
            check(false, "wrong checksumm");
        }

        auto supply = ds_asset(0, NUT_SYMBOL);
        for (auto itr = nutlocks.begin(); itr != nutlocks.end(); itr++) {
            accounts acnts(_self, itr->account.value);
            auto acnts_itr = acnts.find(NUT_SYMBOL.code().raw());
            if (acnts_itr != acnts.end()) {
                supply += acnts_itr->balance;
            }
        }

        auto sym = NUT_SYMBOL.code().raw();
        stats statstable(_self, sym);
        auto stat_itr = statstable.find(sym);
        auto burnt_supply = stat_itr->max_supply - supply;
        statstable.modify(stat_itr, SAME_PAYER, [&](auto &s) {
            s.supply = supply;
        });

        nutparams.emplace(_self, [&](auto &s) {
            s.param_id = SINGLETON_ID;
            s.burnt_supply = burnt_supply;
            s.locked_supply = supply;
        });
    }

}