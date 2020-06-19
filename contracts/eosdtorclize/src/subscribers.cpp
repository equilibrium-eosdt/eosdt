#include "eosdtorclize.hpp"

namespace eosdt {
    void eosdtorclize::on_rate_changed(const ds_symbol &asset_symbol, const ds_symbol &base) {
        int threshold = 0;
        orasubscribers_table orasubscribers(_self, _self.value);
        for (auto itr = orasubscribers.begin(); itr != orasubscribers.end() && threshold < MAXSUBSCRIBERS; itr++, threshold++) {
            if (itr->asset_symbol != asset_symbol || itr->base != base) {
                continue;
            }
            eosio::action(
                    eosio::permission_level{_self, "active"_n},
                    EOSDTORCLIZE,
                    "ratechanged"_n,
                    std::make_tuple(itr->contract, asset_symbol, base)
            ).send();
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
                  && quantity.symbol == UTILITY_SYMBOL, "Wrong asset: '%'", quantity);
        ds_assert(to_double(quantity) >= 2.0, "Wrong quantity: '%'", quantity);
        ds_assert(memo.find("subscribe") == 0, "Wrong memo value: '%'", memo);

        auto symbol_pos = memo.find("symbol:");
        auto base_pos = memo.find("base:");
        ds_assert(symbol_pos != ds_string::npos, "Symbol is not found ('%').", memo);
        ds_assert(base_pos != ds_string::npos, "Base is not found ('%').", memo);
        auto symbol_comma_pos = memo.find(",", symbol_pos + 7);
        auto base_comma_pos = memo.find(",", base_pos + 5);
        ds_assert(symbol_comma_pos != ds_string::npos, "Symbol(',') is not found ('%').", memo);
        ds_assert(base_comma_pos != ds_string::npos, "Base(',') is not found ('%').", memo);
        ds_symbol asset_symbol(memo.substr(symbol_pos + 7, symbol_comma_pos - symbol_pos - 7),
                               memo[symbol_comma_pos + 1] - '0');
        ds_symbol base(memo.substr(base_pos + 5, base_comma_pos - base_pos - 5), memo[base_comma_pos + 1] - '0');

        orarate_get(_self, asset_symbol, base);
        auto comp_key = compress_key(asset_symbol.code().raw(), from.value ^ base.code().raw());
        orasubscribers_table orasubscribers(_self, _self.value);
        auto index = orasubscribers.template get_index<"assetcntract"_n>();
        auto itr = index.find(comp_key);
        if (itr == index.end()) {
            orasubscribers.emplace(_self, [&](auto &row) {
                row.contract = from;
                row.quantity = quantity;
                row.withdrawal_date = time_get() + UNSUBSCRIBE_FREEZE_PERIOD;

                row.id = orasubscribers.available_primary_key();
                row.asset_symbol = asset_symbol;
                row.base = base;
            });
        } else {
            index.modify(itr, ds_account(0), [&](auto &row) {
                row.quantity += quantity;
            });
        }

        PRINT_FINISHED("transfer"_n);
    }

    void eosdtorclize::unsubscribe(const ds_account &contract, const ds_symbol &asset_symbol, const ds_symbol &base) {
        PRINT_STARTED("unsubscribe"_n);
        require_auth(contract);
        auto comp_key = compress_key(asset_symbol.code().raw(), contract.value ^ base.code().raw());
        orasubscribers_table orasubscribers(_self, _self.value);
        auto index = orasubscribers.template get_index<"assetcntract"_n>();
        auto itr = index.find(comp_key);
        ds_assert(itr != index.end(), "Wrong unsubscribe contract: '%' , asset_symbol: '%' , base: '%'.",
                  contract, asset_symbol, base);
        auto time = time_get();

        ds_assert(itr->withdrawal_date <= time, "Unsubscribe date '%' more than current '%'.", itr->withdrawal_date, time);
        if (itr->quantity.amount > 0) {
            eosio::action(
                    eosio::permission_level{_self, "active"_n},
                    ctrsetting_get().nutoken_account,
                    "transfer"_n,
                    std::make_tuple(_self, contract, itr->quantity, ds_string("unsubscribe"))
            ).send();
        }
        index.erase(itr);

        PRINT_STARTED("unsubscribe"_n);
    }

    void eosdtorclize::ratechanged(const ds_account &contract, const ds_symbol &asset_symbol, const ds_symbol &base) {
        ds_print("\r\ncontract: %, asset_symbol: %, base: %.",
                 contract, asset_symbol, base);
        require_auth(_self);
        require_recipient(contract);
        auto comp_key = compress_key(asset_symbol.code().raw(), contract.value ^ base.code().raw());
        orasubscribers_table orasubscribers(_self, _self.value);
        auto index = orasubscribers.template get_index<"assetcntract"_n>();
        auto itr = index.find(comp_key);
        auto time = time_get();
        if (itr != index.end()) {
            index.modify(itr, ds_account(0), [&](auto &row) {
                row.callcount++;
                row.lastcall = time;
            });
        }
    }

}