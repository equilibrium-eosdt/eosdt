#include "eosdtgovernc.hpp"

namespace eosdt {

    void eosdtgovernc::bpregister(const ds_account &bp_name, const ds_asset &reward_amount) {
        PRINT_STARTED("bpregister"_n)
        auto payer = _self;
        if (!has_auth(_self)) {
            require_auth(bp_name);
            payer = bp_name;
        }
        auto min_reward = govparam_get().min_reward;
        if(isbpintop(bp_name)){
            min_reward = op_mul_ceil(min_reward,1.0L/(1.0L-10000.0L/votepay_factor_get()));
        }
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
        auto min_reward = govparam_get().min_reward;
        if(isbpintop(bp_name)){
            min_reward = op_mul_ceil(min_reward,1.0l/(1.0l-10000.0l/votepay_factor_get()));
        }
        ds_assert(reward_amount >= min_reward, "Wrong reward_amount: % expected more or equal min_reward: %.",
                  reward_amount, min_reward);
        {
            govbpparams_table govbpparams(_self, _self.value);
            auto itr = govbpparams.find(bp_name.value);
            ds_assert(itr != govbpparams.end(), "bp_name: % does not exists.", bp_name);
            ds_assert((payer != bp_name) || (itr->reward_amount < reward_amount),
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
        auto min_reward = govparam_get().min_reward;
        auto top21_reward = op_mul_ceil(min_reward,1.0l/(1.0l-10000.0l/votepay_factor_get()));
        govbpparams_table govbpparams(_self, _self.value);
        for (auto itr = govbpparams.begin(); itr != govbpparams.end(); itr++) {

            if (!itr->is_active) {
                ds_print("\r\nbp_name: %, enabled: %, is_active: %", itr->bp_name, itr->enabled, itr->is_active);
                continue;
            }
            auto reward = isbpintop(itr->bp_name) ? top21_reward : min_reward;
            govbpparams.modify(itr, ds_account(0), [&](auto &row) {
                auto current = total_payable;
                auto period = (time - itr->active_since).to_seconds();
                if(period > 0)
                {
                    if (reward < row.reward_amount) {
                        reward = row.reward_amount;
                    }
                    reward = op_div_floor(reward * period, 24 * 3600);
                    if(row.balance > reward)
                    {
                        total_payable += reward;
                        row.balance -= reward;
                    } else {
                        total_payable+=row.balance;
                        row.balance.amount=0;
                    }
                }
                if(row.balance.amount <= 0)
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
            auto position_account = govsettings.position_account;
            auto for_main = op_mul_ceil(total_payable, reward_weight);
            trans(position_account, for_main, "profit");
            trans(liquidator_account, total_payable - for_main, "profit");
            ds_print("\r\nreward_weight: %, total_payable: %, position_account: %, liquidator_account: %.",
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
        calcminrewrd();
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
        ds_assert(itr != govparams.end(), "% govparams.", NEED_TO_SET_UP);
        return itr->NUT_voting_balance;
    }

    void eosdtgovernc::calcnutbal(){
        PRINT_STARTED("calcnutbal"_n)
        require_auth(_self);
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
                row.min_reward = ds_asset(0, EOS_SYMBOL);
            });
        } else {
            govparams.modify(itr, ds_account(0), [&](auto &row) {
                row.NUT_voting_balance = nut_voting_balance;
            });
        }
        auto total = balance_get(_self,NUT_SYMBOL);
        ds_print("total:%, voted:%", total, nut_voting_balance);
        PRINT_FINISHED("calcnutbal"_n)
    }

    void eosdtgovernc::calcminrewrd(){
        PRINT_STARTED("calcminrewrd"_n)
        auto stake_reward = govsetting_get().stake_reward;
        sysglobal_table sysglobal("eosio"_n, ("eosio"_n).value);
        auto itr_global = sysglobal.begin();
        ds_assert(itr_global != sysglobal.end(), "eosio global is empty.");
        sysglobal4_table sysglobal4("eosio"_n, ("eosio"_n).value);
        auto itr_global4 = sysglobal4.begin();
        ds_assert(itr_global4 != sysglobal4.end(), "eosio global4 is empty.");
        sysvoters_table sysvoters("eosio"_n, ("eosio"_n).value);
        auto itr_voter = sysvoters.find((EOSDTBPPROXY).value);
        ds_assert(itr_voter != sysvoters.end(), "eosio voters does not contain %.", EOSDTBPPROXY);
        sysstat_table sysstat( EOSCTRACT,(EOS_SYMBOL).code().raw());
        auto itr_stat = sysstat.find((EOS_SYMBOL).code().raw());
        ds_assert( itr_stat != sysstat.end(), "eosio.token stat does not contain %.", EOS_SYMBOL);

        auto tokens_per_day =  op_mul_ceil(itr_stat->supply, std::exp(itr_global4->continuous_rate/365.25) - 1.0L);
        ds_print("\r\ntokens_per_day(%) = supply(%)*(exp(continuous_rate(%)/365.25)-1)",
                 tokens_per_day, itr_stat->supply, itr_global4->continuous_rate);
        auto vote_reward = op_mul_ceil(tokens_per_day, 1.0L-10000.0L/itr_global4->votepay_factor);
        ds_print("\r\nvote_reward(%) = tokens_per_day(%) * (1-10000/votepay_factor(%))",
                 vote_reward, tokens_per_day, itr_global4->votepay_factor);
        if(itr_global->total_producer_vote_weight == 0.0){
            ds_print("\r\ntotal_producer_vote_weight = %",itr_global->total_producer_vote_weight);
            PRINT_FINISHED("calcminrewrd"_n)
            return;
        }
        auto min_reward = op_mul_div(vote_reward,stake_reward*itr_voter->last_vote_weight, itr_global->total_producer_vote_weight);
        ds_print("\r\nmin_reward(%)=last_vote_weight(%)*vote_reward(%)*stake_reward(%)/total_producer_vote_weight(%)",
                 min_reward,itr_voter->last_vote_weight,vote_reward,stake_reward,itr_global->total_producer_vote_weight);

        govparams_table govparams(_self, _self.value);
        auto itr = govparams.begin();
        if (itr == govparams.end()) {
            govparams.emplace(_self, [&](auto &row) {
                row.param_id = 0;
                row.NUT_voting_balance = ds_asset(0, NUT_SYMBOL);
                row.min_reward = min_reward;
            });
        } else {
            govparams.modify(itr, ds_account(0), [&](auto &row) {
                row.min_reward = min_reward;
            });
        }
        PRINT_FINISHED("calcminrewrd"_n)
    }

    void eosdtgovernc::paraminit() {
        PRINT_STARTED("paraminit"_n)
        require_auth(_self);

        auto nut_voting_balance = ds_asset(0, NUT_SYMBOL);
        struct govparam_del {
            ds_ulong param_id;
            ds_asset NUT_voting_balance;
            ds_ulong primary_key() const { return param_id; }
        };
        eosio::multi_index<"govparams"_n, govparam_del> govparams_del(_self, _self.value);
        for (auto itr = govparams_del.begin(); itr != govparams_del.end(); itr = govparams_del.erase(itr)) {
            nut_voting_balance = itr->NUT_voting_balance;
        }

        govparams_table govparams(_self, _self.value);
        govparams.emplace(_self, [&](auto &row) {
            row.param_id = 0;
            row.NUT_voting_balance = nut_voting_balance;
            row.min_reward = ds_asset(0, EOS_SYMBOL);
        });
        calcminrewrd();
        PRINT_FINISHED("paraminit"_n)
    }

    bool eosdtgovernc::isbpintop(const ds_account &bp){
        PRINT_STARTED("isbpintop"_n)
        sysproducers_table sysproducers("eosio"_n, ("eosio"_n).value);
        auto idx = sysproducers.template get_index<"prototalvote"_n>();
        for (auto [itr,cnt] = std::tuple{idx.begin(),0}; itr != idx.end() && cnt<21; itr++,cnt++) {
            if(itr->owner == bp){
                PRINT_FINISHED("isbpintop21"_n)
                return true;
            }
        }
        PRINT_FINISHED("isbpintop"_n)
        return false;
    };

    govparam eosdtgovernc::govparam_get()
    {
        govparams_table govparams(_self, _self.value);
        auto itr = govparams.begin();
        ds_assert(itr != govparams.end(), "% % on %.", NEED_TO_SET_UP, "paraminit"_n, EOSDTGOVERNC);
        return *itr;
    }

    double eosdtgovernc::votepay_factor_get()
    {
        sysglobal4_table sysglobal4("eosio"_n, ("eosio"_n).value);
        auto itr_global4 = sysglobal4.begin();
        ds_assert(itr_global4 != sysglobal4.end(), "eosio global4 is empty.");
        return itr_global4->votepay_factor;
    }

}
