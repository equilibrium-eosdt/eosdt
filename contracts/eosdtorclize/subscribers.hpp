#include "settings.hpp"


class subscribers : public settings {
public:
    subscribers(ds_account receiver, ds_account code, datastream<const char *> ds) : settings(receiver, code, ds) {
    }

private:
    typedef eosio::multi_index<"subscribers"_n, subscribe> subscribers_table;
protected:
    void on_rate_changed(const ds_time &update, const ds_asset &rate) {
        subscribers_table subscribers(_self, _self.value);
        for (auto itr = subscribers.begin(); itr != subscribers.end(); itr++) {






            eosio::transaction t;
            t.delay_sec = 0;
            t.expiration = ds_time(now() + 300);
            t.actions.emplace_back(
                    eosio::permission_level(_self, "active"_n),
                    EOSDTORCLIZE,
                    "ratechanged"_n,
                    std::make_tuple(itr->contract, update, rate)
            );

            auto id = (((uint128_t) rate.symbol.raw()) << 64) | (((uint128_t) itr->contract.value));
            auto deleted = cancel_deferred(id);
            ds_print("\r\ncancel: %, create: %", deleted, id);
            t.send(id, _self);
        }
    }


public:
    void transfer(const ds_account &from,
                  const ds_account &to,
                  ds_asset &quantity,
                  const ds_string &memo) {
        PRINT_STARTED("transfer"_n);
        if (to != _self)
            return;
        if(quantity.symbol == DAPP_SYMBOL)
        {

            return;
        }
        ds_assert(get_first_receiver() == eosdt_ctract_setting_get().nutoken_account
                && quantity.symbol == UTILITY_SYMBOL, "Wrong asset: %", quantity);
        ds_assert(to_double(quantity) >= 2.0, "Wrong quantity: %", quantity);
        ds_assert(memo == "subscribe", "Wrong memo value: %", memo);

        subscribers_table subscribers(_self, _self.value);

        auto itr = subscribers.find(from.value);
        if (itr == subscribers.end()) {
            subscribers.emplace(_self, [&](auto &row) {
                row.contract = from;
                row.quantity = quantity;
            });
        } else {
            subscribers.modify(itr, ds_account(0), [&](auto &row) {
                row.quantity += quantity;
            });
        }

        PRINT_FINISHED("transfer"_n);
    }



    void unsubscribe(const ds_account &contract) {
        PRINT_STARTED("unsubscribe"_n);
        require_auth(contract);
        subscribers_table subscribers(_self, _self.value);

        auto itr = subscribers.find(contract.value);
        ds_assert(itr != subscribers.end(), "Wrong contract: %", contract);

        auto time = time_get();
        if (itr->withdrawal_date == ds_time(0)) {
            subscribers.modify(itr, ds_account(0), [&](auto &row) {
                row.withdrawal_date = time + 86400;
            });
            eosio::transaction t;
            t.delay_sec = 86400;
            t.actions.emplace_back(
                    eosio::permission_level(_self, "active"_n),
                    _self,
                    "unsubscribe"_n,
                    std::make_tuple(contract));
            auto id = contract.value;
            auto deleted = cancel_deferred(id);
            t.send(id, _self);
        } else {
            ds_assert(itr->withdrawal_date <= time, "Unsubscribe % more that current %.", itr->withdrawal_date, time);
            if (itr->quantity.amount > 0) {
                eosio::action(
                        eosio::permission_level{_self, "active"_n},
                        eosdt_ctract_setting_get().nutoken_account,
                        "transfer"_n,
                        std::make_tuple(_self, contract, itr->quantity, ds_string("unsubscribe"))
                ).send();
            }
            subscribers.erase(itr);
        }
        PRINT_STARTED("unsubscribe"_n);
    }

    void ratechanged(const ds_account &contract, const ds_time &update, const ds_asset &rate) {
        ds_print("\r\ncontract: %, update: %, rate: %.",
                 contract, update, rate);
        require_recipient(contract);
        subscribers_table subscribers(_self, _self.value);
        auto itr = subscribers.find(contract.value);
        auto time = time_get();
        if (itr != subscribers.end()) {
            subscribers.modify(itr, ds_account(0), [&](auto &row) {
                row.callcount++;
                row.lastcall = time;
            });
        }
    }
};
