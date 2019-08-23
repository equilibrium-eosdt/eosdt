#include "oracle.hpp"
#include "json_parser.hpp"

class balances : public oracle {

private:
    typedef eosio::multi_index<"accounts"_n, account> accounts_table;
    typedef eosio::multi_index<"stat"_n, currency_stats> currency_stats_table;

protected:
    typedef eosio::multi_index<"voters"_n, voter> voters_table;
    typedef eosio::multi_index<"votes"_n, vote,
            eosio::indexed_by<"byproposal"_n, eosio::const_mem_fun < vote, __uint128_t, &vote::by_proposal>>,
            eosio::indexed_by<"byvoter"_n, eosio::const_mem_fun<vote, __uint128_t, &vote::by_voter>>>
            votes_table;
    typedef eosio::multi_index<"bpvotes"_n, bpvote,
            eosio::indexed_by<"byvotes"_n, eosio::const_mem_fun < bpvote, uint64_t, &bpvote::byvotes>>> bpvotes_table;
    typedef const bpvotes_table::const_iterator &bpvotes_iterator;

protected:
    auto ctract_by_symbol(const ds_symbol &symbol) {
        if (symbol == UTILITY_SYMBOL) {
            return eosdt_ctract_setting_get().nutoken_account;
        }
        return EOSCTRACT;
    }

    auto supply_get(const ds_symbol &symbol)
    {
        auto id = symbol.code().raw();
        currency_stats_table currencies(ctract_by_symbol(symbol), id);
        auto itr = currencies.find(id);
        if (itr == currencies.end()) {
            return ds_asset(0ll, symbol);
        }
        return itr->supply;
    }

    auto balance_get(const ds_account &account, const ds_symbol &symbol) {
        accounts_table accounts(ctract_by_symbol(symbol), account.value);
        auto itr = accounts.find(symbol.code().raw());
        if (itr == accounts.end()) {
            return ds_asset(0ll, symbol);
        }
        return itr->balance;
    }

    ds_asset voting_amount_get(const ds_account &owner, const ds_symbol &sym) const {
        voters_table voters(_self, owner.value);
        const auto itr = voters.find(sym.code().raw());
        return itr == voters.end() ? ds_asset(0, sym) : itr->voting_amount;
    }

    json_parser get_json_parser_for_bpvotes(const char *json) {
        json_parser::k_v_map key_values;

        key_values.insert(json_parser::k_v_pair("eosdtbpproxy.producers", json_parser::VALUE_ARRAY));

        return json_parser(json, key_values);
    }

    void activate_bpvote_internal(const ds_account &voter) {
        auto voting_amount = voting_amount_get(voter, UTILITY_SYMBOL);
        votes_table votes(_self, _self.value);
        auto index = votes.template get_index<"byproposal"_n>();
        auto itr = index.find(compress_key(blockproduceproposal.value, voter.value));
        if (itr != index.end() and voting_amount.amount > 0) {
            bpvotes_table bpvote_tbl(_self, _self.value);
            std::vector <std::string> vote_producers;
            auto parser = get_json_parser_for_bpvotes(itr->vote_json.c_str());
            auto parse_status = parser.parse();
            for (; parse_status == json_parser::STATUS_OK; parse_status = parser.parse()) {
                if (parser.is_key_equals("eosdtbpproxy.producers")) {
                    vote_producers = parser.get_out_array();
                }
            }
            for (auto producer_name:vote_producers) {
                auto bpaccount = eosio::name{producer_name};
                auto bpitr = bpvote_tbl.find(bpaccount.value);
                if (bpitr == bpvote_tbl.end()) {
                    ds_print("\r\nactivate.emplace: account: %, amount: %.",bpaccount,voting_amount);
                    bpvote_tbl.emplace(_self, [&]( auto& b ) {
                        b.producer = bpaccount;
                        b.votes = voting_amount;
                    });
                } else {
                    ds_print("\r\nactivate.modify: account: %, amount: %.",bpaccount,voting_amount);
                    bpvote_tbl.modify(bpitr, _self, [&]( auto& b ) {
                        b.votes += voting_amount;
                    });
                }
            }
        }
    }
    void deactivate_bpvote_internal(const ds_account &voter) {
        auto voting_amount = voting_amount_get(voter, UTILITY_SYMBOL);
        votes_table votes(_self, _self.value);
        auto index = votes.template get_index<"byproposal"_n>();
        auto itr = index.find(compress_key(blockproduceproposal.value, voter.value));
        if (itr != index.end() and voting_amount.amount > 0) {
            bpvotes_table bpvote_tbl(_self, _self.value);
            std::vector <std::string> vote_producers;
            auto parser = get_json_parser_for_bpvotes(itr->vote_json.c_str());
            auto parse_status = parser.parse();
            for (; parse_status == json_parser::STATUS_OK; parse_status = parser.parse()) {
                if (parser.is_key_equals("eosdtbpproxy.producers")) {
                    vote_producers = parser.get_out_array();
                }
            }
            for (auto producer_name:vote_producers) {
                auto bpaccount = eosio::name{producer_name};
                auto bpitr = bpvote_tbl.find(bpaccount.value);
                if (bpitr != bpvote_tbl.end()) {
                    if (bpitr->votes == voting_amount) {
                        ds_print("\r\nactivate.erase: account: %, amount: %.",bpaccount,voting_amount);
                        bpvote_tbl.erase(bpitr);
                    } else {
                        ds_print("\r\nactivate.modify: account: %, amount: %.",bpaccount,voting_amount);
                        bpvote_tbl.modify(bpitr, _self, [&](auto &b) {
                            b.votes -= voting_amount;
                        });
                    }
                }
            }
        }
    }

    void voting_amount_sub(ds_account voter, ds_asset value) {
        voters_table voters(_self, voter.value);
        const auto &from = voters.get(value.symbol.code().raw(), "no balance object found");
        ds_assert(from.voting_amount >= value, "overdrawn balance");
        deactivate_bpvote_internal(voter);
        if (from.voting_amount == value) {
            voters.erase(from);
        } else {
            voters.modify(from, voter, [&](auto &a) {
                a.voting_amount -= value;
            });
            activate_bpvote_internal(voter);
        }
    }

    void voting_amount_add(ds_account voter, ds_asset value, ds_account ram_payer) {
        voters_table voters(_self, voter.value);
        auto to = voters.find(value.symbol.code().raw());
        auto time = time_get() + settings_get().unstake_period;
        if (to == voters.end()) {
            voters.emplace(ram_payer, [&](auto &a) {
                a.voting_amount = value;
                a.withdrawal_date = time;
            });
        } else {
            deactivate_bpvote_internal(voter);
            voters.modify(to, ds_account(0), [&](auto &a) {
                a.voting_amount += value;
                a.withdrawal_date = time;
            });
        }
        activate_bpvote_internal(voter);
    }

    void trans(const ds_account &to, const ds_asset &quantity, const ds_string &memo) {
        auto ctract = ctract_by_symbol(quantity.symbol);
        auto balance = balance_get(_self, quantity.symbol);
        ds_print("\r\ntrans: {from: %, to: %, quantity: % ,by: %, memo: '%', before: %, after: %}",
                 _self, to, quantity, ctract, memo, balance, balance - quantity);
        if (quantity.amount <= 0) {
            return;
        }
        eosio::action(
                eosio::permission_level{_self, "active"_n},
                ctract,
                "transfer"_n,
                std::make_tuple(_self, to, quantity, memo)
        ).send();
    }
public:
    balances(ds_account receiver, ds_account code, eosio::datastream<const char *> ds) :
            oracle(receiver, code, ds) {
    }


    void transfer(const ds_account &from,
                  const ds_account &to,
                  ds_asset &quantity,
                  const ds_string &memo) {
        PRINT_STARTED("transfer"_n)
        if (to != _self)
            return;
        auto settings = settings_get();
        ds_assert(quantity >= settings.min_vote_stake, "wrong amount % expected >= %", quantity, settings.min_vote_stake);
        auto ctract_for_symbol = ctract_by_symbol(quantity.symbol);
        ds_assert(ctract_for_symbol == _first_receiver, "wrong contract % for asset: % expected: %.", _first_receiver, quantity,
                  ctract_for_symbol);

        voting_amount_add(from, quantity, _self);
        PRINT_FINISHED("transfer"_n)
    }




};
