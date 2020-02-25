#include "eosdtnutoken.hpp"
#include <eosio/crypto.hpp>

namespace eosdt {

    void eosdtnutoken::sub_balance(ds_account owner, ds_asset value) {
        accounts from_acnts(_self, owner.value);

        const auto &from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
        check(from.balance.amount >= value.amount, "overdrawn NUT balance");


        if (from.balance.amount == value.amount) {
            from_acnts.erase(from);
        } else {
            from_acnts.modify(from, owner, [&](auto &a) {
                a.balance -= value;
            });
        }

        nutparams_table nutparams(_self, _self.value);
        check(nutparams.find(0) != nutparams.end(), "nutparams did not init");

        nutlocks_table nutlocks(_self, _self.value);
        auto lock_itr = nutlocks.find(owner.value);
        if (value.symbol == NUT_SYMBOL && lock_itr != nutlocks.end()) {
            check(lock_itr == nutlocks.end(), "not transfers from locked accounts");
        }
    }

    void eosdtnutoken::add_balance(ds_account owner, ds_asset value, ds_account ram_payer) {
        accounts to_acnts(_self, owner.value);
        auto to = to_acnts.find(value.symbol.code().raw());
        if (to == to_acnts.end()) {
            to_acnts.emplace(ram_payer, [&](auto &a) {
                a.balance = value;
            });
        } else {
            to_acnts.modify(to, ds_account(0), [&](auto &a) {
                a.balance += value;
            });
        }

        nutlocks_table nutlocks(_self, _self.value);
        auto lock_itr = nutlocks.find(owner.value);
        if (value.symbol == NUT_SYMBOL && lock_itr != nutlocks.end()) {
            nutparams_table nutparams(_self, _self.value);
            auto param_itr = nutparams.find(0);
            eosio::check(param_itr != nutparams.end(), "nutparams did not init");
            nutparams.modify(param_itr, ds_account(0), [&](auto &s) {
                s.locked_supply += value;
                check(0 <= s.locked_supply.amount, "quantity exceeds available locked supply");
            });
        }

    }

    ACTION eosdtnutoken::lockacc(std::vector <ds_account> &lock_list) {
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
            nutparams.modify(param_itr, ds_account(0), [&](auto &s) {
                s.locked_supply += supply;
                check(s.locked_supply.amount <= get_supply(NUT_SYMBOL).amount, "quantity exceeds available supply");
            });
        }
    }

    ACTION eosdtnutoken::unlockacc(std::vector <ds_account> &unlock_list) {
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
        nutparams.modify(param_itr, ds_account(0), [&](auto &s) {
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

    ACTION eosdtnutoken::paraminit(const ds_asset &checksumm) {
        nutlocks_table nutlocks(_self, _self.value);
        nutparams_table nutparams(_self, _self.value);
        auto param_itr = nutparams.begin();
        check(param_itr == nutparams.end(), "nutparams already exist");

        auto hash = hash_by_nutlocks_balance();
        if (checksumm.amount != hash.amount) {
            eosio::print("checksumm: ");
            eosio::print(checksumm);
            eosio::print(", hash: ");
            eosio::print(hash);
            check(checksumm == hash, "wrong checksumm");
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
        statstable.modify(stat_itr, ds_account(0), [&](auto &s) {
            s.supply = supply;
        });

        nutparams.emplace(_self, [&](auto &s) {
            s.param_id = 0;
            s.burnt_supply = burnt_supply;
            s.locked_supply = supply;
        });
    }

}
