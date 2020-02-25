#include "eosdtgovernc.hpp"

namespace eosdt {

    void eosdtgovernc::propose(
            const ds_account &proposer,
            const ds_account &proposal_name,
            const ds_string &title,
            const ds_string &proposal_json,
            const ds_time &expires_at,
            uint8_t proposal_type
    ) {
        PRINT_STARTED("propose"_n)
        require_auth(proposer);
        govproposals_table
                govproposals(_self, _self.value);

        auto already_created = 1;
        auto index = govproposals.template get_index<"byproposer"_n>();
        for (auto itr = index.upper_bound(proposer.value);
             itr != index.end() && itr->proposer == proposer; itr++) {
            already_created++;
        }

        auto govsettings = govsetting_get();
        auto balance = balance_get(proposer, UTILITY_SYMBOL) + voting_amount_get(proposer, UTILITY_SYMBOL);
        auto min_proposal_weight = op_mul_div(orarate_get(UTILITY_SYMBOL),
                                              to_ldouble(govsettings.min_proposal_weight) * already_created,
                                              to_ldouble(orarate_get(USD_SYMBOL)));
        ds_assert(min_proposal_weight <= balance, "balance % not enough to make a proposal, expected >= % ",
                  balance, min_proposal_weight);

        ds_assert(proposal_name.length() > 2, "proposal_name should be at least 3 characters long.");
        ds_assert(title.size() < 1024, "title should be less than 1024 characters long.");
        ds_assert(proposal_json.size() < 32768, "proposal_json should be less than 32768 characters long.");
        ds_assert(proposal_type == 1, "proposal_type % is unsupported", proposal_type);
        validate_proposal_json_main(proposal_json.c_str());

        auto time = time_get();
        ds_assert(expires_at > time, "expires_at (%) must be a value in the future.", expires_at);
        ds_assert(expires_at <= time + 6 * MONTHS_IN_SECONDS, "expires_at (%) must be within 6 months from now.",
                  expires_at);
        ds_assert(govproposals.find(proposal_name.value) == govproposals.end(),
                  "proposal with same name already exists.");

        govproposals.emplace(proposer, [&](auto &row) {
            row.proposal_name = proposal_name;
            row.proposer = proposer;
            row.title = title;
            row.proposal_json = proposal_json;
            row.created_at = time_get();
            row.expires_at = expires_at;
            row.proposal_type = proposal_type;
        });
        PRINT_FINISHED("propose"_n)
    }

    auto eosdtgovernc::proposal_get(const ds_account &proposal_name) {
        govproposals_table
                govproposals(_self, _self.value);
        auto itr = govproposals.find(proposal_name.value);
        ds_assert(itr != govproposals.end(), "govproposal %.", DOES_NOT_EXIST);
        return *itr;
    }

    void eosdtgovernc::proposal_del(const ds_account &proposal_name) {
        govproposals_table
                govproposals(_self, _self.value);
        auto itr = govproposals.find(proposal_name.value);
        ds_assert(itr != govproposals.end(), "proposal %.", DOES_NOT_EXIST);
        govproposals.erase(itr);
    }

    void eosdtgovernc::clnproposal(const ds_account &proposal_name, const ds_ulong &max_count) {
        PRINT_STARTED("clnproposal"_n)
        auto time = time_get();
        auto freeze_period = govsetting_get().freeze_period;
        auto proposal = proposal_get(proposal_name);
        auto needtime = (ds_long) proposal.expires_at.sec_since_epoch() + (ds_long) freeze_period -
                        (ds_long) time.sec_since_epoch();
        ds_assert(needtime <= 0,
                  "proposal % must be expired for at least % seconds prior to running clnproposal (need wait % sec).",
                  proposal_name, freeze_period, needtime);

        govvotes_table
                govvotes(_self, _self.value);
        auto index = govvotes.template get_index<"byproposal"_n>();
        ds_ulong count = 0;
        bool isLimit = false;
        for (auto itr = index.upper_bound(compress_key(proposal_name.value, 0));
             itr != index.end() && itr->proposal_name == proposal_name;
             itr = index.erase(itr)) {
            count++;
            if (count > max_count) {
                isLimit = true;
                break;
            }
        }

        if (!isLimit) {
            proposal_del(proposal_name);
        }
        PRINT_FINISHED("clnproposal"_n)
    }

    void eosdtgovernc::expire(const ds_account &proposal_name) {
        PRINT_STARTED("expire"_n)
        govproposals_table
                govproposals(_self, _self.value);
        auto itr = govproposals.find(proposal_name.value);

        ds_assert(itr != govproposals.end(), "proposal % not found.", proposal_name);
        auto time = time_get();
        ds_assert(itr->expires_at > time, "proposal % is already expired.", proposal_name);

        require_auth(itr->proposer);

        govproposals.modify(itr, ds_account(0), [&](auto &row) {
            row.expires_at = time;
        });
        PRINT_FINISHED("expire"_n)
    }

}
