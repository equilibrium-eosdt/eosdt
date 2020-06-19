#include "eosdtgovernc.hpp"

namespace eosdt {
    void eosdtgovernc::apply_ctrsetting(const ds_string &json, const std::map <ds_account, ds_account> &cntract_types,
                                        const ds_account &pos_account, const char *pos_name) {
        ctrsettings_table ctrsettings(pos_account, pos_account.value);
        auto itr = ctrsettings.begin();
        ds_assert(itr != ctrsettings.end(), "Need to set up ctrsettings on ctract by forum.");
        auto apply_settings = *itr;
        auto parser = get_json_parser(json.c_str(), cntract_types);
        auto parse_status = parser.parse();
        auto settings_changed = false;
        auto apply_producers = std::vector < ds_account > {};
        for (; parse_status == json_parser::STATUS_OK; parse_status = parser.parse()) {
            if (!parser.is_key_equals_ctr(pos_name)) {
                continue;
            }
            auto ctr_len = strlen(pos_name);
            if (parser.is_key_equals_key(ctr_len, ".governance_fee")) {
                apply_settings.governance_fee = parser.get_out_double();
                settings_changed = true;
            } else if (parser.is_key_equals_key(ctr_len, ".stability_fee")) {
                apply_settings.stability_fee = parser.get_out_double();
                settings_changed = true;
            } else if (parser.is_key_equals_key(ctr_len, ".critical_ltv")) {
                apply_settings.critical_ltv = parser.get_out_double();
                settings_changed = true;
            } else if (parser.is_key_equals_key(ctr_len, ".liquidation_penalty")) {
                apply_settings.liquidation_penalty = parser.get_out_double();
                settings_changed = true;
            } else if (parser.is_key_equals_key(ctr_len, ".liquidator_discount")) {
                apply_settings.liquidator_discount = parser.get_out_double();
                settings_changed = true;
            } else if (parser.is_key_equals_key(ctr_len, ".nut_auct_ratio")) {
                apply_settings.nut_auct_ratio = parser.get_out_double();
                settings_changed = true;
            } else if (parser.is_key_equals_key(ctr_len, ".nut_discount")) {
                apply_settings.nut_discount = parser.get_out_double();
                settings_changed = true;
            } else if (parser.is_key_equals_key(ctr_len, ".profit_factor")) {
                apply_settings.profit_factor = parser.get_out_double();
                settings_changed = true;
            } else if (parser.is_key_equals_key(ctr_len, ".vote_period")) {
                apply_settings.vote_period = parser.get_out_double();
                settings_changed = true;
            } else if (parser.is_key_equals_key(ctr_len, ".stake_period")) {
                apply_settings.stake_period = parser.get_out_double();
                settings_changed = true;
            } else if (parser.is_key_equals_key(ctr_len, ".reserve_ratio")) {
                apply_settings.reserve_ratio = parser.get_out_double();
                settings_changed = true;
            } else if (parser.is_key_equals_key(ctr_len, ".staking_weight")) {
                apply_settings.staking_weight = parser.get_out_double();
                settings_changed = true;
            } else if (parser.is_key_equals_key(ctr_len, ".referral_min_stake")) {
                apply_settings.referral_min_stake = parser.get_out_asset();
                settings_changed = true;
            } else if (parser.is_key_equals_key(ctr_len, ".referral_ratio")) {
                apply_settings.referral_ratio = parser.get_out_double();
                settings_changed = true;
            } else if (parser.is_key_equals_key(ctr_len, ".collateral_token")) {
                apply_settings.collateral_token = parser.get_out_symbol();
                settings_changed = true;
            } else if (parser.is_key_equals_key(ctr_len, ".min_pos")) {
                apply_settings.min_pos = parser.get_out_asset();
                settings_changed = true;
            }
        }
        ds_assert(parse_status == json_parser::STATUS_END, "\r\nproposal_json is invalid, code %", parse_status);
        if (settings_changed) {
            if (pos_account == EOSDTCNTRACT) {
                eosio::action(
                        eosio::permission_level{pos_account, "active"_n},
                        pos_account,
                        "settingset"_n,
                        std::make_tuple(apply_settings.liquidator_account,
                                        apply_settings.oraclize_account,
                                        apply_settings.sttoken_account,
                                        apply_settings.nutoken_account,
                                        apply_settings.governance_fee,
                                        apply_settings.stability_fee,
                                        apply_settings.critical_ltv,
                                        apply_settings.liquidation_penalty,
                                        apply_settings.liquidator_discount,
                                        apply_settings.nut_auct_ratio,
                                        apply_settings.nut_discount,
                                        apply_settings.profit_factor,
                                        apply_settings.vote_period,
                                        apply_settings.stake_period,
                                        apply_settings.reserve_ratio,
                                        apply_settings.staking_weight,
                                        apply_settings.bpproxy_account,
                                        apply_settings.governc_account,
                                        apply_settings.referral_min_stake,
                                        apply_settings.referral_ratio,
                                        apply_settings.collateral_account,
                                        apply_settings.collateral_token,
                                        apply_settings.savings_account,
                                        apply_settings.min_pos
                        )
                ).send();
            } else {
                eosio::action(
                        eosio::permission_level{pos_account, "active"_n},
                        pos_account,
                        "settingset"_n,
                        std::make_tuple(apply_settings.liquidator_account,
                                        apply_settings.oraclize_account,
                                        apply_settings.sttoken_account,
                                        apply_settings.nutoken_account,
                                        apply_settings.stability_fee,
                                        apply_settings.critical_ltv,
                                        apply_settings.liquidation_penalty,
                                        apply_settings.liquidator_discount,
                                        apply_settings.nut_auct_ratio,
                                        apply_settings.nut_discount,
                                        apply_settings.governc_account,
                                        apply_settings.collateral_account,
                                        apply_settings.collateral_token,
                                        apply_settings.savings_account,
                                        apply_settings.min_pos
                        )
                ).send();
            }
        }
    }

    void eosdtgovernc::apply_liqsetting(const ds_string &json, const std::map <ds_account, ds_account> &cntract_types,
                                        const ds_account &liq_account, const char *liq_name) {
        liqsettings_table liqsettings(liq_account, liq_account.value);
        auto itr = liqsettings.begin();
        ds_assert(itr != liqsettings.end(), "Need to set up liqsettings on liqdatr by forum.");
        auto apply_struct = *itr;
        auto parser = get_json_parser(json.c_str(), cntract_types);
        auto parse_status = parser.parse();
        auto change_exists = false;
        for (; parse_status == json_parser::STATUS_OK; parse_status = parser.parse()) {
            if (parser.is_key_equals(liq_name,".position_account")) {
                apply_struct.position_account = parser.get_out_name();
                change_exists = true;
            }
            else if (parser.is_key_equals(liq_name,".burn_rate")) {
                apply_struct.burn_rate = parser.get_out_double();
                change_exists = true;
            }
            else if (parser.is_key_equals(liq_name,".gov_return_rate")) {
                apply_struct.gov_return_rate = parser.get_out_double();
                change_exists = true;
            }
            else if (parser.is_key_equals(liq_name,".set_aside_rate")) {
                apply_struct.set_aside_rate = parser.get_out_double();
                change_exists = true;
            }
            else if (parser.is_key_equals(liq_name,".savings_rate")) {
                apply_struct.savings_rate = parser.get_out_double();
                change_exists = true;
            }
        }
        ds_assert(parse_status == json_parser::STATUS_END, "\r\nproposal_json is invalid, code %", parse_status);
        if (change_exists) {
            eosio::action(
                    eosio::permission_level{liq_account, "active"_n},
                    liq_account,
                    "settingset"_n,
                    std::make_tuple(
                            apply_struct.position_account,
                            apply_struct.burn_rate,
                            apply_struct.gov_return_rate,
                            apply_struct.set_aside_rate,
                            apply_struct.savings_rate)
            ).send();
        }
    }

    void eosdtgovernc::apply_orasetting(const ds_string &json, const std::map <ds_account, ds_account> &cntract_types,
                                        const ds_account &ora_account, const char *ora_name) {
        orasettings_table orasettings(ora_account, ora_account.value);
        auto itr = orasettings.begin();
        ds_assert(itr != orasettings.end(), "Need to set up orasetings on oracle by forum.");
        auto apply_struct = *itr;
        auto parser = get_json_parser(json.c_str(), cntract_types);
        auto parse_status = parser.parse();
        auto change_exists = false;
        for (; parse_status == json_parser::STATUS_OK; parse_status = parser.parse()) {
            if (!parser.is_key_equals_ctr(ora_name)) {
                continue;
            }
            auto ctr_len = strlen(ora_name);
            if (parser.is_key_equals_key(ctr_len,".rate_timeout")) {
                ds_print("\r\nrate_timeout");
                apply_struct.rate_timeout = parser.get_out_int();
                change_exists = true;
            } else if (parser.is_key_equals_key(ctr_len,".query_timeout")) {
                apply_struct.query_timeout = parser.get_out_int();
                change_exists = true;
            } else if (parser.is_key_equals_key(ctr_len,".provablecb1a_interval")) {
                apply_struct.provablecb1a_interval = parser.get_out_int();
                change_exists = true;
            } else if (parser.is_key_equals_key(ctr_len,".delphioracle_interval")) {
                apply_struct.delphioracle_interval = parser.get_out_int();
                change_exists = true;
            } else if (parser.is_key_equals_key(ctr_len,".equilibriumdsp_interval")) {
                apply_struct.equilibriumdsp_interval = parser.get_out_int();
                change_exists = true;
            }
        }
        ds_assert(parse_status == json_parser::STATUS_END, "\r\nproposal_json is invalid, code %", parse_status);
        if (change_exists) {
            eosio::action(
                    eosio::permission_level{ora_account, "active"_n},
                    ora_account,
                    "settingset"_n,
                    std::make_tuple(
                            apply_struct.rate_timeout,
                            apply_struct.query_timeout,
                            apply_struct.provablecb1a_interval,
                            apply_struct.delphioracle_interval,
                            apply_struct.equilibriumdsp_interval,
                            apply_struct.validity_timeout)
            ).send();
        }

    }

    void eosdtgovernc::apply_govsetting(const ds_string &json, const std::map <ds_account, ds_account> &cntract_types,
                                        const ds_account &gov_account, const char *gov_name) {
        auto apply_struct = govsetting_get();
        auto parser = get_json_parser(json.c_str(), cntract_types);
        auto parse_status = parser.parse();
        auto change_exists = false;
        for (; parse_status == json_parser::STATUS_OK; parse_status = parser.parse()) {
            if (!parser.is_key_equals_ctr(gov_name)) {
                continue;
            }
            auto ctr_len = strlen(gov_name);
            if (parser.is_key_equals_key(ctr_len,".position_account")) {
                apply_struct.position_account = parser.get_out_name();
                change_exists = true;
            } else if (parser.is_key_equals_key(ctr_len,".min_proposal_weight")) {
                apply_struct.min_proposal_weight = parser.get_out_asset();
                change_exists = true;
            } else if (parser.is_key_equals_key(ctr_len,".freeze_period")) {
                apply_struct.freeze_period = parser.get_out_int();
                change_exists = true;
            } else if (parser.is_key_equals_key(ctr_len,".min_participation")) {
                apply_struct.min_participation = parser.get_out_double();
                change_exists = true;
            } else if (parser.is_key_equals_key(ctr_len,".success_margin")) {
                apply_struct.success_margin = parser.get_out_double();
                change_exists = true;
            } else if (parser.is_key_equals_key(ctr_len,".top_holders_amount")) {
                apply_struct.top_holders_amount = parser.get_out_int();
                change_exists = true;
            } else if (parser.is_key_equals_key(ctr_len,".max_bp_count")) {
                apply_struct.max_bp_count = parser.get_out_int();
                change_exists = true;
            } else if (parser.is_key_equals_key(ctr_len,".max_bp_votes")) {
                apply_struct.max_bp_votes = parser.get_out_int();
                change_exists = true;
            } else if (parser.is_key_equals_key(ctr_len,".min_vote_stake")) {
                apply_struct.min_vote_stake = parser.get_out_asset();
                change_exists = true;
            } else if (parser.is_key_equals_key(ctr_len,".unstake_period")) {
                apply_struct.unstake_period = parser.get_out_int();
                change_exists = true;
            } else if (parser.is_key_equals_key(ctr_len,".reward_weight")) {
                apply_struct.unstake_period = parser.get_out_int();
                change_exists = true;
            } else if (parser.is_key_equals_key(ctr_len,".stake_reward")) {
                apply_struct.stake_reward = parser.get_out_double();
                change_exists = true;
            }
        }
        ds_assert(parse_status == json_parser::STATUS_END, "\r\nproposal_json is invalid, code %", parse_status);
        if (change_exists) {
            eosio::action(
                    eosio::permission_level{_self, "active"_n},
                    _self,
                    "settingset"_n,
                    std::make_tuple(apply_struct.position_account,
                                    apply_struct.min_proposal_weight,
                                    apply_struct.freeze_period,
                                    apply_struct.min_participation,
                                    apply_struct.success_margin,
                                    apply_struct.top_holders_amount,
                                    apply_struct.max_bp_count,
                                    apply_struct.max_bp_votes,
                                    apply_struct.min_vote_stake,
                                    apply_struct.unstake_period,
                                    apply_struct.reward_weight,
                                    apply_struct.stake_reward
                    )
            ).send();

        }
    }

    void eosdtgovernc::apply(const ds_account &proposal_name) {
        PRINT_STARTED("apply"_n)
        auto proposal = proposal_get(proposal_name);
        auto settings = govsetting_get();
        auto freeze_period = settings.freeze_period;
        auto time = time_get();
        ds_assert(proposal.proposal_type == 1, "proposal_type % is unsupported", proposal.proposal_type);
        ds_assert(proposal.expires_at <= time, "proposal % did not expire, need to wait %.",
                  proposal_name, proposal.expires_at);
        ds_assert(proposal.expires_at + freeze_period > time,
                  "too late for apply proposal % freeze_period ended at %[%].",
                  proposal_name, proposal.expires_at + freeze_period, time);

        ds_int count_all, count_yes, count_no;
        ds_asset quantity_all, quantity_yes, quantity_no;
        std::map <std::string, ds_asset> vote_producers;
        vote_count(proposal_name,
                   count_all, count_yes, count_no,
                   quantity_all, quantity_yes, quantity_no, vote_producers);

        ds_assert(count_all > 0, "proposal % does not have any vote", proposal_name);

        auto total_nut_supply = supply_get(UTILITY_SYMBOL);
        ds_assert(total_nut_supply.amount > 0, "need to issue some nuts.");
        auto min_participation = to_ldouble(quantity_all) / to_double(total_nut_supply);
        ds_assert(min_participation >= settings.min_participation - 0.0001,
                  "min_participation (%) less than necessary % for proposal %.",
                  min_participation, settings.min_participation, proposal_name);


        auto success_margin = (long double) quantity_yes.amount / (long double) quantity_all.amount;
        ds_assert(success_margin >= settings.success_margin - 0.0001,
                  "success_margin (%) less than necessary % for proposal %.",
                  success_margin, settings.success_margin, proposal_name);
        auto json = proposal.proposal_json;
        auto cntract_types = getposcntr(settings.position_account);
        for (auto item:cntract_types) {
            auto ctr = item.first;
            auto ctr_name = ctr.to_string().c_str();
            auto type = item.second;
            if (json.find(ctr_name) != ds_string::npos) {
                if (type == EOSDTCNTRACT) {
                    apply_ctrsetting(json, cntract_types, ctr, ctr_name);
                } else if (type == EOSDTLIQDATR) {
                    apply_liqsetting(json, cntract_types, ctr, ctr_name);
                } else if (type == EOSDTORCLIZE) {
                    apply_orasetting(json, cntract_types, ctr, ctr_name);
                } else if (type == EOSDTGOVERNC) {
                    apply_govsetting(json, cntract_types, ctr, ctr_name);
                }
            }
        }

        PRINT_FINISHED("apply"_n)
    }

    ds_checksum eosdtgovernc::calc_accounts_hash(std::vector <ds_account> &accounts) {
        auto n = accounts.size();
        constexpr size_t size = sizeof(ds_account);

        std::vector<char> chars_to_hash(n * size, '\0');

        int i = 0;
        for (auto acc : accounts) {
            auto value = acc.value;
            char value_chars[size];
            std::memcpy(value_chars, &value, size);

            for (int j = 0; j < size; j++) {
                chars_to_hash[i] = value_chars[j];
                i++;
            }
        }

        auto hash = eosio::sha256(chars_to_hash.data(), chars_to_hash.size());
        return hash;
    }

    void eosdtgovernc::save_vote_hash(eosio::checksum256 hash) {
        auto time = time_get();

        auto set = [&](auto &row) {
            row.id = 0;
            row.voted_at = time;
            row.vote_hash = hash;
        };

        votingstatus_table votingstatus(_self, _self.value);
        auto itr = votingstatus.find(0);
        if (itr == votingstatus.end()) {
            votingstatus.emplace(_self, set);
        } else {
            votingstatus.modify(itr, _self, set);
        }
    }

    void eosdtgovernc::applybpproxy() {
        PRINT_STARTED("applybpproxy"_n)

        dobilling();

        govbpvotes_table govbpvotes(_self, _self.value);
        ds_print("\r\nfill vector");
        std::vector <govbpvote> byvotes;
        for (auto itr = govbpvotes.begin(); itr != govbpvotes.end(); itr++) {
            byvotes.push_back(*itr);
        }
        if (byvotes.size() == 0) {
            PRINT_FINISHED("applybpproxy"_n)
            return;
        }
        std::sort(byvotes.begin(), byvotes.end(),
                  [](const govbpvote &a, const govbpvote &b) {
                      return a.votes > b.votes;
                  });
        ds_print("\r\ndone vector");

        auto index = govbpvotes.get_index<"byvotes"_n>();
        auto itr = index.rbegin();

        auto max_bp_count = govsetting_get().max_bp_count;

        std::vector <ds_account> top_producers;
        auto cnt = 0;
        govbpparams_table govbpparams(_self, _self.value);

        for (auto itr = byvotes.begin(); itr != byvotes.end() && cnt < max_bp_count; itr++) {
            cnt++;
            if (itr->votes.amount <= 0l) {
                break;
            }
            auto par_itr = govbpparams.find(itr->producer.value);
            auto is_found = par_itr != govbpparams.end();
            if (is_found && par_itr->enabled) {
                top_producers.push_back(ds_account{itr->producer});
                ds_print("\r\nadding bp: %, votes: %", itr->producer, itr->votes);
            }
        }

        ds_print("\r\ntop_producers size: %", top_producers.size());
        if (top_producers.size() == 0) {
            top_producers.push_back(ds_account{byvotes.begin()->producer});
            ds_print("\r\nadding bp: %, top one", byvotes.begin()->producer);
        }

        std::sort(top_producers.begin(), top_producers.end());

        bool voting_needed = false;
        votingstatus_table votingstatus(_self, _self.value);
        auto vs_itr = votingstatus.find(0);

        ds_checksum current_hash = calc_accounts_hash(top_producers);

        if (vs_itr == votingstatus.end()) {
            ds_print("\r\nprev checksum is not found");
            voting_needed = true;
        } else {
            ds_checksum prev_hash = vs_itr->vote_hash;
            voting_needed = prev_hash != current_hash;
            if (voting_needed) {
                ds_print("\r\nchecksum is changed");
            }
        }

        if (voting_needed) {
            save_vote_hash(current_hash);

            updatebilling(std::set<ds_account>(top_producers.begin(), top_producers.end()));

            auto eosdtbpproxy_account = ctrsetting_get().bpproxy_account;
            auto proxy = ds_account(0);
            eosio::action(
                    eosio::permission_level{eosdtbpproxy_account, "active"_n},
                    "eosio"_n,
                    "voteproducer"_n,
                    std::make_tuple(eosdtbpproxy_account, proxy, top_producers)).send();
        } else {
            updatebilling(std::set<ds_account>(top_producers.begin(), top_producers.end()));
        }
        PRINT_FINISHED("applybpproxy"_n)
    }
}
