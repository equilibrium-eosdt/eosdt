#include "types.hpp"


static bool ne(double l, double r) {
    return l < r || r < l;
}

static double to_double(const ds_asset &asset) {
    auto result = 1;
    for (ds_ulong i = 0; i < asset.symbol.precision(); i++)result *= 10;
    return (double) asset.amount / result;
}

static long double to_ldouble(const ds_asset &asset) {
    auto result = 1;
    for (ds_ulong i = 0; i < asset.symbol.precision(); i++)result *= 10;
    return (long double) asset.amount / result;
}

ds_asset operator*( const ds_asset& quantity,const double value ) {
    return ds_asset(int64_t(((double)quantity.amount)*value), quantity.symbol);
}

ds_asset operator/( const ds_asset& quantity,const double value ) {
    return ds_asset(int64_t(((double)quantity.amount)/value), quantity.symbol);
}

ds_asset operator*( const ds_asset& quantity,const long double value ) {
    return ds_asset(int64_t(((long double)quantity.amount)*value), quantity.symbol);
}

ds_asset operator/( const ds_asset& quantity,const long double value ) {
    return ds_asset(int64_t(((long double)quantity.amount)/value), quantity.symbol);
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

static char* write_value(char *r, const char *l, const uint128_t &v) {
    if (r+32>l) {
        return r;
    }
    for (auto i = 0; i < 16; i++) {
        auto b = (v>>(120-(i*8)))&255;
        auto f = b >> 4;
        *(r++) = char(f + (f < 10 ? 48 : 55));
        auto s = b % 16;
        *(r++) = char(s + (s < 10 ? 48 : 55));
    }
    return r;
}

static char* write_value(char *r, const char *l, const eosio::checksum256 &v) {
    if (r+64>l) {
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
    if (r + 1 > l || m < 0.00000001) {
        return r;
    }
    *(r++) = '.';
    if (m >= 0.0001) {
        return write_ulong(r, l, (ds_ulong) (m * 10000 + 0.9999), 4ULL);
    }
    return write_ulong(r, l, (ds_ulong) (m * 100000000 + 0.99999999), 8ULL);
}

static char *write_value(char *r, const char *l, const long double v) {
    return write_value(r,l, (double)v);
}

static char *write_symbol_name(char *r, const char *l, const eosio::symbol_code &sc) {
    auto v = sc.raw();
    if (r + 7 > l)return r;
    for (; v > 0; v >>= 8)
        *(r++) = (char) v;
    return r;
}

static char *write_value(char *r, const char *l,const ds_symbol &v) {
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
    while(*(r-1)=='.')r--;
    return r;
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

    eosio::print(write_format_string(f, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9));
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
    eosio::check(false, write_format_string(f, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9));
}


static ds_asset parse_price(const ds_symbol &symbol, const char *data) {
    int64_t mint = 0;
    bool decimals = false;
    int32_t digits = 0;
    while (*data && *data != ',' && *data != '}' && *data != ' ') {
        if (*data > 47 && *data < 58) {
            if (decimals) {
                if (digits++ == (int32_t) symbol.precision()) {
                    break;
                }
            }
            mint =  (int64_t) * data - 48L + mint*10;
        } else if (*data == 46 && !decimals) {
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

static ds_asset parse_rate(const ds_symbol &token_symbol, const ds_string &data) {
    char buffer[8];
    buffer[0] = '"';
    auto b = write_symbol_name(buffer + 1, buffer+8U, token_symbol.code());
    *(b++) = '"';
    *(b++) = ':';
    auto pos = data.find(buffer, 0, (uint32_t)(b - buffer));
    if (pos == ds_string::npos) {
        return ds_asset(0, token_symbol);
    }
    return parse_price(token_symbol, data.c_str() + pos + (b - buffer));
}

ds_asset op_mul_div( const ds_asset& quantity, const long double mul, const long double div ) {
    auto value = int64_t(((long double)quantity.amount)*mul/div);
    ds_assert(value <= 4611686018427387904L, "magnitude of asset amount must be less than 2^62\r\namount(%)=quantity(%)*mul(%)/div(%)",
              value, quantity, mul, div);
    return ds_asset(int64_t(((long double)quantity.amount)*mul/div), quantity.symbol);
}
