#define IMM_JSON_IMPLEMENTATION
#include "json/imm_json.hh"
#include <cassert>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <utility>

namespace split
{
	struct sentinel {};

	struct iterator
	{
		using difference_type = ptrdiff_t;
		using value_type = std::string_view;
		using iterator_category = std::input_iterator_tag;
		using pointer = void;
		using reference = std::string_view&;

		explicit iterator(std::convertible_to<std::string_view> auto&& source, char delim)
			: source{source}
			, delim{delim}
		{
			++*this; // Compute first cell
		}

		inline iterator begin() const
		{
			return *this;
		}

		inline sentinel end() const
		{
			return sentinel{};
		}

		inline bool operator==(sentinel) const
		{
			return reached_end;
		}

		inline iterator& operator++()
		{
			if (source.empty()) {
				reached_end = true;
				return *this;
			}

			if (auto tab = source.find(delim); tab != std::string_view::npos) {
				current = source.substr(0, tab);
				source.remove_prefix(tab+1);
			} else {
				current = source;
				source = {};
			}
			return *this;
		}

		inline iterator operator++(int)
		{
			auto copy = *this;
			++*this;
			return copy;
		}

		inline std::string_view operator*() const
		{
			return current;
		}

		std::string_view current;
		std::string_view source;
		char delim;
		bool reached_end = false;
	};
}

struct Expression
{
	enum Type
	{
		Symbol,
		Call,
	};

	Type type;
	std::string_view symbol;
	std::vector<Expression> sub;

	Expression(std::string_view symbol)
		: type{Type::Symbol}
		, symbol{symbol}
	{
	}

	Expression(std::string_view name, Expression &&arg)
		: type{Type::Call}
		, symbol{name}
		, sub{std::move(arg)}
	{
	}

	Expression(Expression const&) = default;
	Expression(Expression &&) = default;

	friend std::ostream& operator<<(std::ostream& os, Expression const& expr)
	{
		switch (expr.type) {
		break; case Type::Symbol: os << std::quoted(expr.symbol);
		break; case Type::Call: os << expr.symbol << '(' << expr.sub.front() << ')';
		}
		return os;
	}
};

std::vector<Expression> parse_normalization_expression(std::string_view &source)
{
	auto const skip_ws = [&] {
		if (auto after_ws = source.find_first_not_of(" \t"); after_ws != std::string_view::npos) {
			source.remove_prefix(after_ws);
		}
	};

	std::vector<Expression> sequence;

	for (;;) {
		std::string_view symbol;
		skip_ws();

		// FIXME String escaping
		if (source.starts_with('"')) {
			source.remove_prefix(1);
			auto const string_end = source.find('"');
			if (string_end == std::string_view::npos) {
				std::cerr << "[ERROR] Failed to parse '" << source << "': expected end of string\n";
				std::exit(2);
			}

			std::string_view symbol{source.begin(), string_end};
			source.remove_prefix(string_end+1);
			skip_ws();
			sequence.emplace_back(symbol);
			goto next;
		}

		{
			// Don't use islower since it uses locale (slow)
			auto const symbol_end = std::find_if_not(source.begin(), source.end(), [](char c) { return c >= 'a' && c <= 'z'; });
			if (symbol_end == source.begin()) {
				std::cerr << "[ERROR] Failed to parse '" << source << "': expected symbol\n";
				std::exit(2);
			}

			symbol = std::string_view{source.begin(), symbol_end};
			source.remove_prefix(symbol.size());
			skip_ws();
		}

		if (source.empty()) {
			sequence.emplace_back(symbol);
			goto next;
		}

		if (source.starts_with("(")) {
			source.remove_prefix(1);
			// FIXME Should separate expression sequence and expression argument parsing
			sequence.emplace_back(symbol, std::move(parse_normalization_expression(source).front()));
			skip_ws();
			if (!source.starts_with(")")) {
				std::cerr << "[ERROR] Failed to parse '" << source << "': expected closing bracket\n";
				std::exit(2);
			}
			source.remove_prefix(1);
			goto next;
		}

next:
		skip_ws();
		if (source.starts_with('.')) {
			source.remove_prefix(1);
			continue;
		} else {
			break;
		}
	}
	return sequence;
}

struct Value
{
	enum class Type
	{
		String,
		Number,
	};

	Type type;
	std::string string;
	double number;

	explicit Value(std::string_view s)
		: type(Type::String)
		, string(s)
	{
	}

	explicit Value(double number)
		: type(Type::Number)
		, number(number)
	{
	}
};

using Builtin = Value(*)(Value, std::optional<std::string_view>);
using Env = std::unordered_map<std::string, Builtin>;

Value eval(std::span<Expression> expressions, Value value, Env const& env)
{
	for (auto const& expr : expressions) {
		auto builtin = env.find(std::string(expr.symbol));
		if (builtin == env.end()) {
			std::cerr << "[ERROR] Unknown builtin: " << expr.symbol << '\n';
			std::exit(1);
		}
		switch (expr.type) {
		break; case Expression::Symbol:
			value = builtin->second(std::move(value), std::nullopt);
		break; case Expression::Call:
			assert(expr.sub.size() == 1);
			assert(expr.sub.front().type == Expression::Symbol);
			value = builtin->second(std::move(value), expr.sub[0].symbol);
		}
	}
	return value;
}

struct Column
{
	Column(std::string_view name, std::string_view normalization_expression)
		: name(name)
		, expression_source(normalization_expression)
		, expression(parse_normalization_expression(normalization_expression))
	{
	}

	inline Value normalize(std::string_view source, Env const& env)
	{
		return eval(std::span(expression), Value(source), env);
	}

	std::string_view name;
	std::string_view expression_source;
	std::vector<Expression> expression;
};

int main(int argc, char** argv)
{
	if (argc != 2) {
		std::cerr << "usage: " << argv[0] << " <columns.tsv>\n";
		std::cerr << "  convert tsv file from TSV using definitions from columns.tsv\n";
	}

	std::ifstream columns_file(argv[1]);
	static std::string source{std::istreambuf_iterator<char>(columns_file), {}};

	if (auto it = std::next(split::iterator(source, '\t'), 1); it == split::sentinel{} || *it != "Type") {
		std::cerr << "[ERROR] Expected Type description in column 2\n";
		return 1;
	}

	std::vector<Column> columns;

	for (std::string_view line : split::iterator(source, '\n')) {
		auto tsv_it = split::iterator(line, '\t');
		if (tsv_it == split::sentinel{} || *tsv_it++ != "y") {
			continue;
		}
		auto const type = *tsv_it++; if (tsv_it == split::sentinel{}) continue;
		[[maybe_unused]] auto const _column_number = *tsv_it++; if (tsv_it == split::sentinel{}) continue;
		auto const name = *tsv_it++; if (tsv_it == split::sentinel{}) continue;

		columns.emplace_back(name, type);
	}

	Env env = {
		std::pair<std::string, Builtin> { "lower", +[](Value v, std::optional<std::string_view>) -> Value {
			assert(v.type == Value::Type::String);
			// FIXME Proper UTF-8 lowercase
			// However, manual inspection of used TSV files prooved that there aren't any non-ascii uppercase letters
			for (char &c : v.string) {
				if (c >= 'A' && c <= 'Z') {
					c = c - 'A' + 'a';
				}
			}
			return v;
		}},

		std::pair<std::string, Builtin> { "int", +[](Value v, std::optional<std::string_view>) -> Value {
			assert(v.type == Value::Type::String);

			long long int n;
			std::cout.flush();
			auto [p, ec] = std::from_chars(v.string.data(), v.string.data() + v.string.size(), n);
			assert(ec == std::errc{});
			return Value(double(n));
		}},
	};


	{
		bool passed_header = false;
		Json json;
		auto _array = json.array();
		for (std::string line; std::getline(std::cin, line); ) {
			if (!passed_header) {
				passed_header = true;
				continue;
			}
			auto _object = json.object();
			auto tsv = split::iterator(line, '\t');
			for (auto i = 0u; tsv != split::sentinel{}; ++i, ++tsv) {
				auto normalized = columns[i].normalize(*tsv, env);
				switch (normalized.type) {
				break; case Value::Type::String: json.key(columns[i].name) = normalized.string;
				break; case Value::Type::Number: json.key(columns[i].name) = double(normalized.number);
				}
			}
			break;
		}
	}

	return 0;
}
