#include "settings.hpp"

using eosio::cancel_deferred;

class positions : public settings{


private:
    typedef eosio::multi_index<"respositions"_n, respositions,
            eosio::indexed_by<"depositor"_n, eosio::const_mem_fun < respositions, uint64_t, &respositions::get_depositor>>,
            eosio::indexed_by<"balance"_n, eosio::const_mem_fun < respositions, uint64_t, &respositions::get_balance>>> respositions_table;

protected:

    auto positions_get(const ds_ulong &position_id) {
        respositions_table respositions(_self, _self.value);
        auto itr = respositions.find(position_id);
        ds_assert(itr != respositions.end(), "%(%) %.", POSITIONS, position_id, DOES_NOT_EXIST);
        return *itr;
    }

    auto ctract_by_symbol(const ds_symbol &symbol) {






        return EOSCTRACT;
    }

    typedef eosio::multi_index<"accounts"_n, account> accounts;
    auto factual_balance(const ds_account &account, const ds_symbol &symbol) {
        accounts accounts_table(ctract_by_symbol(symbol), account.value);
        auto itr = accounts_table.find(symbol.code().raw());
        if (itr == accounts_table.end()) {
            return ds_asset(0ll, symbol);
        }
        return itr->balance;
    }

    void trans(const ds_account &to, const ds_asset &quantity, const ds_string &memo) {
        auto ctract = ctract_by_symbol(quantity.symbol);
        auto balance = factual_balance(_self, quantity.symbol);
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

    void remove_redundant_depositors()
    {
        auto settings = settings_get();
        respositions_table respositions(_self, _self.value);
        auto size = std::distance(respositions.begin(),respositions.end());
        ds_print("maxdepositors: %, current size: %",settings.maxdepositors,size);
        auto index = respositions.get_index<"balance"_n>();
        auto itr = index.begin();
        auto count = settings.maxdepositors;
        while (itr != index.end() && count < size)
        {
            count++;
            eosio::action(
                    eosio::permission_level{_self, "active"_n},
                    EOSCTRACT,
                    "transfer"_n,
                    std::make_tuple(_self, itr->depositor, itr->eos_balance, ds_string("withdraw"))
            ).send();
            itr = index.erase(itr);
        }
    }


public:

    positions(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) : settings(receiver, code, ds) {
    }
    void transfer(const ds_account &from,
                  const ds_account &to,
                  ds_asset &quantity,
                  const ds_string &memo) {
        PRINT_STARTED("transfer"_n);
        if (to != _self)
            return;
        auto ctract_for_symbol = ctract_by_symbol(quantity.symbol);
        ds_assert(ctract_for_symbol == _first_receiver, "Wrong contract % for asset: % expected: %.", _first_receiver, quantity,
                  ctract_for_symbol);
        ds_assert(quantity.symbol == EOS_SYMBOL, "Wrong token %, eos was expected.", quantity);
        auto settings = settings_get();
        ds_assert(quantity >= settings.mindeposit, "Amount % is insufficient, expected % or greater.", quantity, settings.mindeposit);

        respositions_table respositions(_self, _self.value);
        auto index = respositions.get_index<"depositor"_n>();
        auto itr = index.find(from.value);

        if(itr == index.end()){
            respositions.emplace(_self, [&](auto &row) {
                row.position_id = respositions.available_primary_key();
                row.depositor = from;
                row.eos_balance = quantity;
                row.withdrawal_date = ds_time(0);
            });

        }else{
            index.modify(itr, ds_account(0), [&](auto &row) {
                row.eos_balance += quantity;
            });

        }
        remove_redundant_depositors();
        PRINT_FINISHED("transfer"_n);
    }

    void assert_if_settlement(bool value) {
        auto settings = settings_get();

        eosio::multi_index<"ctrsettings"_n, ctrsetting> ctrsettings_tbl("eosdtcntract"_n, ("eosdtcntract"_n).value);
        auto ctrsettings = ctrsettings_tbl.require_find(0);
        auto liquidator_account = ctrsettings->liquidator_account;

        eosio::multi_index<"parameters"_n, liqparameter> liqparameters_tbl(liquidator_account, liquidator_account.value);
        auto liqparameters = liqparameters_tbl.require_find(0);
        eosio::multi_index<"liqsettings"_n, liqsetting> liqsettings_tbl(liquidator_account, liquidator_account.value);
        auto liqsettings = liqsettings_tbl.require_find(0);
        auto need_send = false;
        if(ctrsettings->global_lock) {
            ds_assert(ctrsettings->liquidation_price.amount > 0, "Liquidation price (%) should be more than zero.",
                      ctrsettings->liquidation_price);
            auto to_send = ds_asset(
                    pow(10.0, EOS_SYMBOL_DECIMAL) *
                    (to_double(liqparameters->bad_debt) / to_double(ctrsettings->liquidation_price)),
                    EOS_SYMBOL) - liqparameters->eos_balance;
            ds_print("\r\nto_send(%) = bad_debt(%)/liquidation_price(%) - eos_balance(%).",
                     to_send, liqparameters->bad_debt, ctrsettings->liquidation_price, liqparameters->eos_balance);
            need_send = liqparameters->bad_debt.amount > 0ll && to_send.amount > 0ll;
        }
        ds_print("\r\nsettlement(%) global_lock(%) global_unlock(%) need_send(%).",
                 value, ctrsettings->global_lock, liqsettings->global_unlock, need_send);
        if (value) {
            ds_assert(ctrsettings->global_lock && liqsettings->global_unlock && need_send,
                      "global settlement isn't engaged");
        } else {
            ds_assert(!ctrsettings->global_lock || (liqsettings->global_unlock && !need_send),
                      "global settlement is engaged");
        }
    }

    void withdraw(const ds_account &depositor)
    {
        PRINT_STARTED("withdraw"_n);
        if (!has_auth(_self))
        {
            require_auth(depositor);
        }
        respositions_table respositions(_self, _self.value);
        auto index = respositions.get_index<"depositor"_n>();
        auto itr = index.find(depositor.value);
        ds_assert(itr != index.end(),"Position does not exists for depositor: %.",depositor);
        auto time = time_get();
        if(itr->withdrawal_date == ds_time(0))
        {
            auto settings = settings_get();
            index.modify(itr, ds_account(0), [&](auto &row) {
                row.withdrawal_date = time+settings.unstake_period;
            });
            eosio::transaction t;
            t.delay_sec = settings.unstake_period;
            t.actions.emplace_back(
                    eosio::permission_level(_self, "active"_n),
                    _self,
                    "withdraw"_n,
                    std::make_tuple(depositor));
            auto id = depositor.value;
            auto deleted = cancel_deferred(id);
            t.send(id, _self);
        } else {
            ds_assert(itr->withdrawal_date <= time,"Withdrawal_date % more that current %.",itr->withdrawal_date , time);
            assert_if_settlement(false);
            eosio::action(
                    eosio::permission_level{_self, "active"_n},
                    EOSCTRACT,
                    "transfer"_n,
                    std::make_tuple(_self, depositor, itr->eos_balance, ds_string("withdrawal"))
            ).send();

            index.erase(itr);
        }
        PRINT_FINISHED("withdraw"_n);
    }

    void sendeos() {
        PRINT_STARTED("sendeos"_n);
        assert_if_settlement(true);
        eosio::multi_index<"ctrsettings"_n, ctrsetting> ctrsettings_tbl("eosdtcntract"_n, ("eosdtcntract"_n).value);
        auto ctrsettings = ctrsettings_tbl.require_find(0);
        auto liquidator_account = ctrsettings->liquidator_account;
        eosio::multi_index<"parameters"_n, liqparameter> liqparameters_tbl(liquidator_account, liquidator_account.value);
        auto liqparameters = liqparameters_tbl.require_find(0);
        auto to_send = ds_asset(liqparameters->bad_debt.amount/ctrsettings->liquidation_price.amount/
                                        (ds_ulong)pow(10, (ds_uint)(STABLE_SYMBOL_DECIMAL - USD_SYMBOL_DECIMAL - EOS_SYMBOL_DECIMAL)),EOS_SYMBOL) - liqparameters->eos_balance;
        ds_print("\r\nto_send(%) = bad_debt(%)/liquidation_price(%) - eos_balance(%).",
                 to_send,
                 liqparameters->bad_debt, ctrsettings->liquidation_price, liqparameters->eos_balance);
        auto balance = factual_balance(_self, EOS_SYMBOL);
        if (to_send.amount > balance.amount) {
            to_send.amount = balance.amount;
        }
        ds_assert(to_send.amount > 0 && balance.amount > 0, "wrong eos amount");
        auto collected = ds_asset(0,EOS_SYMBOL);
        respositions_table respositions(_self, _self.value);
        auto index = respositions.get_index<"depositor"_n>();
        auto itr = index.begin();
        while (itr != index.end()) {
            index.modify(itr, ds_account(0), [&](auto &row) {
                auto new_balance = ds_asset(row.eos_balance.amount * (balance.amount - to_send.amount) / balance.amount, EOS_SYMBOL);
                ds_print("\r\ncollected(%) = collected(%) + %.eos_balance(%) - new_balance(%).",
                         collected + row.eos_balance - new_balance, collected, row.depositor, row.eos_balance, new_balance);
                collected = collected + row.eos_balance - new_balance;
                row.eos_balance = new_balance;
            });
            itr++;
        }

        trans(liquidator_account, collected, "reservefund");

        PRINT_FINISHED("sendeos"_n);
    }

};
