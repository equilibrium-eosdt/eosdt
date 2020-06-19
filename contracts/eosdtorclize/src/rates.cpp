#include "eosdtorclize.hpp"

namespace eosdt {

    auto eosdtorclize::parse_price(const ds_symbol &symbol, const char *data) {
        int64_t mint = 0;
        bool decimals = false;
        int32_t digits = 0;
        while (*data && *data != ',' && *data != '}' && *data != ' ') {
            if (*data >= '0' && *data <= '9') {
                if (decimals) {
                    if (digits++ == (int32_t) symbol.precision()) {
                        break;
                    }
                }
                mint = (int64_t) * data - '0' + mint * 10;
            } else if (*data == '.' && !decimals) {
                decimals = true;
            } else {
                return ds_asset(0, symbol);
            }
            data++;
        }
        auto diff = (int32_t) symbol.precision() - digits;
        if (diff > 0) {
            for (int32_t i = 0; i < diff; i++)mint *= 10;
        }
        return ds_asset(mint, symbol);
    }

    auto eosdtorclize::parse_rate(const ds_symbol &token_symbol, const ds_string &data) {
        char buffer[8];
        buffer[0] = '"';
        auto b = write_symbol_name(buffer + 1, buffer + 8U, token_symbol.code());
        *(b++) = '"';
        *(b++) = ':';
        auto pos = data.find(buffer, 0, (uint32_t)(b - buffer));
        if (pos == ds_string::npos) {
            return ds_asset(0, token_symbol);
        }
        return parse_price(token_symbol, data.c_str() + pos + (b - buffer));
    }

    auto eosdtorclize::get_usd_rate() {
        PRINT_STARTED("getusdrate"_n)

        auto base = EOS_SYMBOL;
        auto symbol = USD_SYMBOL;
        orarates_table orarates(_self, _self.value);
        auto index = orarates.template get_index<"ratebase"_n>();
        auto rate_itr = index.find(compress_key(symbol.code().raw(), base.code().raw()));
        ds_assert(rate_itr != index.end(), "rates does not exist for symbol: '%'.", symbol);

        auto curr_now = time_get();
        auto elapsed = (curr_now - rate_itr->update).to_seconds();
        ds_assert(elapsed >= 0, "last_update for % is in the future", symbol);
        ds_print("\r\nelapsed: %", elapsed);
        auto orasettings = orasetting_get();
        ds_assert(elapsed < orasettings.rate_timeout, "USD rate is out of date");

        PRINT_FINISHED("getusdrate"_n)
        return rate_itr->rate;
    }


    void eosdtorclize::set_median(const ds_time &threshold, orarate &orarate) {
        std::vector <ds_asset> rates;
        std::vector <ds_time> times;
        times.push_back(orarate.update);
        if (orarate.provablecb1a_update >= threshold) {
            rates.push_back(orarate.provablecb1a_price);
            times.push_back(orarate.provablecb1a_update);
        }
        if (orarate.delphioracle_update >= threshold) {
            rates.push_back(orarate.delphioracle_price);
            times.push_back(orarate.delphioracle_update);
        }
        if (orarate.equilibriumdsp_update >= threshold) {
            rates.push_back(orarate.equilibriumdsp_price);
            times.push_back(orarate.equilibriumdsp_update);
        }
        ds_assert(rates.size() > 0, "rate % is wrong.", orarate.rate);
        std::sort(rates.begin(), rates.end());

        auto l = (rates.size() - 1) / 2;
        auto r = rates.size() / 2;
        orarate.rate = ds_asset((rates[l] + rates[r]).amount / 2, rates[l].symbol);
        ds_assert(orarate.rate.amount > 0, "rate % is equal zero.", orarate.rate);
        orarate.update = *std::min_element(times.begin(), times.end());
    }
    void eosdtorclize::rate_set(const ds_symbol &token_symbol, const source_type &source, const price_type &price_type,
                                const ds_string &data) {
        rate_set(source,price_type,parse_price(token_symbol, data.c_str()));
    }
    void eosdtorclize::rate_set(const ds_symbol &token_symbol, const source_type &source, const price_type &price_type,
                                const ds_symbol &base, const ds_string &data) {
        rate_set(source,price_type,base,parse_price(token_symbol, data.c_str()));
    }
    void eosdtorclize::rate_set(const source_type &source, const price_type &price_type, const ds_asset &data) {
        rate_set(source, price_type, EOS_SYMBOL, data);
    }

    auto eosdtorclize::ratebyprtype(const price_type &price_type, const ds_asset &data)
    {
        PRINT_STARTED("ratebyprtype"_n)
        auto rate = data;
        switch (price_type) {
            case price_type::SYMBOL_TO_EOS:
                if(data.amount!=0) {
                    rate = ds_asset(pow(10.0, data.symbol.precision()) / to_ldouble(data), data.symbol);
                }
                break;
            case price_type::EOS_TO_SYMBOL:
                break;
            case price_type::SYMBOL_TO_USD: {
                auto usd_rate = get_usd_rate();
                ds_print("\r\nusd_rate: %, parsed: %", usd_rate, data);
                rate = ds_asset(to_ldouble(usd_rate) * to_ldouble(data) * pow(10.0, data.symbol.precision()),
                                data.symbol);
                break;
            }
            default:
                ds_assert(false, "Unsupported price type: %", price_type_to_i(price_type));
        }
        PRINT_FINISHED("ratebyprtype"_n)
        return rate;
    }


    void eosdtorclize::rate_set(const source_type &source, const price_type &price_type, const ds_symbol &base, const ds_asset &data) {
        PRINT_STARTED("rateset"_n)
        auto time = time_get();
        auto threshold = time - orasetting_get().rate_timeout;
        auto rate = ratebyprtype(price_type, data);
        ds_print("\r\nparsed: %, setting rate: %, base: %, price_type: %, threshold: %",
                data, rate, base, price_type_to_i(price_type), threshold);
        if (rate.amount <= 0) {
            ds_print("\r\nrateset hasn't succeded");
            return;
        }

        ds_ulong id;
        ds_asset median;
        auto set = [&](auto &row) {
            row.id = id;
            row.update = time;
            row.base = base;
            switch (source) {
                case source_type::provablecb1a:
                    row.provablecb1a_price = rate;
                    row.provablecb1a_update = row.update;
                    break;
                case source_type::delphioracle:
                    row.delphioracle_price = rate;
                    row.delphioracle_update = row.update;
                    break;
                case source_type::equilibriumdsp:
                    row.equilibriumdsp_price = rate;
                    row.equilibriumdsp_update = row.update;
                    break;
                default:
                    ds_assert(false, "Unsupported source: %", to_string(source));
            }
            set_median(threshold, row);
            median = row.rate;
        };
        orarates_table orarates(_self, _self.value);
        auto index = orarates.template get_index<"ratebase"_n>();
        auto rate_itr = index.find(compress_key(rate.symbol.code().raw(), base.code().raw()));
        if (rate_itr == index.end()) {
            id = orarates.available_primary_key();
            ds_print("\r\nemplace rate: % base: %", rate, base);
            orarates.emplace(_self, set);
        } else {
            id = rate_itr->id;
            ds_asset old_rate;
            switch (source) {
                case source_type::provablecb1a:
                    old_rate = rate_itr->provablecb1a_price;
                    break;
                case source_type::delphioracle:
                    old_rate = rate_itr->delphioracle_price;
                    break;
                case source_type::equilibriumdsp:
                    old_rate = rate_itr->equilibriumdsp_price;
                    break;
                default:
                    ds_assert(false, "Unsupported source: %", to_string(source));
            }
            ds_print("\r\nchange rate: % => %", old_rate, rate);
            index.modify(rate_itr, _self, set);
        }
        on_rate_changed(median.symbol, base);
        PRINT_FINISHED("rateset"_n)
    }


}