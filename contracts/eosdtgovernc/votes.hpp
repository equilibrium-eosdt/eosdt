#include "proposals.hpp"

class votes : public proposals {

protected:
    void vote_count(const ds_account &proposal_name,
                    ds_int &count_all, ds_int &count_yes, ds_int &count_no,
                    ds_asset &quantity_all, ds_asset &quantity_yes, ds_asset &quantity_no,
                    std::map<std::string,ds_asset> &vote_producers) {
        auto proposal = proposal_get(proposal_name);
        quantity_all = quantity_yes = quantity_no = ds_asset(0, UTILITY_SYMBOL);
        votes_table
        votes(_self, _self.value);
        auto index = votes.template get_index<"byproposal"_n>();
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
    void validate_bpvote_json(ds_string vote_json) {
        std::vector <std::string> vote_producers;
        auto parser = get_json_parser_for_bpvotes(vote_json.c_str());
        auto parse_status = parser.parse();
        for (; parse_status == json_parser::STATUS_OK; parse_status = parser.parse()) {
            if (parser.is_key_equals("eosdtbpproxy.producers")) {
                vote_producers = parser.get_out_array();
            }
        }
        ds_assert(parse_status == json_parser::STATUS_END, "\r\nproposal_json is invalid, code %", parse_status);
        auto max_bp_votes = settings_get().max_bp_votes;
        ds_assert(vote_producers.size() <= max_bp_votes, "too many producers, expected <= %", max_bp_votes);

        eosio::multi_index<"producers"_n, producer_info> producers("eosio"_n, ("eosio"_n).value);
        for (auto producer_name:vote_producers) {
            auto producer = producers.find((eosio::name{producer_name}).value);
            ds_assert(producer != producers.end(), "producer '%' isn't registered.", producer_name);
            ds_assert(producer->producer_key != eosio::public_key() && producer->is_active,
                      "producer '%' isn't active.", producer_name);
        }
    }

public:
    votes(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) :
            proposals(receiver, code, ds) {
    }

    void vote(const ds_account &voter, const ds_account &proposal_name, uint8_t vote, ds_string vote_json) {
        PRINT_STARTED("vote"_n)
        require_auth(voter);
        auto voting_amount = voting_amount_get(voter, UTILITY_SYMBOL);
        ds_assert(voting_amount.amount > 0, "please stake before voting");
        ds_print("\r\nvalidation");
        if(proposal_name == blockproduceproposal) {
            validate_bpvote_json(vote_json);
        } else {
            auto proposal = proposal_get(proposal_name);
            auto time = time_get();
            ds_assert(proposal.expires_at > time, "cannot vote on an expired % proposal, now: %.",
                      proposal.expires_at, time);
        }

        votes_table votes(_self, _self.value);
        auto index = votes.template get_index<"byproposal"_n>();
        auto itr = index.find(compress_key(proposal_name.value, voter.value));
        if (itr == index.end()) {
            ds_print("\r\nvotes.emplace");
            votes.emplace(voter, [&](auto &row) {
                row.id = votes.available_primary_key();
                row.proposal_name = proposal_name;
                row.voter = voter;
                row.updated_at = time_get();
                row.vote = vote;
                row.vote_json = vote_json;
            });
        } else {
            if(proposal_name == blockproduceproposal) {
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
        if(proposal_name == blockproduceproposal) {
            ds_print("\r\nactivate_bpvote_internal");
            activate_bpvote_internal(voter);
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
        auto time = time_get();
        auto settings = settings_get();
        voters_table voters_tbl(_self, voter.value);
        auto voter_itr = voters_tbl.require_find(UTILITY_SYMBOL.code().raw());

        ds_assert(quantity.symbol == UTILITY_SYMBOL, "wrong quantity: % expected:  %", quantity, UTILITY_SYMBOL);
        ds_assert(quantity <= voter_itr->voting_amount, "voting_amount % not enough to unstake %",
                  voter_itr->voting_amount, quantity);
        auto count_all = 0;
        auto count_active = 0;
        votes_table
                votes(_self, _self.value);
        auto index = votes.template get_index<"byvoter"_n>();
        for (auto itr = index.upper_bound(compress_key(voter.value, 0));
             itr != index.end() && itr->voter == voter; itr++) {
            count_all++;
            if (itr->proposal_name == blockproduceproposal) {
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
        if (voter_itr->withdrawal_date > time) {
            ds_print("please wait % before unstake", voter_itr->withdrawal_date);
        }
        else {
            voting_amount_sub(voter, quantity);
            trans(voter, quantity, "unstake");
        }
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
