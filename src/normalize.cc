#include <iostream>
#include <fstream>
#include <unordered_set>
#include <optional>

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

std::optional<float> time2float(std::string_view time)
{
	if (time.size() != 8) return std::nullopt; // Expected time in format hh:mm:ss

	auto const hours   = ((time[0] - '0') * 10 + time[1] - '0');
	auto const minutes = ((time[3] - '0') * 10 + time[4] - '0');
	if (hours >= 24) return std::nullopt; // Single day has only 24 hours

	return (float(hours) * 60 + float(minutes)) / (60 * 24);
}

int main()
{
	std::cout.sync_with_stdio(false);

	bool passed_header = false;
	for (std::string line; std::getline(std::cin, line);) {
		split::iterator row(line, '\t');
		std::advance(row, 2); if (row == split::sentinel{}) continue; std::string_view const departure_time = *row;
		std::advance(row, 1); if (row == split::sentinel{}) continue; std::string_view const stop_id = *row;
		std::advance(row, 2); if (row == split::sentinel{}) continue; std::string_view const stop_headsign = *row;

		if (!passed_header) {
			std::cout << departure_time << '\t' << stop_id << '\t' << stop_headsign << '\n';
			passed_header = true;
			continue;
		}

		auto const departure = time2float(departure_time); if (!departure) continue;

		std::cout << *departure << '\t' << stop_id << '\t' << stop_headsign << '\n';
	}
}
