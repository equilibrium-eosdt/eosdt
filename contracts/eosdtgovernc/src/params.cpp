#include "eosdtgovernc.hpp"

namespace eosdt {

    void eosdtgovernc::bpregister(const ds_account &bp_name, const ds_asset &reward_amount) {
        PRINT_STARTED("bpregister"_n)
        auto payer = _self;
        if (!has_auth(_self)) {
            require_auth(bp_name);
            payer = bp_name;
        }
        auto min_reward = govsetting_get().min_reward;
        ds_assert(reward_amount >= min_reward, "Wrong reward_amount: % expected more or equal min_reward: %.",
                  reward_amount, min_reward);
        govbpparams_table govbpparams(_self, _self.value);
        auto itr = govbpparams.find(bp_name.value);
        ds_assert(itr == govbpparams.end(), "bp_name: % already exists", bp_name);
        govbpparams.emplace(payer, [&](auto &row) {
            row.bp_name = bp_name;
            row.reward_amount = reward_amount;
            row.balance = ds_asset(0, EOS_SYMBOL);
            row.enabled = false;
            row.is_active = false;
        });
        reinit();
        PRINT_FINISHED("bpregister"_n)
    }

    void eosdtgovernc::bpsetparams(const ds_account &bp_name, const ds_asset &reward_amount) {
        PRINT_STARTED("bpsetparams"_n)
        auto payer = ds_account(0);
        if (!has_auth(_self)) {
            require_auth(bp_name);
            payer = bp_name;
        }
        auto min_reward = govsetting_get().min_reward;
        ds_assert(reward_amount >= min_reward, "Wrong reward_amount: % expected more or equal min_reward: %.",
                  reward_amount, min_reward);
        {
            govbpparams_table govbpparams(_self, _self.value);
            auto itr = govbpparams.find(bp_name.value);
            ds_assert(itr != govbpparams.end(), "bp_name: % does not exists.", bp_name);
            ds_assert((payer == bp_name) && (itr->reward_amount < reward_amount),
                      "rewards % may not be decreased to %.",
                      itr->reward_amount, reward_amount);
        }

        reinit();

        {
            govbpparams_table govbpparams(_self, _self.value);
            auto itr = govbpparams.find(bp_name.value);
            govbpparams.modify(itr, payer, [&](auto &row) {
                row.reward_amount = reward_amount;
            });
        }

        PRINT_FINISHED("bpsetparams"_n)
    }

    void eosdtgovernc::bpunregister(const ds_account &bp_name) {
        PRINT_STARTED("bpunregister"_n)
        if (!has_auth(_self)) {
            require_auth(bp_name);
        }
        {
            govbpparams_table govbpparams(_self, _self.value);
            auto itr = govbpparams.find(bp_name.value);
            ds_assert(itr != govbpparams.end(), "bp_name: % does not exists.", bp_name);
            govbpparams.modify(itr, ds_account(0), [&](auto &row) {
                row.enabled = false;
            });
        }

        reinit();

        {
            govbpparams_table govbpparams(_self, _self.value);
            auto itr = govbpparams.find(bp_name.value);
            govbpparams.modify(itr, ds_account(0), [&](auto &row) {
                trans(bp_name, row.balance, "bpunregister");
                row.balance = ds_asset(0, EOS_SYMBOL);
                row.is_active = false;
            });
        }
        PRINT_FINISHED("bpunregister"_n)
    }

    void eosdtgovernc::bpdeposit(const ds_string &bp_name_str, const ds_asset &quantity) {
        PRINT_STARTED("bpdeposit"_n)
        ds_account bp_name(bp_name_str);
        govbpparams_table govbpparams(_self, _self.value);
        auto itr = govbpparams.find(bp_name.value);
        ds_assert(itr != govbpparams.end(), "bp_name: % does not exists.", bp_name);
        ds_assert(quantity+itr->balance >= itr->reward_amount, "wrong amount % + (balance)% expected >= %",
                quantity,itr->balance,itr->reward_amount);
        govbpparams.modify(itr, ds_account(0), [&](auto &row) {
            row.balance += quantity;
            row.enabled = true;
            ds_print("\r\nbp_name: %, enabled: %, balance: %", bp_name, row.enabled, row.balance);
        });

        PRINT_FINISHED("bpdeposit"_n)
    }

    void eosdtgovernc::dobilling() {
        PRINT_STARTED("dobilling"_n)
        auto total_payable = ds_asset(0, EOS_SYMBOL);
        auto time = time_get();
        auto govsettings = govsetting_get();
        govbpparams_table govbpparams(_self, _self.value);
        for (auto itr = govbpparams.begin(); itr != govbpparams.end(); itr++) {

            if (!itr->is_active) {
                ds_print("\r\nbp_name: %, enabled: %, is_active: %", itr->bp_name, itr->enabled, itr->is_active);
                continue;
            }

            govbpparams.modify(itr, ds_account(0), [&](auto &row) {
                auto current = total_payable;
                auto period = (time - itr->active_since).to_seconds();
                if(period > 0)
                {
                    auto reward = op_div_floor(row.reward_amount * period, 24 * 3600);
                    if(row.balance > reward)
                    {
                        total_payable += reward;
                        row.balance -= reward;
                    } else {
                        total_payable+=row.balance;
                        row.balance.amount=0;
                    }
                }
                if(row.balance.amount <= 0 || row.reward_amount < govsettings.min_reward)
                {
                    row.enabled = false;
                }
                ds_print("\r\nbp_name: %, enabled: %, period: %, reward: %, balance: %, reward_by_period: %",
                        row.bp_name, row.enabled,  period, row.reward_amount, row.balance,total_payable-current);
            });
        };
        if (total_payable.amount > 0) {
            auto reward_weight = govsetting_get().reward_weight;
            auto liquidator_account = ctrsetting_get().liquidator_account;
            auto eosdtcntract_account = govsettings.eosdtcntract_account;
            auto for_main = op_mul_ceil(total_payable, reward_weight);
            trans(eosdtcntract_account, for_main, "profit");
            trans(liquidator_account, total_payable - for_main, "profit");
            ds_print("\r\nreward_weight: %, total_payable: %, eosdtcntract_account: %, liquidator_account: %.",
                     reward_weight, total_payable, for_main, total_payable - for_main);
        }
        PRINT_FINISHED("dobilling"_n)
    }

    void eosdtgovernc::updatebilling(const std::set <ds_account> &top_producers) {
        PRINT_STARTED("updatebillin"_n)
        auto time = time_get();
        govbpparams_table govbpparams(_self, _self.value);
        for (auto itr = govbpparams.begin(); itr != govbpparams.end(); itr++) {
            auto in_list = std::find(top_producers.begin(), top_producers.end(), itr->bp_name) != top_producers.end();
            //ds_assert(!in_list || (in_list && itr->enabled), "disabled bproducer: %.", itr->bp_name);
            if(in_list && !itr->enabled)
            {
                ds_print("\r\ndisabled bproducer: %.", itr->bp_name);
                continue;
            }
            govbpparams.modify(itr, ds_account(0), [&](auto &row) {
                if (in_list) {
                    row.active_since = time;
                    if (!row.is_active) {
                        row.is_active = true;
                    }
                } else {
                    row.is_active = false;
                }

                ds_print("\r\nbp_name: %, enabled: %, is_active: %, active_since: %, in_list: %", row.bp_name, row.enabled, row.is_active, row.active_since, in_list);
            });
        };
        PRINT_FINISHED("updatebillin"_n)
    }
    void eosdtgovernc::reinit() {
        PRINT_STARTED("reinit"_n)
        applybpproxy();
        PRINT_FINISHED("reinit"_n)
    }

    void eosdtgovernc::updvotingbal(const ds_asset &quantity)
    {
        PRINT_STARTED("updvotingbal"_n)
        govparams_table govparams(_self, _self.value);
        auto itr = govparams.find(0);
        if (itr == govparams.end()) {
            govparams.emplace(_self, [&](auto &row) {
                row.param_id = 0;
                row.NUT_voting_balance = quantity;
            });
        } else {
            //govparams.erase(itr);
            govparams.modify(itr, ds_account(0), [&](auto &row) {
                row.NUT_voting_balance += quantity;
            });
        }
        PRINT_FINISHED("updvotingbal"_n)
    }

    ds_asset eosdtgovernc::getvotingbal()
    {
        govparams_table govparams(_self, _self.value);
        auto itr = govparams.find(0);
        return itr == govparams.end()?ds_asset(0,NUT_SYMBOL):itr->NUT_voting_balance;
    }

    void eosdtgovernc::calcnutbal(){
        PRINT_STARTED("calcnutbal"_n)
        ds_asset quantity;
        auto nut_voting_balance = ds_asset(0, NUT_SYMBOL);
        govvoters_table govvoters(_self, _self.value);
        for (auto itr = govvoters.begin(); itr != govvoters.end(); itr++) {
            nut_voting_balance+=itr->voting_amount;
        }
        govparams_table govparams(_self, _self.value);
        auto itr = govparams.find(0);
        if (itr == govparams.end()) {
            govparams.emplace(_self, [&](auto &row) {
                row.param_id = 0;
                row.NUT_voting_balance = nut_voting_balance;
            });
        } else {
            govparams.modify(itr, ds_account(0), [&](auto &row) {
                row.NUT_voting_balance = nut_voting_balance;
            });
        }
        PRINT_FINISHED("calcnutbal"_n)
    }

}
