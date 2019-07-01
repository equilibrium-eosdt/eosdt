#include "balances.hpp"
#include "json_parser.hpp"

class proposals : public balances {

private:
    typedef eosio::multi_index<"proposals"_n, proposal,
            eosio::indexed_by<"byproposer"_n, eosio::const_mem_fun < proposal, uint64_t, &proposal::by_proposer>>
    >
    proposals_table;

protected:
    auto proposal_get(const ds_account &proposal_name) {
        proposals_table
                proposals(_self, _self.value);
        auto itr = proposals.find(proposal_name.value);
        ds_assert(itr != proposals.end(), "proposal %.", DOES_NOT_EXIST);
        return *itr;
    }

    void proposal_del(const ds_account &proposal_name) {
        proposals_table
                proposals(_self, _self.value);
        auto itr = proposals.find(proposal_name.value);
        ds_assert(itr != proposals.end(), "proposal %.", DOES_NOT_EXIST);
        proposals.erase(itr);
    }
    json_parser get_json_parser(const char *json) {
        json_parser::k_v_map key_values;
        key_values.insert(json_parser::k_v_pair("eosdtcntract.governance_fee", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.stability_fee", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.critical_ltv", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.liquidation_penalty", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.liquidator_discount", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.nut_auct_ratio", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.nut_discount", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.profit_factor", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.vote_period", json_parser::VALUE_INT));
        key_values.insert(json_parser::k_v_pair("eosdtcntract.stake_period", json_parser::VALUE_INT));

        key_values.insert(json_parser::k_v_pair("eosdliqdatr.eosdtcntract_account", json_parser::VALUE_NAME));

        key_values.insert(json_parser::k_v_pair("eosdtorclize.utility_listing_date", json_parser::VALUE_DATETIME));
        key_values.insert(json_parser::k_v_pair("eosdtorclize.rate_timeout", json_parser::VALUE_INT));
        key_values.insert(json_parser::k_v_pair("eosdtorclize.query_timeout", json_parser::VALUE_INT));
        key_values.insert(json_parser::k_v_pair("eosdtorclize.master_interval", json_parser::VALUE_INT));
        key_values.insert(json_parser::k_v_pair("eosdtorclize.slave_interval", json_parser::VALUE_INT));

        key_values.insert(json_parser::k_v_pair("eosdtgovernc.eosdtcntract_account", json_parser::VALUE_NAME));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.oraclize_account", json_parser::VALUE_NAME));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.nutoken_account", json_parser::VALUE_NAME));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.min_proposal_weight", json_parser::VALUE_ASSET));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.freeze_period", json_parser::VALUE_INT));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.min_participation", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.success_margin", json_parser::VALUE_DOUBLE));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.top_holders_amount", json_parser::VALUE_INT));
        key_values.insert(json_parser::k_v_pair("eosdtgovernc.min_threshold", json_parser::VALUE_DOUBLE));

        key_values.insert(json_parser::k_v_pair("reserved", json_parser::VALUE_STRING));
        return json_parser(json, key_values);
    }

    json_parser get_json_parser_for_bpvotes(const char *json) {
        json_parser::k_v_map key_values;

        key_values.insert(json_parser::k_v_pair("eosdtcntract.producers", json_parser::VALUE_ARRAY));

        return json_parser(json, key_values);
    }


public:
    proposals(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) :
            balances(receiver, code, ds) {
    }


    bool validate_proposal_json_main(const char *json) {
        auto parser = get_json_parser(json);
        auto s = parser.validate();
        ds_assert(s == json_parser::STATUS_END, "\r\nproposal_json for type main is invalid, code %", s);
        return true;
    }

    bool validate_proposal_json_bpvoting(const char *json) {
        auto parser = get_json_parser_for_bpvotes(json);
        auto parse_status = parser.parse();
        eosio::multi_index<"producers"_n, producer_info> producers("eosio"_n, ("eosio"_n).value);
        for (; parse_status == json_parser::STATUS_OK; parse_status = parser.parse()) {
            if (parser.is_key_equals("eosdtcntract.producers")) {
                for (auto producer_name:parser.get_out_array()) {
                    auto producer = producers.find((eosio::name{producer_name}).value);
                    ds_assert(producer != producers.end(), "producer: '%' did not register.", producer_name);
                }
            }
        }
        ds_assert(parse_status == json_parser::STATUS_END, "\r\nproposal_json for type bpvoting is invalid, code %",
                  parse_status);

        return true;
    }

    void propose(
            const ds_account &proposer,
            const ds_account &proposal_name,
            const ds_string &title,
            const ds_string &proposal_json,
            const ds_time &expires_at,
            uint8_t proposal_type
    ) {
        PRINT_STARTED("propose"_n)
        require_auth(proposer);
        proposals_table
                proposals(_self, _self.value);

        auto already_created = 1;
        auto index = proposals.template get_index<"byproposer"_n>();
        for (auto itr = index.upper_bound(proposer.value);
             itr != index.end() && itr->proposer == proposer; itr++) {
            already_created++;
        }

        auto settings = settings_get();
        auto balance = balance_get(proposer, UTILITY_SYMBOL) + voting_amount_get(proposer, UTILITY_SYMBOL);
        auto min_proposal_weight = op_mul_div(oracle_get_rate(UTILITY_SYMBOL),
                                              to_ldouble(settings.min_proposal_weight) * already_created,
                                              to_ldouble(oracle_get_rate(USD_SYMBOL)));
        ds_assert(min_proposal_weight <= balance, "balance % not enough to make a proposal, expected >= % ",
                  balance, min_proposal_weight);

        ds_assert(proposal_name.length() > 2, "proposal_name should be at least 3 characters long.");
        ds_assert(title.size() < 1024, "title should be less than 1024 characters long.");
        ds_assert(proposal_json.size() < 32768, "proposal_json should be less than 32768 characters long.");
        if (proposal_type == 1) {
            validate_proposal_json_main(proposal_json.c_str());
        } else if (proposal_type == 2) {
            validate_proposal_json_bpvoting(proposal_json.c_str());
        }


        ds_assert(expires_at > time_get(), "expires_at (%) must be a value in the future.", expires_at);

        ds_assert(expires_at <= time_get() + SIX_MONTHS_IN_SECONDS, "expires_at (%) must be within 6 months from now.",
                  expires_at);


        ds_assert(proposals.find(proposal_name.value) == proposals.end(), "proposal with same name already exists.");

        proposals.emplace(proposer, [&](auto &row) {
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

    void expire(const ds_account &proposal_name) {
        PRINT_STARTED("expire"_n)
        proposals_table
                proposals(_self, _self.value);
        auto itr = proposals.find(proposal_name.value);

        ds_assert(itr != proposals.end(), "proposal % not found.", proposal_name);
        auto time = time_get();
        ds_assert(itr->expires_at > time, "proposal % is already expired.", proposal_name);

        require_auth(itr->proposer);

        proposals.modify(itr, ds_account(0), [&](auto &row) {
            row.expires_at = time;
        });
        PRINT_FINISHED("expire"_n)
    }
};
