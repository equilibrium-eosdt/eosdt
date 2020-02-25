#include "eosdtgovernc.hpp"

namespace eosdt {
    void eosdtgovernc::vote_count(const ds_account &proposal_name,
                                  ds_int &count_all, ds_int &count_yes, ds_int &count_no,
                                  ds_asset &quantity_all, ds_asset &quantity_yes, ds_asset &quantity_no,
                                  std::map <std::string, ds_asset> &vote_producers) {
        auto proposal = proposal_get(proposal_name);
        quantity_all = quantity_yes = quantity_no = ds_asset(0, UTILITY_SYMBOL);
        govvotes_table
                govvotes(_self, _self.value);
        auto index = govvotes.template get_index<"byproposal"_n>();
        for (auto itr = index.upper_bound(compress_key(proposal_name.value, 0));
             itr != index.end() && itr->proposal_name == proposal_name; itr++) {
            count_all++;
            auto voting_amount = voting_amount_get(itr->voter, UTILITY_SYMBOL);
            quantity_all += voting_amount;
            if (itr->vote == 1) {
                count_yes++;
                quantity_yes += voting_amount;
            } else {
                count_no++;
                quantity_no += voting_amount;
                continue;
            }
        }
        ds_print(
                "\r\nproposal: %, count_all: %, count_yes: %, count_no: %, quantity_all: %, quantity_yes: %, quantity_no: %",
                proposal_name, count_all, count_yes, count_no, quantity_all, quantity_yes, quantity_no);
    }

    void eosdtgovernc::vote(const ds_account &voter, const ds_account &proposal_name, uint8_t vote,
                            const ds_string &vote_json) {
        PRINT_STARTED("vote"_n)
        require_auth(voter);
        auto voting_amount = voting_amount_get(voter, UTILITY_SYMBOL);
        ds_assert(voting_amount.amount > 0, "please stake before voting");
        ds_print("\r\nvalidation");
        if (proposal_name == BLOCKPRODUCEPROPOSAL) {
            validate_bpvote_json(vote_json);
        } else {
            auto proposal = proposal_get(proposal_name);
            auto time = time_get();
            ds_assert(proposal.expires_at > time, "cannot vote on an expired % proposal, now: %.",
                      proposal.expires_at, time);
        }

        govvotes_table govvotes(_self, _self.value);
        auto index = govvotes.template get_index<"byproposal"_n>();
        auto itr = index.find(compress_key(proposal_name.value, voter.value));
        if (itr == index.end()) {
            ds_print("\r\nvotes.emplace");
            govvotes.emplace(voter, [&](auto &row) {
                row.id = govvotes.available_primary_key();
                row.proposal_name = proposal_name;
                row.voter = voter;
                row.updated_at = time_get();
                row.vote = vote;
                row.vote_json = vote_json;
            });
        } else {
            if (proposal_name == BLOCKPRODUCEPROPOSAL) {
                ds_print("\r\ndeactivate_bpvote_internal");
                deactivate_bpvote_internal(voter);
            }
            ds_print("\r\nindex.modify");
            index.modify(itr, voter, [&](auto &row) {
                row.updated_at = time_get();
                row.vote = vote;
                row.vote_json = vote_json;
            });
        }
        if (proposal_name == BLOCKPRODUCEPROPOSAL) {
            ds_print("\r\nactivate_bpvote_internal");
            activate_bpvote_internal(voter);
            reinit();
        }
        PRINT_FINISHED("vote"_n)
    }

    void eosdtgovernc::unvote(const ds_account &voter, const ds_account &proposal_name) {
        PRINT_STARTED("unvote"_n)
        require_auth(voter);
        auto freeze_period = govsetting_get().freeze_period;
        auto proposal = proposal_get(proposal_name);
        auto time = time_get();
        if (proposal.expires_at < time) {
            ds_assert(time > ds_time(proposal.expires_at + freeze_period),
                      "cannot unvote on an expired proposal within its freeze period.", proposal.expires_at,
                      time);
        }

        govvotes_table
                govvotes(_self, _self.value);

        auto index = govvotes.template get_index<"byproposal"_n>();

        auto itr = index.find(compress_key(proposal_name.value, voter.value));
        ds_assert(itr != index.end(), "no vote exists for this %/%.", voter, proposal_name);

        govvotes.erase(*itr);
        PRINT_FINISHED("unvote"_n)
    }

}
