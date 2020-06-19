#include "eosdtorclize.hpp"

//read_action_data
#undef ORACLE_HOOK_FILTER
#define ORACLE_HOOK_FILTER(uri, data)\
auto _self = current_receiver(); \
eosdt::eosdtorclize cntract(name(_self),name(_self),datastream<const char*> ("",0));\
cntract.filter(uri, data);

#include "../dappservices/oracle.hpp"
#include "../dappservices/cron.hpp"



namespace eosdt {

#define DAPPSERVICES_ACTIONS() \
      XSIGNAL_DAPPSERVICE_ACTION \
      ORACLE_DAPPSERVICE_ACTIONS
#define DAPPSERVICE_ACTIONS_COMMANDS() \
      ORACLE_SVC_COMMANDS()

#define CONTRACT_NAME() eosdtorclize

    void eosdtorclize::filter(std::vector<char> uri,std::vector<char> data) {
        PRINT_STARTED("filter")
        auto time = time_get();
        auto settings = orasetting_get();
        auto validity_timeout = settings.validity_timeout;
        auto query = ds_string(uri.begin(), uri.end());
        auto pos = query.find("https+json");
        if (pos != std::string::npos) {
            query = query.substr(pos);
        }
        auto result = ds_string(data.begin(), data.end());
        ds_print("\r\ntime: '%', validity_timeout: '%', uri: '%', data: '%'",
                 time, validity_timeout, query, result);
        oraqueries_table oraclqueries(_self, _self.value);
        for (auto itr = oraclqueries.begin(); itr != oraclqueries.end(); itr++) {
            if (itr->query == query) {
                auto new_rate = parse_price(itr->asset_symbol, result.c_str());
                new_rate = ratebyprtype(i_to_price_type(itr->price_type), new_rate);
                auto filter = itr->filter;
                orarates_table orarates(_self, _self.value);
                auto index = orarates.template get_index<"ratebase"_n>();
                auto rate_itr = index.find(compress_key(itr->asset_symbol.code().raw(), itr->base.code().raw()));
                if (rate_itr != index.end()) {
                    auto rate = rate_itr->equilibriumdsp_price;
                    ds_print("\r\nrate: '%', new: '%'", rate, new_rate);
                    if (rate.amount == 0) {
                        break;
                    }

                    auto left = op_mul_ceil(rate, 1.0 - filter);
                    auto right = op_mul_floor(rate, 1.0 + filter);
                    auto coridor_is_good = left < new_rate && new_rate < right;
                    ds_print("\r\nleft: '%', right: '%', coridor_is_good: '%'",
                             left, right, coridor_is_good ? "Yes" : "No");
                    if (!coridor_is_good) {
                        break;
                    }

                    auto update = rate_itr->equilibriumdsp_update;
                    auto time_diff = (time - update).to_seconds();
                    auto time_out_is_good = time_diff < validity_timeout;
                    ds_print("\r\nupdate: '%', time_diff: '%', time_out_is_good: '%'",
                             update, time_diff, time_out_is_good ? "Yes" : "No");
                    if (!time_out_is_good) {
                        break;
                    }

                    ds_assert(false, "{abort_service_request}");
                }
                break;
            }
        }
        PRINT_FINISHED("filter")
    }

    void eosdtorclize::xgeturi(const ds_account &current_provider, const ds_account &package, const ds_uint &size,
                               const std::vector<char> &uri, const std::vector<char> &data) {
        require_auth(current_provider);

        oracle_svc_helper svc_helper;
        svc_helper._oracle_geturi(size, uri, data, current_provider);
        oracle_svc_helper::sgeturi signal;
        if (current_provider != "eosdtproduce"_n) {
            svc_helper.signal_svc("oracleservic"_n, current_provider, package, signal);
        }
        PRINT_FINISHED("xgeturi")
    }

    void eosdtorclize::xorcclean(const ds_account &current_provider, const ds_account &package, const ds_uint &size,
                                 const std::vector<char> &uri) {
        require_auth(current_provider);

        oracle_svc_helper svc_helper;
        svc_helper._oracle_orcclean(size, uri, current_provider);
        oracle_svc_helper::sorcclean signal;
        if (current_provider != "eosdtproduce"_n) {
            svc_helper.signal_svc("oracleservic"_n, current_provider, package, signal);
        }
        PRINT_FINISHED("xorcclean")
    }

    void eosdtorclize::xsignal(const ds_account &service, const ds_account &action, const ds_account &provider,
                               const ds_account &package,
                               const std::vector<char> &signalRawData) {
        require_auth(_self);

        oracle_svc_helper svc_helper;
        require_recipient(DAPPSERVICES_CONTRACT);
        PRINT_FINISHED("xsignal")
    }

    static void timer_callback(name timer, std::vector<char> payload, uint32_t seconds) {
    }

    struct cron_svc_helper {
        //CRON_DAPPSERVICE_ACTIONS
    };


    void eosdtorclize::xschedule(const ds_account &current_provider, const ds_account &package, const ds_account &timer,
                                 const std::vector<char> &payload, ds_uint seconds) {
        require_auth(current_provider);
        vector<char> result;
        ds_symbol symbol;
        price_type price_type;

        if (current_provider != _self) {
            oraqueries_table oraqueries(_self, _self.value);
            for (auto itr = oraqueries.begin(); itr != oraqueries.end(); itr++) {
                if (itr->source_contract != "heliosselene"_n
                    && itr->source_contract != "eosdtdspnode"_n
                    && itr->source_contract != "eospherexdsp"_n){
                    continue;
                }
                if (itr->asset_symbol.raw() != timer.value) {
                    continue;
                }
                price_type = i_to_price_type(itr->price_type);
                symbol = itr->asset_symbol;
                std::vector<char> uri(itr->query.begin(), itr->query.end());
                result = oracle_svc_helper::getURI(uri, [&](auto &results) {
                    return results[0].result;
                });
            }
        }

        auto settings = orasetting_get();
        auto interval = settings.equilibriumdsp_interval;
        seconds = interval - time_get().sec_since_epoch() % interval;
        cron_svc_helper cron_svc_helper;
        //cron_svc_helper.schedule_timer(timer, payload, seconds);

        if (!result.empty()) {
            ds_string result_str(result.begin(), result.end());
            rate_set(symbol, source_type::equilibriumdsp, price_type, result_str);
            ds_print("\r\nsymbol: % result: %", symbol, result);
        }
        PRINT_FINISHED("xschedule")
    }

    void eosdtorclize::orarecover() {
        require_auth(_self);
        oraqueries_table oraqueries(_self, _self.value);
        for (auto itr = oraqueries.begin(); itr != oraqueries.end(); itr++) {
            if (itr->source_contract != "heliosselene"_n
                && itr->source_contract != "eosdtdspnode"_n
                && itr->source_contract != "eospherexdsp"_n){
                continue;
            }
            std::vector<char> uri(itr->query.begin(), itr->query.end());
            auto result = oracle_svc_helper::getURI(uri, [&](auto &results) {
                return results[0].result;
            });
            std::vector<char> payload;
            eosio::action(
                    eosio::permission_level{_self, "active"_n},
                    _self,
                    "xschedule"_n,
                    std::make_tuple(_self.value, "package"_n, itr->asset_symbol.raw(), payload, 0U)
            ).send();
        }
        PRINT_FINISHED("orarecover")
    }

    void eosdtorclize::liquidrefres(const ds_symbol &symbol, const ds_symbol &base) {
        oraqueries_table oraqueries(_self, _self.value);
        for (auto itr = oraqueries.begin(); itr != oraqueries.end(); itr++) {
            if (itr->source_contract != "heliosselene"_n
                && itr->source_contract != "eosdtdspnode"_n
                && itr->source_contract != "eospherexdsp"_n){
                continue;
            }
            if (itr->asset_symbol != symbol || itr->base != base) {
                continue;
            }
            std::vector<char> uri(itr->query.begin(), itr->query.end());
            auto result = oracle_svc_helper::getURI(uri, [&](auto &results) {
                return results[0].result;
            });
            ds_string result_str(result.begin(), result.end());
            rate_set(symbol, source_type::equilibriumdsp, i_to_price_type(itr->price_type), itr->base, result_str);
            ds_print("\r\nsymbol: % result: %", symbol, result);
            break;
        }
        PRINT_FINISHED("liquidrefres")
    }
}
