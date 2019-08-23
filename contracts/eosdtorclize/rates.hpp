#include "subscribers.hpp"


class rates : public subscribers {
public:
    rates(ds_account receiver, ds_account code, datastream<const char *> ds) : subscribers(receiver, code, ds) {
    }

protected:
    typedef multi_index<"orarates"_n, oracle_rate> oracle_rates_type;
    typedef const oracle_rates_type::const_iterator &oracle_rates_iterator;
private:
    void set_median(const ds_int &rate_timeout, oracle_rate &orarate)
    {
        auto time = orarate.update - rate_timeout;
        std::vector<ds_asset> rates;
        std::vector<ds_time> times;
        times.push_back(orarate.update);
        if(orarate.provablecb1a_update >= time)
        {
            rates.push_back(orarate.provablecb1a_price);
            times.push_back(orarate.provablecb1a_update);
        }
        if(orarate.eosnationdsp_update >= time)
        {
            rates.push_back(orarate.eosnationdsp_price);
            times.push_back(orarate.eosnationdsp_update);
        }
        if(orarate.equilibriumdsp_update >= time)
        {
            rates.push_back(orarate.equilibriumdsp_price);
            times.push_back(orarate.equilibriumdsp_update);
        }
        ds_assert(rates.size() > 0, "rate % is wrong.", orarate.rate);
        std::sort(rates.begin(),rates.end());

        auto l = (rates.size() - 1) / 2;
        auto r = rates.size() / 2;
        orarate.rate = ds_asset((rates[l] + rates[r]).amount / 2,rates[l].symbol);
        ds_assert(orarate.rate.amount > 0, "rate % is equal zero.", orarate.rate);
        orarate.update = *std::min_element(times.begin(),times.end());
    }
protected:
    auto get_usd_rate() {
        PRINT_STARTED("getusdrate"_n)

        auto symbol = USD_SYMBOL;
        oracle_rates_type oracle_rates(_self, _self.value);
        auto itr = oracle_rates.find(symbol.raw());
        ds_assert(itr != oracle_rates.end(), "rates does not exists for symbol: '%'.", symbol);

        auto curr_now = time_get();
        auto elapsed = (curr_now - itr->update).to_seconds();
        ds_assert(elapsed >= 0, "last_update for % is in the future", symbol);
        ds_print("\r\nelapsed: %", elapsed);
        auto settings = setting_get();
        ds_assert(elapsed < settings.rate_timeout, "USD rate is out of date");

        PRINT_FINISHED("getusdrate"_n)
        return itr->rate;
    }

    void rate_set(const ds_symbol &token_symbol, const source_type &source, const price_type &price_type, const ds_string &data) {
        PRINT_STARTED("rateset"_n)
        oracle_rates_type oracle_rates(_self, _self.value);
        auto parsed = parse_price(token_symbol, data.c_str());
        auto settings = setting_get();
        ds_asset rate;
        switch (price_type) {
            case price_type::SYMBOL_TO_EOS:
                rate = ds_asset(pow(10.0, token_symbol.precision()) / to_ldouble(parsed), token_symbol);
                break;
            case price_type::EOS_TO_SYMBOL:
                rate = parsed;
                break;
            case price_type::SYMBOL_TO_USD: {
                auto usd_rate = get_usd_rate();
                ds_print("\r\nusd_rate: %, parsed: %", usd_rate, parsed);
                rate = ds_asset(to_ldouble(usd_rate) * to_ldouble(parsed) * pow(10, token_symbol.precision()), token_symbol);
                break;
            }
            default:
                ds_assert(false, "Unsupported price type: %", price_type_to_i(price_type));
        }

        ds_print("\r\nparsed: %, setting rate: %", parsed, rate);
        if (rate.amount <= 0) {
            ds_print("\r\nrateset hasn't succeded");
            return;
        }

        auto set = [&](auto &row) {
            row.update = time_get();
            switch (source) {
                case source_type::provablecb1a:
                    row.provablecb1a_price = rate;
                    row.provablecb1a_update = row.update;
                    break;
                case source_type::eosnationdsp:
                    row.eosnationdsp_price = rate;
                    row.eosnationdsp_update = row.update;
                    break;
                case source_type::equilibriumdsp:
                    row.equilibriumdsp_price = rate;
                    row.equilibriumdsp_update = row.update;
                    break;
                default:
                    ds_assert(false, "Unsupported source: %", to_string(source));
            }
            set_median(settings.rate_timeout,row);
        };

        auto rate_itr = oracle_rates.find(token_symbol.raw());
        if (rate_itr == oracle_rates.end()) {
            ds_print("\r\nemplace rate: %", rate);
            oracle_rates.emplace(_self, set);
        } else {
            ds_asset old_rate;
            switch (source) {
                case source_type::provablecb1a:
                    old_rate = rate_itr-> provablecb1a_price;
                    break;
                case source_type::eosnationdsp:
                    old_rate = rate_itr-> eosnationdsp_price;
                    break;
                case source_type::equilibriumdsp:
                    old_rate = rate_itr-> equilibriumdsp_price;
                    break;
                default:
                    ds_assert(false, "Unsupported source: %", to_string(source));
            }
            ds_print("\r\nchange rate: % => %", old_rate, rate);
            oracle_rates.modify(rate_itr, _self, set);
        }
        on_rate_changed(rate_itr->update, rate);
        PRINT_FINISHED("rateset"_n)
    }
public:
};
