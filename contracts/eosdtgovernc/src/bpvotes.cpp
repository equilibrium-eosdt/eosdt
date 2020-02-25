#include "eosdtgovernc.hpp"

namespace eosdt {
    auto eosdtgovernc::get_json_parser_for_bpvotes(const char *json) {
        json_parser::k_v_map key_values;

        key_values.insert(json_parser::k_v_pair("eosdtbpproxy.producers", json_parser::VALUE_ARRAY));

        return json_parser(json, key_values);
    }

    void eosdtgovernc::validate_bpvote_json(const ds_string &vote_json) {
        std::vector <std::string> vote_producers;
        auto parser = get_json_parser_for_bpvotes(vote_json.c_str());
        auto parse_status = parser.parse();
        for (; parse_status == json_parser::STATUS_OK; parse_status = parser.parse()) {
            if (parser.is_key_equals("eosdtbpproxy.producers")) {
                vote_producers = parser.get_out_array();
            }
        }
        ds_assert(parse_status == json_parser::STATUS_END, "\r\nproposal_json is invalid, code %", parse_status);
        auto max_bp_votes = govsetting_get().max_bp_votes;
        ds_assert(vote_producers.size() <= max_bp_votes, "too many producers, expected <= %", max_bp_votes);

        sysproducers_table sysproducers("eosio"_n, ("eosio"_n).value);
        for (auto producer_name:vote_producers) {
            auto producer = sysproducers.find((ds_account{producer_name}).value);
            ds_assert(producer != sysproducers.end(), "producer '%' isn't registered.", producer_name);
            ds_assert(producer->producer_key != ds_public_key() && producer->is_active,
                      "producer '%' isn't active.", producer_name);
        }
    }

    void eosdtgovernc::activate_bpvote_internal(const ds_account &voter) {
        ds_print("\r\nstart activate_bpvote_internal");
        auto voting_amount = voting_amount_get(voter, UTILITY_SYMBOL);
        govvotes_table govvotes(_self, _self.value);
        auto index = govvotes.template get_index<"byproposal"_n>();
        auto itr = index.find(compress_key(BLOCKPRODUCEPROPOSAL.value, voter.value));
        if (itr != index.end() and voting_amount.amount > 0) {
            govbpvotes_table govbpvotes(_self, _self.value);
            std::vector <ds_account> vote_producers;
            auto parser = get_json_parser_for_bpvotes(itr->vote_json.c_str());
            auto parse_status = parser.parse();
            for (; parse_status == json_parser::STATUS_OK; parse_status = parser.parse()) {
                if (parser.is_key_equals("eosdtbpproxy.producers")) {
                    for (auto producer_name:parser.get_out_array()) {
                        vote_producers.push_back(eosio::name{producer_name});
                    }
                    sort( vote_producers.begin(), vote_producers.end() );
                    vote_producers.erase( unique( vote_producers.begin(), vote_producers.end() ), vote_producers.end() );
                    auto max_bp_votes = govsetting_get().max_bp_votes;
                    if (vote_producers.size() > max_bp_votes) {
                        vote_producers.resize(max_bp_votes);
                    }
                    break;
                }
            }
            for (auto bpaccount:vote_producers) {
                ds_print("\r\ngovbpvotes.find: account: %", bpaccount);
                auto itr = govbpvotes.find(bpaccount.value);
                if (itr == govbpvotes.end()) {
                    ds_print("\r\nactivate.emplace: account: %, amount: %.", bpaccount, voting_amount);
                    govbpvotes.emplace(_self, [&](auto &b) {
                        b.producer = bpaccount;
                        b.votes = voting_amount;
                    });
                } else {
                    ds_print("\r\nactivate.modify: account: %, amount: %.", bpaccount, voting_amount);
                    govbpvotes.modify(itr, _self, [&](auto &b) {
                        b.votes += voting_amount;
                    });
                }
            }
        }
        ds_print("\r\nstop activate_bpvote_internal");
    }

    void eosdtgovernc::deactivate_bpvote_internal(const ds_account &voter) {
        auto voting_amount = voting_amount_get(voter, UTILITY_SYMBOL);
        govvotes_table govvotes(_self, _self.value);
        auto index = govvotes.template get_index<"byproposal"_n>();
        auto itr = index.find(compress_key(BLOCKPRODUCEPROPOSAL.value, voter.value));
        if (itr != index.end() and voting_amount.amount > 0) {
            govbpvotes_table govbpvotes(_self, _self.value);
            std::vector <ds_account> vote_producers;
            auto parser = get_json_parser_for_bpvotes(itr->vote_json.c_str());
            auto parse_status = parser.parse();
            for (; parse_status == json_parser::STATUS_OK; parse_status = parser.parse()) {
                if (parser.is_key_equals("eosdtbpproxy.producers")) {
                    for (auto producer_name:parser.get_out_array()) {
                        vote_producers.push_back(eosio::name{producer_name});
                    }
                    sort( vote_producers.begin(), vote_producers.end() );
                    vote_producers.erase( unique( vote_producers.begin(), vote_producers.end() ), vote_producers.end() );
                    auto max_bp_votes = govsetting_get().max_bp_votes;
                    if (vote_producers.size() > max_bp_votes) {
                        vote_producers.resize(max_bp_votes);
                    }
                    break;
                }
            }
            for (auto bpaccount:vote_producers) {
                auto itr = govbpvotes.find(bpaccount.value);
                if (itr != govbpvotes.end()) {
                    if (itr->votes == voting_amount) {
                        ds_print("\r\nactivate.erase: account: %, amount: %.", bpaccount, voting_amount);
                        govbpvotes.erase(itr);
                    } else {
                        ds_print("\r\nactivate.modify: account: %, amount: %.", bpaccount, voting_amount);
                        govbpvotes.modify(itr, _self, [&](auto &b) {
                            b.votes -= voting_amount;
                        });
                    }
                }
            }
        }
    }

    void eosdtgovernc::bpvoteerase(const ds_account &voter) {
        PRINT_STARTED("bpvoteerase"_n)
        govvotes_table govvotes(_self, _self.value);
        auto index = govvotes.template get_index<"byproposal"_n>();
        auto itr = index.find(compress_key(BLOCKPRODUCEPROPOSAL.value, voter.value));
        if (itr != index.end()) {
            index.erase(itr);
        }
        PRINT_FINISHED("bpvoteerase"_n)
    }

    void eosdtgovernc::bpvotescount() {
        PRINT_STARTED("bpvotescount"_n)
        govbpvotes_table govbpvotes(_self, _self.value);
        for (auto itr = govbpvotes.begin(); itr != govbpvotes.end(); itr = govbpvotes.erase(itr));

        govvotes_table govvotes(_self, _self.value);
        auto index = govvotes.template get_index<"byproposal"_n>();
        for (auto itr = index.upper_bound(compress_key(BLOCKPRODUCEPROPOSAL.value, 0));
             itr != index.end() && itr->proposal_name == BLOCKPRODUCEPROPOSAL; itr++) {
            activate_bpvote_internal(itr->voter);
        }
        PRINT_FINISHED("bpvotescount"_n)
    }

}
