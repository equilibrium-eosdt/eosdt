#include "proposals.hpp"
class votes : public proposals {
private:
    typedef eosio::multi_index<"votes"_n, vote,
            eosio::indexed_by<"byproposal"_n, eosio::const_mem_fun < vote, __uint128_t, &vote::by_proposal>>,
    eosio::indexed_by<"byvoter"_n, eosio::const_mem_fun<vote, __uint128_t, &vote::by_voter>>>
    votes_table;
protected:
    void vote_count(const ds_account &proposal_name,
                    ds_int &count_all, ds_int &count_yes, ds_int &count_no,
                    ds_asset &quantity_all, ds_asset &quantity_yes, ds_asset &quantity_no) {
        quantity_all = quantity_yes = quantity_no = ds_asset(0, UTILITY_SYMBOL);
        votes_table
        votes(_self, _self.value);
        auto index = votes.template get_index<"byproposal"_n>();
        for (auto itr = index.upper_bound(compress_key(proposal_name.value, 0));
             itr != index.end() && itr->proposal_name == proposal_name; itr++) {
            count_all++;
            quantity_all += itr->quantity;
            if (itr->vote == 1) {
                count_yes++;
                quantity_yes += itr->quantity;
            } else {
                count_no++;
                quantity_no += itr->quantity;
            }
        }
        ds_print(
                "\r\nproposal: %, count_all: %, count_yes: %, count_no: %, quantity_all: %, quantity_yes: %, quantity_no: %",
                proposal_name, count_all, count_yes, count_no, quantity_all, quantity_yes, quantity_no);
    }
public:
    votes(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) :
            proposals(receiver, code, ds) {
    }
    void vote(const ds_account &voter, const ds_account &proposal_name, uint8_t vote) {
        PRINT_STARTED("vote"_n)
        require_auth(voter);
        auto voting_amount = voting_amount_get(voter, UTILITY_SYMBOL);
        auto proposal = proposal_get(proposal_name);
        auto time = time_get();
        ds_assert(proposal.expires_at > time, "cannot vote on an expired % proposal, now: %.",
                  proposal.expires_at, time);
        ds_assert(voting_amount.amount > 0, "cannot vote on proposal % without voting_amount.",
                  proposal_name);
        votes_table
        votes(_self, _self.value);
        auto index = votes.template get_index<"byproposal"_n>();
        auto itr = index.find(compress_key(proposal_name.value, voter.value));
        if (itr == index.end()) {
            votes.emplace(voter, [&](auto &row) {
                row.id = votes.available_primary_key();
                row.proposal_name = proposal_name;
                row.voter = voter;
                row.updated_at = time_get();
                row.vote = vote;
                row.quantity = voting_amount;
            });
        } else {
            ds_assert(itr->vote != vote || itr->quantity != voting_amount, "vote %", ALREADY_STORED);
            index.modify(itr, voter, [&](auto &row) {
                row.updated_at = time_get();
                row.vote = vote;
                row.quantity = voting_amount;
            });
        }
        PRINT_FINISHED("vote"_n)
    }
    void unvote(const ds_account &voter, const ds_account &proposal_name) {
        PRINT_STARTED("unvote"_n)
        require_auth(voter);
        auto freeze_period = settings_get().freeze_period;
        auto proposal = proposal_get(proposal_name);
        auto time = time_get();
        if (proposal.expires_at < time) {
            ds_assert(time > ds_time(proposal.expires_at + freeze_period),
                      "cannot unvote on an expired proposal within its freeze period.", proposal.expires_at,
                      time);
        }
        votes_table
        votes(_self, _self.value);
        auto index = votes.template get_index<"byproposal"_n>();
        auto itr = index.find(compress_key(proposal_name.value, voter.value));
        ds_assert(itr != index.end(), "no vote exists for this %/%.", voter, proposal_name);
        votes.erase(*itr);
        PRINT_FINISHED("unvote"_n)
    }
    void unstake(const ds_account &voter,
                 ds_asset &quantity) {
        PRINT_STARTED("unstake"_n)
        require_auth(voter);
        ds_assert(quantity.symbol == UTILITY_SYMBOL, "wrong quantity: % expected:  %", quantity, UTILITY_SYMBOL);
        auto voting_amount = voting_amount_get(voter, UTILITY_SYMBOL);
        ds_assert(quantity <= voting_amount, "voting_amount % not enough to unstake %", voting_amount, quantity);
        auto time = time_get();
        auto freeze_period = settings_get().freeze_period;
        auto used = ds_asset(0, UTILITY_SYMBOL);
        auto count_all = 0;
        auto count_active = 0;
        votes_table
        votes(_self, _self.value);
        auto index = votes.template get_index<"byvoter"_n>();
        for (auto itr = index.upper_bound(compress_key(voter.value, 0));
             itr != index.end() && itr->voter == voter; itr++) {
            count_all++;
            if ((ds_long) time.sec_since_epoch() - (ds_long) freeze_period >
                (ds_long) proposal_get(itr->proposal_name).expires_at.sec_since_epoch()) {
                continue;
            }
            count_active++;
            if (itr->quantity > used) {
                used = itr->quantity;
            }
        }
        ds_assert(quantity <= voting_amount - used,
                  "count_all: %, count_active: %, %(voting_amount % - used %) not enough to unstake %",
                  count_all, count_active, voting_amount - used, voting_amount, used, quantity);
        voting_amount_sub(voter, quantity);
        trans(voter, quantity, "unstake");
        PRINT_FINISHED("unstake"_n)
    }
    void clnproposal(const ds_account &proposal_name, const ds_ulong &max_count) {
        PRINT_STARTED("clnproposal"_n)
        auto time = time_get();
        auto settings = settings_get();
        auto proposal = proposal_get(proposal_name);
        auto needtime = (ds_long) proposal.expires_at.sec_since_epoch() + (ds_long) settings.freeze_period -
                        (ds_long) time.sec_since_epoch();
        ds_assert(needtime <= 0,
                  "proposal % must be expired for at least % seconds prior to running clnproposal (need wait % sec).",
                  proposal_name, settings.freeze_period, needtime);
        votes_table
        votes(_self, _self.value);
        auto index = votes.template get_index<"byproposal"_n>();
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
};
