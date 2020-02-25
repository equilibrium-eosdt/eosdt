#pragma once

#include "defines.hpp"


namespace eosdt {

    enum class price_type : ds_int {
        SYMBOL_TO_EOS,
        EOS_TO_SYMBOL,
        SYMBOL_TO_USD
    };

    static ds_int price_type_to_i(const price_type price_type) {
        return static_cast<ds_int>(price_type);
    }

    static void assert_price_type(const ds_int num) {
        eosio::check(num >= static_cast<ds_int>(
                             price_type::SYMBOL_TO_EOS) && num <= static_cast<ds_int>(price_type::SYMBOL_TO_USD),
                     "price_type value out of range");
    }

    static price_type i_to_price_type(const ds_int num) {
        assert_price_type(num);

        return static_cast<price_type>(num);
    }

    enum class source_type : ds_int {
        provablecb1a,
        delphioracle,
        equilibriumdsp
    };

    auto to_string(const source_type &source) {
        switch (source) {
            case source_type::provablecb1a:
                return "provablecb1a";
            case source_type::delphioracle:
                return "delphioracle";
            case source_type::equilibriumdsp:
                return "equilibriumdsp";
            default:
                return std::to_string((ds_int) source).c_str();
        }
    }
}
