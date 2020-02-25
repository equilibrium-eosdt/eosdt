
class json_parser {

public:
    enum parse_status : uint8_t {
        STATUS_OK = 0,
        STATUS_END = 1,
        STATUS_ERR_NULL = 2,
        STATUS_ERR_CHAR = 3,
        STATUS_ERR_FMT = 4,
        STATUS_ERR_KEY = 5,
        STATUS_ERR_VALUE = 6,
    };

    enum parse_value : uint8_t {
        VALUE_NONE = 0,
        VALUE_STRING = 1,
        VALUE_NAME = 2,
        VALUE_DOUBLE = 3,
        VALUE_INT = 4,
        VALUE_ASSET = 5,
        VALUE_DATETIME = 6,
        VALUE_ARRAY = 7,
    };

    struct cmp_str {
        bool operator()(char const *a, char const *b) const {
            return std::strcmp(a, b) < 0;
        }
    };

    typedef std::map<const char *, parse_value, cmp_str> k_v_map;
    typedef std::pair<const char *, parse_value> k_v_pair;

private:
    const char *_json;
    k_v_map _key_values;
    parse_status _status;
    int _pos;
    int _key_pos;
    int _key_end;
    int _value_pos;
    int _value_end;
    bool _has_values;
    uint8_t _out_precision;
    double _out_double;
    ds_int _out_int;
    std::vector <std::string> _out_array;
    ds_asset _out_asset;
    ds_account _out_name;
    ds_time _out_time;
    char _out_key[500];

public:
    json_parser(const char *json, const k_v_map &key_values) {
        _json = json;
        _key_values = key_values;
        _status = STATUS_ERR_NULL;
        _pos = 0;
        _key_pos = 0;
        _key_end = 0;
        _value_pos = 0;
        _value_end = 0;
        _has_values = false;
    }

    bool is_whitespace(char c) {
        return c == '\t' || c == '\r' || c == '\n' || c == ' ';
    }

    bool skip_whitespace() {
        auto result = false;
        for (; _json[_pos] != '\0'; _pos++) {
            if (is_whitespace(_json[_pos])) {
                result = true;
            } else {
                break;
            }
        }
        return result;
    }

    bool skip_char(char c) {
        if (_json[_pos] != c) {
            return false;
        }
        _pos++;
        return true;
    }

    bool skip_colon() {
        skip_whitespace();
        if (!skip_char(':')) {
            return false;
        }
        skip_whitespace();
        return true;
    }

    parse_status parse_key() {
        skip_whitespace();

        if (!skip_char('"')) {
            return _status = STATUS_ERR_FMT;
        }
        _key_pos = _pos;
        for (; _json[_pos] != '\0' && _json[_pos] != '"'; _pos++);
        _key_end = _pos;
        if (!skip_char('"')) {
            return _status = STATUS_ERR_FMT;
        }
        if (!skip_colon()) {
            return _status = STATUS_ERR_FMT;
        }
        return STATUS_OK;
    }

    bool skip_letter() {
        if (_json[_pos] < 'A' || _json[_pos] > 'Z') {
            return false;
        }
        _pos++;
        return true;
    }

    bool parse_string() {
        if (!skip_char('"')) {
            return false;
        }
        _value_pos = _pos;
        for (; _json[_pos] != '\0' && _json[_pos] != '"'; _pos++);
        _value_end = _pos;
        if (!skip_char('"')) {
            return false;
        }
        return true;
    }

    bool parse_positive_int(ds_int &out_int) {
        auto result = false;
        out_int = 0;
        for (; _json[_pos] >= '0' && _json[_pos] <= '9'; _pos++) {
            out_int = (ds_int)(_json[_pos] - '0') + out_int * 10;
            result = true;
        }
        return result;
    }

    bool parse_int() {
        _value_pos = _pos;
        bool neg = skip_char('-');
        if (!parse_positive_int(_out_int)) {
            return false;
        }
        _out_int = neg ? -_out_int : _out_int;
        _value_end = _pos;
        return true;
    }

    bool parse_double() {
        _value_pos = _pos;
        bool neg = skip_char('-');
        ds_int out_int;
        auto result = parse_positive_int(out_int);
        if (!result) {
            return false;
        }
        if (skip_char('.')) {
            auto decimals_pos = _pos;
            ds_int out_decimals;
            result = parse_positive_int(out_decimals);
            if (!result) {
                return false;
            }
            _out_double = (double) out_decimals;
            _out_precision = _pos - decimals_pos;
            for (int i = 0; i < _out_precision; i++) {
                _out_double *= 0.1;
            }
            _out_double += (double) out_int;
            _out_double = neg ? -_out_double : _out_double;
        } else {
            _out_precision = 0;
            _out_double = (double) (neg ? -out_int : out_int);
        }
        _value_end = _pos;
        return true;
    }

    bool parse_time() {
        if (!skip_char('"')) {
            return false;
        }
        std::tm tm;
        if (!parse_positive_int(tm.tm_year) || tm.tm_year < 1900 || tm.tm_year > 3000) {
            return false;
        }
        if (!skip_char('-')) {

        }
        if (!parse_positive_int(tm.tm_mon) || tm.tm_mon < 1 || tm.tm_mon > 12) {
            return false;
        }
        if (!skip_char('-')) {

        }
        if (!parse_positive_int(tm.tm_mday) || tm.tm_mday < 1 || tm.tm_mday > 31) {
            return false;
        }
        if (!skip_char('T')) {

        }
        if (!parse_positive_int(tm.tm_hour) || tm.tm_hour < 0 || tm.tm_hour > 23) {
            return false;
        }
        if (!skip_char(':')) {

        }
        if (!parse_positive_int(tm.tm_min) || tm.tm_min < 0 || tm.tm_min > 59) {
            return false;
        }
        if (!skip_char(':')) {

        }
        if (!parse_positive_int(tm.tm_sec) || tm.tm_sec < 0 || tm.tm_sec > 59) {
            return false;
        }
        if (!skip_char('"')) {
            return false;
        }
        _out_time = ds_time(0);
        _out_time += tm.tm_sec;
        _out_time += tm.tm_min * 60;
        _out_time += tm.tm_hour * 60 * 60;
        auto d = tm.tm_mday;
        d += (tm.tm_year - 1970 - 3) / 4;
        d += tm.tm_mon > 1 ? 31 : 0;
        d += tm.tm_mon > 2 ? (tm.tm_year % 4 == 0 ? 29 : 28) : 0;
        d += tm.tm_mon > 3 ? 31 : 0;
        d += tm.tm_mon > 4 ? 30 : 0;
        d += tm.tm_mon > 5 ? 31 : 0;
        d += tm.tm_mon > 6 ? 30 : 0;
        d += tm.tm_mon > 7 ? 31 : 0;
        d += tm.tm_mon > 8 ? 31 : 0;
        d += tm.tm_mon > 9 ? 30 : 0;
        d += tm.tm_mon > 10 ? 31 : 0;
        d += tm.tm_mon > 11 ? 30 : 0;
        _out_time += d * 24 * 60 * 60;
        _out_time += (tm.tm_year - 1970) * 365 * 24 * 60 * 60;
        return true;
    }

    bool parse_array() {
        _out_array.clear();
        skip_whitespace();
        if (!skip_char('[')) {
            return false;
        }
        skip_whitespace();
        if (skip_char(']')) {
            return true;
        }
        while (true) {
            if (!parse_string()) {
                return false;
            }
            auto str = ds_string(_json, _value_pos, _value_end - _value_pos);
            _out_array.push_back(str);
            skip_whitespace();
            if (skip_char(']')) {
                return true;
            }
            skip_whitespace();
            if (!skip_char(',')) {
                return false;
            }
            skip_whitespace();
        }
    }

    bool parse_asset() {
        _value_pos = _pos;
        if (!parse_double()) {
            return false;
        }
        if (!skip_whitespace()) {
            return false;
        }
        for (auto i = 0; i < _out_precision; i++) {
            _out_double *= 10;
        }
        char symbol[8];
        char *c = symbol;
        while (skip_letter()) {
            *c++ = _json[_pos - 1];
        }
        *c = '\0';
        _out_asset = ds_asset(_out_double, ds_symbol(symbol, _out_precision));
        _value_end = _pos;
        return true;
    }

    bool parse_name() {
        if (parse_string()) {
            std::string_view str = ds_string(_json, _value_pos, _value_end - _value_pos);
            _out_name = (eosio::name) str;
            return true;
        }
        return false;
    }


    parse_value find_key() {
        auto itr = _key_values.find(get_out_key());
        if (itr != _key_values.end()) {
            return itr->second;
        }
        return VALUE_NONE;
    }

    parse_status parse() {
        if (_pos == 0) {
            skip_whitespace();
            if (!skip_char('{')) {
                return _status = STATUS_ERR_FMT;
            }
        }
        for (; _json[_pos] != '\0'; _pos++) {
            if (STATUS_OK != parse_key()) {
                return _status;
            }
            auto v = find_key();
            auto s = false;
            switch (v) {
                case VALUE_STRING:
                    s = parse_string();
                    break;
                case VALUE_NAME:
                    s = parse_name();
                    break;
                case VALUE_DOUBLE:
                    s = parse_double();
                    break;
                case VALUE_INT:
                    s = parse_int();
                    break;
                case VALUE_ASSET:
                    s = parse_asset();
                    break;
                case VALUE_DATETIME:
                    s = parse_time();
                    break;
                case VALUE_ARRAY:
                    s = parse_array();
                    break;
                default:
                    ds_print("\r\nparse key error position:%", _key_pos);
                    break;
            }
            if (!s) {
                return _status = STATUS_ERR_VALUE;
            }
            _has_values = true;
            skip_whitespace();
            if (skip_char('}')) {
                return _status = STATUS_OK;
            } else if (skip_char(',')) {
                return _status = STATUS_OK;
            }
            return _status = STATUS_ERR_FMT;
        }
        if (_has_values) {
            _status = STATUS_END;
            return _status;
        }
        _status = STATUS_ERR_NULL;
        return _status;
    }

    parse_status validate() {
        parse_status s;
        do {
            s = parse();
        } while (STATUS_OK == s);
        return s;
    }

    const char *get_value() {
        return _json + _value_pos;
    }

    int get_value_len() {
        return _value_end - _value_pos;
    }

    double get_out_double() {
        return _out_double;
    }

    std::vector <std::string> get_out_array() {
        return _out_array;
    }

    ds_int get_out_int() {
        return _out_int;
    }

    ds_asset get_out_asset() {
        return _out_asset;
    }

    ds_account get_out_name() {
        return _out_name;
    }

    ds_time get_out_time() {
        return _out_time;
    }

    const char *get_out_key() {
        ds_assert(_key_end - _key_pos < 500, "error parse json key");
        std::memcpy(_out_key, _json + _key_pos, _key_end - _key_pos);
        _out_key[_key_end - _key_pos] = '\0';
        return _out_key;
    }

    bool is_key_equals(const char *key) {
        return strlen(key) == _key_end - _key_pos && strncmp(_json + _key_pos, key, _key_end - _key_pos) == 0;
    }
};
