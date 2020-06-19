#pragma once

#include "types.hpp"
#include <cmath>

using eosio::check;
using eosio::print_f;

static bool ne(double l, double r) {
    return l < r || r < l;
}

constexpr double to_double(const ds_asset &asset) {
    auto result = 1;
    for (ds_ulong i = 0; i < asset.symbol.precision(); i++)result *= 10;
    return (double) asset.amount / result;
}

constexpr long double to_ldouble(const ds_asset &asset) {
    auto result = 1;
    for (ds_ulong i = 0; i < asset.symbol.precision(); i++)result *= 10;
    return (long double) asset.amount / result;
}

static __int128_t pow(__int128_t x, ds_uint n) {
    if (n == 0)
        return 1;
    __int128_t y = 1;
    while (n > 1) {
        if (n % 2 == 0) {
            x *= x;
            n /= 2;
        } else {
            y = x * y;
            x *= x;
            n = (n - 1) / 2;
        }
    }
    return x * y;
}

static double pow(double x, ds_long n) {
    if (n == 0)
        return 1;
    double y = 1;
    while (n > 1) {
        if (n % 2 == 0) {
            x *= x;
            n /= 2;
        } else {
            y = x * y;
            x *= x;
            n = (n - 1) / 2;
        }
    }
    return x * y;
}

constexpr long double pow(long double x, ds_long n) {
    if (n == 0)
        return 1;
    long double y = 1.0L;
    while (n > 1) {
        if (n % 2 == 0) {
            x *= x;
            n /= 2;
        } else {
            y = x * y;
            x *= x;
            n = (n - 1) / 2;
        }
    }
    return x * y;
}

ds_asset op_mul_ceil(const ds_asset &quantity, const double value, ds_uint decimals = 1) {
    return ds_asset(std::ceil((double) quantity.amount * value / (double) decimals) * decimals, quantity.symbol);
}

ds_asset op_mul_floor(const ds_asset &quantity, const double value, ds_uint decimals = 1) {
    return ds_asset(std::floor((double) quantity.amount * value / (double) decimals) * decimals, quantity.symbol);
}

ds_asset op_div_ceil(const ds_asset &quantity, const double value, ds_uint decimals = 1) {
    return ds_asset(std::ceil((double) quantity.amount / (value * (double) decimals)) * decimals, quantity.symbol);
}

ds_asset op_div_floor(const ds_asset &quantity, const double value, ds_uint decimals = 1) {
    return ds_asset(std::floor((double) quantity.amount / (value * (double) decimals)) * decimals, quantity.symbol);
}

ds_asset op_mul_round(const ds_asset &quantity, const double value, ds_uint decimals = 1) {
    return ds_asset(std::llround((double) quantity.amount * value / (double) decimals) * decimals, quantity.symbol);
}

bool is_rounded(const ds_asset &value, int to = 4) {
    auto digits = value.symbol.precision() - to;
    auto rounded = value.amount;
    for (int i = 0; i < digits; i++) {
        rounded /= 10;
    }
    for (int i = 0; i < digits; i++) {
        rounded *= 10;
    }
    return value.amount == rounded;
}

static auto equal(ds_checksum l, ds_checksum r) {
    auto lhash = l.extract_as_byte_array();
    auto rhash = r.extract_as_byte_array();
    for (auto i = 0; i < 32; i++) {
        if (lhash[i] != rhash[i]) {
            return false;
        }
    }
    return true;
}

static char *write_value(char *r, const char *l, ds_ulong v) {
    r++;
    for (auto m = v / 10; m; m /= 10)r++;
    if (r > l) {
        return r;
    }
    auto f = r;
    do {
        *(--r) = v % 10 + 48;
        v /= 10;
    } while (v);
    return f;
}

static char *write_value(char *r, const char *l, ds_long v) {
    if (v < 0) {
        *(r++) = '-';
        v = -v;
    }
    return write_value(r, l, (ds_ulong) v);
}

static char *write_value(char *r, const char *l, ds_uint v) {
    return write_value(r, l, (ds_ulong) v);
}

static char *write_value(char *r, const char *l, ds_int v) {
    return write_value(r, l, (ds_long) v);
}

static char *write_value(char *r, const char *l, const uint128_t &v) {
    if (r + 32 > l) {
        return r;
    }
    for (auto i = 0; i < 16; i++) {
        auto b = (v >> (120 - (i * 8))) & 255;
        auto f = b >> 4;
        *(r++) = char(f + (f < 10 ? 48 : 55));
        auto s = b % 16;
        *(r++) = char(s + (s < 10 ? 48 : 55));
    }
    return r;
}

static char *write_value(char *r, const char *l, const eosio::checksum256 &v) {
    if (r + 64 > l) {
        return r;
    }
    auto vhash = v.extract_as_byte_array();
    for (auto i = 0; i < 32; i++) {
        auto b = vhash[i];
        auto f = b >> 4;
        *(r++) = char(f + (f < 10 ? 48 : 55));
        auto s = b % 16;
        *(r++) = char(s + (s < 10 ? 48 : 55));
    }
    return r;
}

static char *write_ulong(char *r, const char *l, ds_ulong v, ds_ulong fix_len) {
    auto s = r;
    r += fix_len;
    if (r > l) return s;
    for (; r > s; v /= 10) {
        *(--r) = v % 10 + 48;
    }
    return r + fix_len;
}

static char *write_value(char *r, const char *l, const double v) {
    r = write_value(r, l, (ds_long) v);
    auto m = v > 0 ? v : -v;
    m -= (ds_ulong) m;
    if (r + 1 > l || m < 0.0000000000000001) {
        return r;
    }
    *(r++) = '.';
    if (m >= 0.0001) {
        //return write_ulong(r, l, (ds_ulong) (m * 10000 + 0.9999), 4ULL);
    }
    if (m >= 0.00000001) {
        //return write_ulong(r, l, (ds_ulong) (m * 100000000 + 0.99999999), 8ULL);
    }
    return write_ulong(r, l, (ds_ulong) (m * 10000000000000000 + 0.9999999999999999), 16ULL);
}

static char *write_value(char *r, const char *l, const long double v) {
    return write_value(r, l, (double) v);
}

static char *write_symbol_name(char *r, const char *l, const eosio::symbol_code &sc) {
    auto v = sc.raw();
    if (r + 7 > l)return r;
    for (; v > 0; v >>= 8)
        *(r++) = (char) v;
    return r;
}

static char *write_value(char *r, const char *l, const ds_symbol &v) {
    r = write_value(r, l, (ds_ulong) v.precision());
    if (r + 1 > l) {
        return r;
    }
    *(r++) = ',';
    return write_symbol_name(r, l, v.code());
}

static char *write_value(char *r, const char *l, ds_asset v) {
    auto digits = 1;
    for (ds_ulong i = 0; i < v.symbol.precision(); i++) digits *= 10;
    if (v.amount / digits == 0 && v.amount < 0) {
        *(r++) = '-';
    }
    r = write_value(r, l, (ds_long) v.amount / digits);
    if (r + 1 > l) {
        return r;
    }
    *(r++) = '.';
    auto m = v.amount > 0 ? v.amount : -v.amount;
    m %= digits;
    r = write_ulong(r, l, (ds_ulong) m, v.symbol.precision());
    *(r++) = ' ';
    return write_symbol_name(r, l, v.symbol.code());
}

static inline bool mm(ds_int *days, ds_uint *month, const ds_uint m, const ds_int count) {
    if (*days < count) {
        *month = m;
        return false;
    }
    *days -= count;
    return true;
}

static char *write_value(char *r, const char *l, const ds_time v) {
    if (r + 15 > l) {
        return r;
    }
    auto utc = v.utc_seconds;
    ds_int days = utc / 86400;
    auto years = 1970 + (days - (days + 730) / 1461) / 365;
    r = write_ulong(r, l, (ds_ulong) years, 4ULL);
    days -= days / 365 * 365 + (days + 365) / 1461;
    if (days < 0) {
        days += 365;
    }
    ds_uint month;
    mm(&days, &month, 1, 31) && mm(&days, &month, 2, years % 4 == 0 ? 29 : 28) && mm(&days, &month, 3, 31) &&
    mm(&days, &month, 4, 30) && mm(&days, &month, 5, 31) && mm(&days, &month, 6, 30) &&
    mm(&days, &month, 7, 31) && mm(&days, &month, 8, 31) && mm(&days, &month, 9, 30) &&
    mm(&days, &month, 10, 31) && mm(&days, &month, 11, 30) && mm(&days, &month, 12, 31);
    r = write_ulong(r, l, (ds_ulong) month, 2ULL);
    r = write_ulong(r, l, (ds_ulong) days + 1, 2ULL);
    *(r++) = ' ';
    r = write_ulong(r, l, (ds_ulong) (utc % 86400 / 3600), 2ULL);
    *(r++) = ':';
    r = write_ulong(r, l, (ds_ulong) (utc % 3600 / 60), 2ULL);
    *(r++) = ':';
    r = write_ulong(r, l, (ds_ulong) (utc % 60), 2ULL);
    return r;
}


static char *write_value(char *r, const char *l, const char *v) {
    while (*v && r < l) *(r++) = *(v++);
    return r;
}

static char *write_value(char *r, const char *l, const std::vector<char> &v) {
    for (auto it = v.begin(); it != v.end() && r < l; it++) *(r++) = *it;
    return r;
}

static char *write_value(char *r, const char *l, const ds_string &v) {
    return write_value(r, l, v.c_str());
}

static char *write_value(char *r, const char *l, const ds_account &a) {
    const ds_ulong v = a.value;
    static const char *char_map = ".12345abcdefghijklmnopqrstuvwxyz";
    if (r + 13 > l) return r;
    for (ds_uint i = 0; i < 60; i += 5) {
        auto c = v >> (59 - i) & 31;
        *(r++) = char_map[c];
    }
    while (*(r - 1) == '.')r--;
    return r;
}

template<typename t>
static char *write_value(char *r, const char *l, const std::vector <t> &v) {
    *(r++) = '[';
    for (auto item: v) {
        r = write_value(r, l, item);
        *(r++) = ',';
    }
    *(r++) = ']';
    return r;
}

template<typename t>
static char *write_value(char *r, const char *l, const std::optional <t> &v) {
    return write_value(r, l, *v);
}


template<typename t0 = ds_nullptr, typename t1 = ds_nullptr, typename t2 = ds_nullptr,
        typename t3 = ds_nullptr, typename t4 = ds_nullptr, typename t5 = ds_nullptr,
        typename t6 = ds_nullptr, typename t7 = ds_nullptr, typename t8 = ds_nullptr, typename t9 = ds_nullptr>
static char *write_format_string(const char *f, t0 v0 = nullptr, t1 v1 = nullptr, t2 v2 = nullptr,
                     t3 v3 = nullptr, t4 v4 = nullptr, t5 v5 = nullptr,
                     t6 v6 = nullptr, t7 v7 = nullptr, t8 v8 = nullptr, t9 v9 = nullptr) {
    ds_uint a = 0;
    char chars[500];
    char *r = chars;
    const char *l = r + 500;
    while (*f && r < l) {
        if (*f != '%') {
            *(r++) = *(f++);
            continue;
        }
        f++;
        switch (a++) {
            case 0:
                r = write_value(r, l, v0);
                break;
            case 1:
                r = write_value(r, l, v1);
                break;
            case 2:
                r = write_value(r, l, v2);
                break;
            case 3:
                r = write_value(r, l, v3);
                break;
            case 4:
                r = write_value(r, l, v4);
                break;
            case 5:
                r = write_value(r, l, v5);
                break;
            case 6:
                r = write_value(r, l, v6);
                break;
            case 7:
                r = write_value(r, l, v7);
                break;
            case 8:
                r = write_value(r, l, v8);
                break;
            case 9:
                r = write_value(r, l, v9);
                break;
        }
    }
    *r = '\0';
    r = chars;
    return r;
}


template<typename t0 = ds_nullptr, typename t1 = ds_nullptr, typename t2 = ds_nullptr,
        typename t3 = ds_nullptr, typename t4 = ds_nullptr, typename t5 = ds_nullptr,
        typename t6 = ds_nullptr, typename t7 = ds_nullptr, typename t8 = ds_nullptr, typename t9 = ds_nullptr>
static void ds_print(const char *f, t0 v0 = nullptr, t1 v1 = nullptr, t2 v2 = nullptr,
                     t3 v3 = nullptr, t4 v4 = nullptr, t5 v5 = nullptr,
                     t6 v6 = nullptr, t7 v7 = nullptr, t8 v8 = nullptr, t9 v9 = nullptr) {
    print_f(write_format_string(f, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9));
}


template<typename t0 = ds_nullptr, typename t1 = ds_nullptr, typename t2 = ds_nullptr,
        typename t3 = ds_nullptr, typename t4 = ds_nullptr, typename t5 = ds_nullptr,
        typename t6 = ds_nullptr, typename t7 = ds_nullptr, typename t8 = ds_nullptr, typename t9 = ds_nullptr>
static void ds_assert(bool test, const char *f, t0 v0 = nullptr, t1 v1 = nullptr, t2 v2 = nullptr,
                      t3 v3 = nullptr, t4 v4 = nullptr, t5 v5 = nullptr,
                      t6 v6 = nullptr, t7 v7 = nullptr, t8 v8 = nullptr, t9 v9 = nullptr) {
    if (test) {
        return;
    }
    check(false, write_format_string(f, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9));
}


ds_asset op_mul_div(const ds_asset &quantity, const long double mul, const long double div) {
    auto value = int64_t(((long double) quantity.amount) * mul / div);
    ds_assert(value <= 4611686018427387904L,
              "magnitude of asset amount must be less than 2^62\r\namount(%)=quantity(%)*mul(%)/div(%)",
              value, quantity, mul, div);
    return ds_asset(int64_t(((long double) quantity.amount) * mul / div), quantity.symbol);
}

static ds_asset try_parse_price(const ds_symbol &symbol, const char *data) {
    int64_t mint = 0;
    bool decimals = false;
    int32_t digits = 0;
    while (*data && *data != ' ') {
        if (*data >= '0' && *data <= '9') {
            if (decimals) digits++;
            mint = (int64_t)(*data) - (int64_t)('0') + mint * 10;
        } else if (*data == '.' && !decimals) {
            decimals = true;
        } else {
            ds_assert(false, "wrong price format.");
        }
        data++;
    }
    auto diff = (int32_t) symbol.precision() - digits;
    if (diff < 0) {
        ds_assert(false, "wrong price decimals.");
    }
    for (int32_t i = 0; i < diff; i++) mint *= 10;
    return ds_asset(mint, symbol);
}

uint64_t try_to_uint64(char const *s) {
    uint64_t result = 0;
    auto found = false;
    auto c = s;
    while (*c!=0) {
        ds_assert(*c >= '0' && *c <= '9',"Wrong uint64 value: '%'['%'].", s,*c);
        result = result * 10 + (uint64_t)(*c - '0');
        c++;
        found = true;
    }
    ds_assert(found,"Wrong uint64 value: '%'.", s);
    return result;
}
