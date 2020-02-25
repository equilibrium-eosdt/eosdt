#include "eosdtgovernc.hpp"

namespace eosdt {
    ds_asset eosdtgovernc::voting_amount_get(const ds_account &owner, const ds_symbol &sym) const {
        govvoters_table govvoters(_self, _self.value);
        const auto itr = govvoters.find(owner.value);
        return itr == govvoters.end() ? ds_asset(0, sym) : itr->voting_amount;
    }


    void eosdtgovernc::voting_amount_add(const ds_account &voter, const ds_asset &value, const ds_account &ram_payer) {
        govvoters_table govvoters(_self, _self.value);
        auto to = govvoters.find(voter.value);
        auto time = time_get() + govsetting_get().unstake_period;
        if (to == govvoters.end()) {
            govvoters.emplace(ram_payer, [&](auto &a) {
                a.voter = voter;
                a.voting_amount = value;
                a.withdrawal_date = time;
            });
        } else {
            deactivate_bpvote_internal(voter);
            govvoters.modify(to, ds_account(0), [&](auto &a) {
                a.voting_amount += value;
                a.withdrawal_date = time;
            });
        }
        activate_bpvote_internal(voter);
        updvotingbal(value);
    }

    bool eosdtgovernc::voting_amount_sub(const ds_account &voter, const ds_asset &value) {
        bool still_alive = true;
        govvoters_table govvoters(_self, _self.value);
        const auto &from = govvoters.get(voter.value, "no balance object found");
        ds_assert(from.voting_amount >= value, "overdrawn voting balance");
        deactivate_bpvote_internal(voter);
        if (from.voting_amount == value) {
            govvoters.erase(from);
            still_alive = false;
        } else {
            govvoters.modify(from, voter, [&](auto &a) {
                a.voting_amount -= value;
                auto govsettings = govsetting_get();
                ds_assert(a.voting_amount >= govsettings.min_vote_stake, "wrong amount % expected >= %",
                          a.voting_amount, govsettings.min_vote_stake);
            });
            activate_bpvote_internal(voter);
        }
        updvotingbal(-value);
        return still_alive;
    }

    void eosdtgovernc::transfer(const ds_account &from, const ds_account &to,
                                ds_asset &quantity, const ds_string &memo) {
        PRINT_STARTED("transfer"_n)
        if (to != _self)
            return;
        auto ctract = ctr_by_symbol(quantity.symbol);
        ds_assert(ctract == _first_receiver, "wrong contract % for asset: % expected: %.", _first_receiver, quantity,
                  ctract);
        auto govsettings = govsetting_get();
        if (from == ctrsetting_get().liquidator_account || memo.find("update_balance") == 0) {
            ds_print("\r\nupdating balance %", quantity);
            PRINT_FINISHED("updating"_n)
            return;
        }
        if (memo.length()>0 && memo.find("bp_deposit:") == 0){
            ds_assert(quantity >= govsettings.min_reward, "wrong amount % expected >= %", quantity,
                      govsettings.min_reward);
            bpdeposit(memo.c_str() + 11,quantity);
        }
        else {
            ds_assert(quantity >= govsettings.min_vote_stake, "wrong amount % expected >= %", quantity,
                      govsettings.min_vote_stake);
            voting_amount_add(from, quantity, _self);
        }
        reinit();
        PRINT_FINISHED("transfer"_n)
    }

    void eosdtgovernc::unstake(const ds_account &voter, ds_asset &quantity) {
        PRINT_STARTED("unstake"_n)
        require_auth(voter);
        auto time = time_get();
        auto settings = govsetting_get();
        govvoters_table govvoters(_self, _self.value);
        auto voter_itr = govvoters.find(voter.value);
        ds_assert(voter_itr != govvoters.end(), "govvoters isn't found.");
        ds_assert(quantity.symbol == UTILITY_SYMBOL, "wrong quantity: % expected:  %", quantity, UTILITY_SYMBOL);
        ds_assert(quantity <= voter_itr->voting_amount, "voting_amount % not enough to unstake %",
                  voter_itr->voting_amount, quantity);
        ds_assert(quantity.amount > 0, "unstake % should be more than zero.",
                quantity);
        auto count_all = 0;
        auto count_active = 0;
        govvotes_table
                govvotes(_self, _self.value);
        auto index = govvotes.template get_index<"byvoter"_n>();
        for (auto itr = index.upper_bound(compress_key(voter.value, 0));
             itr != index.end() && itr->voter == voter; itr++) {
            count_all++;
            if (itr->proposal_name == BLOCKPRODUCEPROPOSAL) {
                continue;
            }
            if ((ds_long) time.sec_since_epoch() - (ds_long) settings.freeze_period >
                (ds_long) proposal_get(itr->proposal_name).expires_at.sec_since_epoch()) {
                continue;
            }
            count_active++;
        }
        ds_assert(count_active == 0,
                  "count_all: %, count_active: %, please unvote before unstake",
                  count_all, count_active);
        ds_assert(voter_itr->withdrawal_date <= time, "please wait % before unstake", voter_itr->withdrawal_date);
        auto nut_balance = balance_get(_self,NUT_SYMBOL);
        auto nut_voting_balance = getvotingbal();
        auto dividend = op_mul_div(quantity,to_ldouble(nut_balance),to_ldouble(nut_voting_balance));
        ds_print("\r\ndividend(%) = quantity(%)*nut_balance(%)/nut_voting_balance(%).",
                 dividend,quantity,nut_balance,nut_voting_balance);

        bool still_alive = voting_amount_sub(voter, quantity);
        trans(voter, dividend, "unstake"); //dividend
        if (!still_alive) {
            bpvoteerase(voter);
        }
        reinit();
        PRINT_FINISHED("unstake"_n)
    }

    void eosdtgovernc::voterreborn(const ds_account &voter) {
        PRINT_STARTED("voterreborn"_n)
        auto ram_payer = _self;
        if (!has_auth(_self)) {
            ram_payer = voter;
            require_auth(voter);
        }
        ds_print("\r\nauth ok");
        struct voter_del {
            ds_asset voting_amount;
            ds_time withdrawal_date;
            ds_ulong primary_key() const { return voting_amount.symbol.code().raw(); }
        };
        ds_print("\r\nstruct ok");
        eosio::multi_index<"voters"_n, voter_del> voters_del(_self, voter.value);
        ds_print("\r\nmultiindex ok");
        auto voter_itr = voters_del.find(UTILITY_SYMBOL.code().raw());
        ds_print("\r\nfind voter %", voter);
        ds_assert(voter_itr != voters_del.end(), "voters isn't found.");
        auto voting_amount = voter_itr->voting_amount;
        auto withdrawal_date = voter_itr->withdrawal_date;
        voters_del.erase(voter_itr);
        ds_print("\r\nerase voter %", voter);
        govvoters_table govvoters(_self, _self.value);
        govvoters.emplace(ram_payer, [&](auto &a) {
            a.voter = voter;
            a.voting_amount = voting_amount;
            a.withdrawal_date = withdrawal_date;
        });
        PRINT_FINISHED("voterreborn"_n)
    }

}
