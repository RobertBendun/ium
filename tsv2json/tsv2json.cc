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
	std::vector<Expression> sub{};

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
		Null,
		Bool,
		Number,
		String,
		Vector,
	};

	Type type = Type::Null;
	bool boolean = false;
	std::string string{};
	double number = 0;
	std::vector<Value> vector{};

	explicit Value()
		: type(Type::Null)
	{
	}

	explicit Value(bool b)
		: type(Type::Bool)
		, boolean(b)
	{
	}

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

	explicit Value(std::vector<Value> vector)
		: type(Type::Vector)
		, vector(std::move(vector))
	{
	}
};

Json& operator+=(Json& json, Value const& value)
{
	switch (value.type) {
	break; case Value::Type::Null:   json = nullptr;
	break; case Value::Type::Bool:   json = value.boolean;
	break; case Value::Type::String: json = value.string;
	break; case Value::Type::Number: json = value.number;
	break; case Value::Type::Vector:
		{
			auto _array = json.array();
			for (auto const& element : value.vector) json += element;
		}
	}
	return json;
}

struct Builtin
{
	std::string_view name;
	Value(*handler)(Value, std::optional<std::string_view>);
	bool accepts_vector = false;
};

using Env = std::vector<Builtin>;

Value eval(std::vector<Expression> const& expressions, Value value, Env const& env)
{
	for (auto const& expr : expressions) {
		auto builtin = std::find_if(env.begin(), env.end(), [expr](Builtin const& b) { return b.name == expr.symbol; });
		if (builtin == env.end()) {
			std::cerr << "[ERROR] Unknown builtin: " << expr.symbol << '\n';
			std::exit(1);
		}
		if (!builtin->accepts_vector && value.type == Value::Type::Vector) {
			for (auto &element : value.vector) {
				switch (expr.type) {
				break; case Expression::Symbol:
					element = builtin->handler(std::move(element), std::nullopt);
				break; case Expression::Call:
					assert(expr.sub.size() == 1);
					assert(expr.sub.front().type == Expression::Symbol);
					element = builtin->handler(std::move(element), expr.sub[0].symbol);
				}
			}
		} else {
			switch (expr.type) {
			break; case Expression::Symbol:
				value = builtin->handler(std::move(value), std::nullopt);
			break; case Expression::Call:
				assert(expr.sub.size() == 1);
				assert(expr.sub.front().type == Expression::Symbol);
				value = builtin->handler(std::move(value), expr.sub[0].symbol);
			}
		}
	}
	return value;
}

struct Column
{
	Column(std::string_view name, std::string_view normalization_expression)
		: name(name)
		, expression_source(normalization_expression)
		, expression()
	{
		expression = parse_normalization_expression(normalization_expression);
	}

	inline Value normalize(std::string_view source, Env const& env)
	{
		return eval(expression, Value(source), env);
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
		if (tsv_it == split::sentinel{} || !(*tsv_it++).starts_with("y")) { continue; }

		auto const type = *tsv_it++; if (tsv_it == split::sentinel{}) continue;
		[[maybe_unused]] auto const _column_number = *tsv_it++; if (tsv_it == split::sentinel{}) continue;
		auto const name = *tsv_it++;

		columns.emplace_back(name, type);
	}

	Env env = {
		Builtin { "lower", +[](Value v, std::optional<std::string_view>) -> Value {
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

		Builtin { "int", +[](Value v, std::optional<std::string_view>) -> Value {
			assert(v.type == Value::Type::String);

			long long int n;
			std::cout.flush();
			if (v.string.empty()) {
				return Value{};
			}
			auto [p, ec] = std::from_chars(v.string.data(), v.string.data() + v.string.size(), n);
			if (ec != std::errc{}) {
				return Value{};
			}


			return Value(double(n));
		}},

		Builtin { "bool", +[](Value v, std::optional<std::string_view>) -> Value {
			assert(v.type == Value::Type::String);
			return Value(v.string.empty());
		}},

		Builtin { "str", +[](Value v, std::optional<std::string_view>) -> Value {
			assert(v.type == Value::Type::String);
			return v;
		}},

		Builtin { "sep", +[](Value v, std::optional<std::string_view> by) -> Value {
			assert(by && "sep requires parameter by which it can split");
			assert(v.type == Value::Type::String && "only string can be splitted");

			std::vector<Value> separated;
			std::string_view source = v.string;
			for (;;) if (auto split_point = source.find(*by); split_point != std::string_view::npos) {
				separated.emplace_back(source.substr(0, split_point));
				source.remove_prefix(split_point + by->size());
			} else {
				break;
			}

			if (source.size()) {
				separated.emplace_back(source);
			}
			return Value(std::move(separated));
		}},

		Builtin {
			.name = "unless",
			.handler = +[](Value v, std::optional<std::string_view> needle) -> Value {
				assert(v.type == Value::Type::String);
				assert(needle && "Unless requires string to search for");
				if (v.string.find(*needle) == std::string::npos)
					return v;
				return Value("");
			},
			.accepts_vector = false
		}
	};


	{
		bool passed_header = false;
		Json json;
		auto _array = json.array();
		for (std::string line; std::getline(std::cin, line);) {
			if (!passed_header) {
				passed_header = true;
				continue;
			}
			auto _object = json.object();
			auto tsv = split::iterator(line, '\t');
			for (auto column = 0u; tsv != split::sentinel{}; ++column, ++tsv) {
				assert(column < columns.size());
				json.key(columns[column].name) += columns[column].normalize(*tsv, env);
			}
			break;
		}
	}

	return 0;
}
