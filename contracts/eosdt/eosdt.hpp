#pragma once

#include "tables.hpp"

namespace eosdt {

    class contract : public eosio::contract {

    public:
        using eosio::contract::contract;

    protected:
        auto orasetting_get() {
            auto ora = (EOSDTCURRENT == EOSDTORCLIZE) ? _self : ctrsetting_get().oraclize_account;
            orasettings_table orasettings(ora, ora.value);
            auto itr = orasettings.find(0);
            ds_assert(itr != orasettings.end(), "% %.", NEED_TO_SET_UP, "orasettings"_n);
            return *itr;
        }

        ctrsetting ctrsetting_get(const ds_account &position_account) {
            ctrsettings_table ctrsettings(position_account, position_account.value);
            auto itr = ctrsettings.begin();
            ds_assert(itr != ctrsettings.end(), "% %.", NEED_TO_SET_UP, "ctrsettings"_n);
            return *itr;
        }

        ctrsetting ctrsetting_get() {
            auto ctr = (EOSDTCURRENT == EOSDTCNTRACT || EOSDTCURRENT == EOSDTPOSCONT) ? _self : EOSDTCNTRACT;
            if (EOSDTCURRENT == EOSDTLIQDATR) {
                ctr = liqsetting_get().position_account;
            } else if (EOSDTCURRENT == EOSDTLOCKUPP) {
                ctr = locsetting_get().eosdtcntract_account;
            }
            return ctrsetting_get(ctr);
        }

        liqsetting liqsetting_get() {
            auto liq = (EOSDTCURRENT == EOSDTLIQDATR) ? _self : ctrsetting_get().liquidator_account;
            liqsettings_table liqsettings(liq, liq.value);
            auto itr = liqsettings.find(0);
            ds_assert(itr != liqsettings.end(), "% %.", NEED_TO_SET_UP, "liqsettings"_n);
            return *itr;
        }


        auto govsetting_get() {
            auto gov = (EOSDTCURRENT == EOSDTGOVERNC) ? _self : EOSDTGOVERNC;
            govsettings_table govsettings(gov, gov.value);
            auto itr = govsettings.find(0);
            ds_assert(itr != govsettings.end(), "% %.", NEED_TO_SET_UP, "govsettings"_n);
            return *itr;
        }

        auto ressetting_get() {
            auto res = (EOSDTCURRENT == EOSDTRESERVE) ? _self : EOSDTRESERVE;
            ressettings_table ressettings(res, res.value);
            auto itr = ressettings.find(0);
            ds_assert(itr != ressettings.end(), "% %.", NEED_TO_SET_UP, "ressettings"_n);
            return *itr;
        }

        locsetting locsetting_get() {
            auto res = (EOSDTCURRENT == EOSDTLOCKUPP) ? _self : EOSDTLOCKUPP;
            locsettings_table locsettings(res, res.value);
            auto itr = locsettings.find(0);
            ds_assert(itr != locsettings.end(), "% %.", NEED_TO_SET_UP, "locsettings"_n);
            return *itr;
        }

        auto ctr_by_symbol(const ds_symbol &symbol, const ctrsetting &setting) {
            if (symbol == STABLE_SYMBOL) {
                return setting.sttoken_account;
            }
            if (symbol == UTILITY_SYMBOL) {
                return setting.nutoken_account;
            }
            if (symbol == setting.collateral_token) {
                return setting.collateral_account;
            }
            return EOSCTRACT;
        }

        auto ctr_by_symbol(const ds_symbol &symbol) {
            return ctr_by_symbol(symbol, ctrsetting_get());
        }


        auto time_get() {
            auto time = ds_time(eosio::current_time_point().sec_since_epoch());
            return time;
        }

        auto balance_get_by_ctract(const ds_account &ctract, const ds_account &account, const ds_symbol &symbol) {
            sysaccounts_table sysaccounts(ctr_by_symbol(symbol), account.value);
            auto itr = sysaccounts.find(symbol.code().raw());
            if (itr == sysaccounts.end()) {
                return ds_asset(0ll, symbol);
            }
            ds_print("\r\n% balance: %", account, itr->balance);
            return itr->balance;
        }

        auto balance_get(const ds_account &account, const ds_symbol &symbol) {
            return balance_get_by_ctract(ctr_by_symbol(symbol), account, symbol);
        }

        auto supply_get(const ds_symbol &symbol) {
            auto id = symbol.code().raw();
            sysstat_table sysstats(ctr_by_symbol(symbol), id);
            auto itr = sysstats.find(id);
            if (itr == sysstats.end()) {
                return ds_asset(0ll, symbol);
            }
            return itr->supply;
        }

        auto unissued_supply_get(const ds_symbol &symbol) {
            auto id = symbol.code().raw();
            sysstat_table sysstats(ctr_by_symbol(symbol), id);
            auto itr = sysstats.find(id);
            if (itr == sysstats.end()) {
                return ds_asset(0ll, symbol);
            }
            return itr->max_supply - itr->supply;
        }

        auto rexpool_available() {
            rexpool_table rexpool(EOSSYSTEM, EOSSYSTEM.value);
            auto itr = rexpool.begin();
            return itr != rexpool.end() && itr->total_rex.amount > 0;
        }

        auto rexpool_get() {
            rexpool_table rexpool(EOSSYSTEM, EOSSYSTEM.value);
            return *rexpool.begin();
        }


        auto rex_to_eos(const ds_asset &from_rex) {
            auto rexpool = rexpool_get();
            const int64_t S0 = rexpool.total_lendable.amount;
            const int64_t R0 = rexpool.total_rex.amount;
            const int64_t p = (uint128_t(from_rex.amount) * S0) / R0;
            auto res = ds_asset(p, EOS_SYMBOL);
            ds_print("\r\nrex_to_eos(%) = from_rex(%) * S0(%) / R0(%)",
                     res, from_rex, rexpool.total_lendable, rexpool.total_rex);
            return res;
        }

        auto eos_to_rex(const ds_asset &from_eos) {
            auto rexpool = rexpool_get();
            const int64_t S0 = rexpool.total_lendable.amount;
            const int64_t R0 = rexpool.total_rex.amount;
            uint128_t fr = uint128_t(from_eos.amount) * R0;
            int64_t p = fr / S0;
            if ((uint128_t(p) * S0) < fr) {
                p++;
            }
            ds_print("\r\neos_to_rex(%) = from_eos(%) * R0(%) / S0(%)",
                     p, from_eos, rexpool.total_rex, rexpool.total_lendable);
            return ds_asset(p, REX_SYMBOL);
        }


        auto rexbalance_get(const ds_account &account) {
            rexbalance_table rexbalances(EOSSYSTEM, EOSSYSTEM.value);
            auto itr = rexbalances.find(account.value);
            if (itr == rexbalances.end()) {
                return ds_asset(0ll, EOS_SYMBOL);
            }
            return rex_to_eos(itr->rex_balance);
        }

        auto rexbalance_pending_get(const ds_account &account) {
            rexqueue_table rexqueue(EOSSYSTEM, EOSSYSTEM.value);
            auto itr = rexqueue.find(account.value);
            if (itr == rexqueue.end()) {
                return ds_asset(0ll, EOS_SYMBOL);
            }
            return rex_to_eos(itr->rex_requested);
        }

        auto orarate_get(const ds_symbol &token_symbol, const ds_symbol &base) {
            auto ora = (EOSDTCURRENT == EOSDTORCLIZE) ? _self : ctrsetting_get().oraclize_account;
            orarates_table orarates(ora, ora.value);
            auto index = orarates.template get_index<"ratebase"_n>();
            auto itr = index.find(compress_key(token_symbol.code().raw(), base.code().raw()));
            ds_assert(itr != index.end(), "did not find token(%/%) for orarate.", token_symbol, base);
            ds_print("\r\nrate:%", itr->rate);
            return itr->rate;
        }

        auto orarate_get(const ds_symbol &token_symbol) {
            return orarate_get(token_symbol, EOS_SYMBOL);
        }

        auto auction_price_get(const ds_symbol &token_symbol, const ds_symbol &base) {
            auto liqsettings = liqsetting_get();
            if (liqsettings.global_unlock && token_symbol == USD_SYMBOL) {
                return liqsettings.auction_price;
            }
            return orarate_get(token_symbol, base);
        }

        auto auction_price_get(const ds_symbol &token_symbol) {
            return auction_price_get(token_symbol, EOS_SYMBOL);
        }

        auto liquidation_price_get(const ds_symbol &token_symbol, const ds_symbol &base) {
            auto ctrsettings = ctrsetting_get();
            if (ctrsettings.global_lock && token_symbol == USD_SYMBOL) {
                return ctrsettings.liquidation_price;
            }
            return orarate_get(token_symbol, base);
        }

        auto liquidation_price_get(const ds_symbol &token_symbol) {
            return liquidation_price_get(token_symbol, EOS_SYMBOL);
        }

        void issue(const ds_account &to, const ds_asset &quantity, const ds_string &memo) {
            auto ctract = ctr_by_symbol(quantity.symbol);
            auto balance = balance_get(to, quantity.symbol);
            ds_print("\r\nissue: {to: %, quantity: % ,by: %, memo: '%', before: %, after: %}",
                     to, quantity, ctract, memo, balance, balance + quantity);
            if (quantity.amount <= 0) {
                return;
            }
            auto issue_memo = memo;
            if (quantity.symbol == EOSDT_SYMBOL) {
                //real memo was lost.
                issue_memo = "via:" + _self.to_string();
            }
            eosio::action(
                    eosio::permission_level{_self, "active"_n},
                    ctract,
                    "issue"_n,
                    std::make_tuple(_self, quantity, issue_memo)
            ).send();
            if (to != _self) {
                trans(to, quantity, memo);
            }
        }


        void trans_by_ctract(const ds_account &ctract, const ds_account &to, const ds_asset &quantity,
                             const ds_string &memo) {
            PRINT_STARTED("trans")
            auto balance = balance_get_by_ctract(ctract, _self, quantity.symbol);
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
            PRINT_FINISHED("trans")
        }

        void trans(const ds_account &to, const ds_asset &quantity, const ds_string &memo) {
            trans_by_ctract(ctr_by_symbol(quantity.symbol), to, quantity, memo);
        }

        void receive(const ds_account &from, const ds_asset &quantity, const ds_string &memo) {
            auto ctract = ctr_by_symbol(quantity.symbol);
            auto balance = balance_get(_self, quantity.symbol);
            ds_print("\r\nreceive: {from: %, to: %, quantity: % ,by: %, memo: '%', before: %, after: %}",
                     from, _self, quantity, ctract, memo, balance, balance + quantity);
            if (quantity.amount <= 0) {
                return;
            }
            if (quantity.symbol == NUT_SYMBOL) {
                eosio::action(
                        eosio::permission_level{_self, "active"_n},
                        ctract,
                        "receive"_n,
                        std::make_tuple(from, _self, _self, quantity, memo)
                ).send();
            } else {
                eosio::action(
                        eosio::permission_level{_self, "active"_n},
                        ctract,
                        "receive"_n,
                        std::make_tuple(from, _self, quantity, memo)
                ).send();
            }
        }

        auto act_by_symbol(const ds_symbol &symbol) {
            if (symbol == EOSDT_SYMBOL) {
                return "retire"_n;
            }

            return "burn"_n;
        }

        void burn(const ds_account &from, const ds_asset &quantity, const ds_string &memo) {
            auto ctract = ctr_by_symbol(quantity.symbol);
            auto action = act_by_symbol(quantity.symbol);
            auto balance = balance_get(from, quantity.symbol);
            ds_print("\r\nburn: {from: %, quantity: % ,by: %, memo: '%', before: %,  after: %}",
                     from, quantity, ctract, memo, balance, balance - quantity);
            if (quantity.amount <= 0) {
                return;
            }
            auto burn_memo = memo;
            auto perm = eosio::permission_level{from, "active"_n};
            if (quantity.symbol == EOSDT_SYMBOL) {
                auto issuer = _self;
                if (issuer == EOSDTLIQDATR) {
                    issuer = EOSDTCNTRACT;
                }
                burn_memo = "via:" + issuer.to_string();
                perm = eosio::permission_level{issuer, "active"_n};
            }
            eosio::action(
                    perm,
                    ctract,
                    action,
                    std::make_tuple(from, quantity, burn_memo)
            ).send();
        }

        void retire_stable(const ds_account &issuer, const ds_account &from, const ds_asset &quantity,
                           const ds_string &memo) {
            auto ctract = ctr_by_symbol(quantity.symbol);
            auto balance = balance_get(from, quantity.symbol);
            ds_print("\r\nretirefrom: {issuer: %, from: %, quantity: %, by: %, memo: '%', before: %, after: %}",
                     issuer, from, quantity, ctract, memo, balance, balance - quantity);
            if (quantity.amount <= 0) {
                return;
            }

            std::vector<ds_account> names = { issuer };
            if (issuer != from) {
                names.push_back(from);
                std::sort(names.begin(), names.end());
            }
            std::vector<eosio::permission_level> perms;
            for (auto n : names) {
                perms.push_back(eosio::permission_level{n, "active"_n});
            }

            eosio::action(
                    perms,
                    ctract,
                    "retirefrom"_n,
                    std::make_tuple(issuer, from, quantity, memo)
            ).send();
        }
    };

}
