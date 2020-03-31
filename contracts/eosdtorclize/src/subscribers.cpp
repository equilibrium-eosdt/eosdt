#include "eosdtorclize.hpp"

namespace eosdt {
    void eosdtorclize::on_rate_changed(const ds_time &update, const ds_asset &rate, const ds_symbol &base) {
        orasubscribers_table orasubscribers(_self, _self.value);
        for (auto itr = orasubscribers.begin(); itr != orasubscribers.end(); itr++) {
            eosio::transaction t;
            t.delay_sec = 0;
            t.expiration = time_get() + 300;
            t.actions.emplace_back(
                    eosio::permission_level(_self, "active"_n),
                    EOSDTORCLIZE,
                    "ratechanged"_n,
                    std::make_tuple(itr->contract, update, rate, base)
            );

            auto id = (((uint128_t) rate.symbol.raw()) << 64) | (((uint128_t) itr->contract.value));
            auto deleted = eosio::cancel_deferred(id);
            ds_print("\r\ncancel: %, create: %, contract: %, update: %, rate: %, base: %", deleted, id, itr->contract, update, rate, base);
            t.send(id, _self);
        }
    }


    void eosdtorclize::transfer(const ds_account &from, const ds_account &to,
                                ds_asset &quantity, const ds_string &memo) {
        PRINT_STARTED("transfer"_n);
        if (to != _self)
            return;
        if (quantity.symbol == DAPP_SYMBOL) {
            return;
        }
        ds_assert(get_first_receiver() == ctrsetting_get().nutoken_account
                  && quantity.symbol == UTILITY_SYMBOL, "Wrong asset: %", quantity);
        ds_assert(to_double(quantity) >= 2.0, "Wrong quantity: %", quantity);
        ds_assert(memo == "subscribe", "Wrong memo value: %", memo);

        orasubscribers_table orasubscribers(_self, _self.value);

        auto itr = orasubscribers.find(from.value);
        if (itr == orasubscribers.end()) {
            orasubscribers.emplace(_self, [&](auto &row) {
                row.contract = from;
                row.quantity = quantity;
                row.withdrawal_date = time_get()+UNSUBSCRIBE_FREEZE_PERIOD;
            });
        } else {
            orasubscribers.modify(itr, ds_account(0), [&](auto &row) {
                row.quantity += quantity;
            });
        }

        PRINT_FINISHED("transfer"_n);
    }


    void eosdtorclize::unsubscribe(const ds_account &contract) {
        PRINT_STARTED("unsubscribe"_n);
        require_auth(contract);
        orasubscribers_table orasubscribers(_self, _self.value);

        auto itr = orasubscribers.find(contract.value);
        ds_assert(itr != orasubscribers.end(), "Wrong contract: %", contract);
        auto time = time_get();
        
            ds_assert(itr->withdrawal_date <= time, "Unsubscribe % more that current %.", itr->withdrawal_date, time);
            if (itr->quantity.amount > 0) {
                eosio::action(
                        eosio::permission_level{_self, "active"_n},
                        ctrsetting_get().nutoken_account,
                        "transfer"_n,
                        std::make_tuple(_self, contract, itr->quantity, ds_string("unsubscribe"))
                ).send();
            }
            orasubscribers.erase(itr);
  
        PRINT_STARTED("unsubscribe"_n);
    }

    void eosdtorclize::ratechanged(const ds_account &contract, const ds_time &update, const ds_asset &rate, const ds_symbol &base) {
        ds_print("\r\ncontract: %, update: %, rate: %.",
                 contract, update, rate);
        require_recipient(contract);
        orasubscribers_table orasubscribers(_self, _self.value);
        auto itr = orasubscribers.find(contract.value);
        auto time = time_get();
        if (itr != orasubscribers.end()) {
            orasubscribers.modify(itr, ds_account(0), [&](auto &row) {
                row.callcount++;
                row.lastcall = time;
            });
        }
    }
}
