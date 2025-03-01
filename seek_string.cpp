#include "seek_file.hpp"
#include "seeker_gui.hpp"

#include "file_io.hpp"

#include <Windows.h>
#include <vector>

SHFILEINFO shfi_string;


static std::optional<search_positions> parse_file(seek_results_t& results, const std::string_view& file, const std::string_view& target);
static std::optional<search_positions> parse_file_regex(seek_results_t& results, const std::string_view& file, const RegexData& target);

static void find_string_recursively(data_thread& data, const std::string_view& source)
{
	std::vector<std::string> directories;

	auto& r = data.data;

	if (!data.active)
		return;

	try {
		for (const auto& entry : _fs::directory_iterator(source)) {
			
			if (containsNonASCII(entry.path().wstring()))
				continue;
			
			auto str = entry.path().string();

			if (entry.is_directory()) {
				directories.push_back(str);
				continue;
			}
			std::lock_guard<std::mutex> lock(data.mtx);

			r.current_file = str;
			r.num_searches++;

			std::optional<search_positions> positions;

			if (data.searchData.type == ESearchType::standard)
				positions = parse_file(r, str, std::get<0>(data.searchData.variant));
			if (data.searchData.type == ESearchType::regex)
				positions = parse_file_regex(r, str, std::get<1>(data.searchData.variant));

			if (positions) {
				auto name = convertToWideString(str.c_str());

				HICON icon = 0;

				if (SHGetFileInfo(name.c_str(), 0, &shfi_string, sizeof(shfi_string), SHGFI_ICON | SHGFI_SMALLICON)) {
					icon = shfi_string.hIcon;
				}

				for (auto& value : *positions)
					r.results.push_back({ str, icon, value });
			}

		}

		for (auto& dir : directories) {
			find_string_recursively(data, dir);
		}
	}
	catch ([[maybe_unused]] std::filesystem::filesystem_error& ex) {
	}

	return;
}

void find_string(data_thread& data, const std::string& source, [[maybe_unused]] const std::string filename)
{
	auto old = steady_clock::now();

	find_string_recursively(data, source);

	auto now = steady_clock::now();
	std::chrono::duration<decltype(data.data.duration)> difference = now - old;

	std::lock_guard<std::mutex> lock(data.mtx);
	data.active = false;
	data.data.duration = difference.count();
	

}

std::optional<search_positions> parse_file(seek_results_t& results, const std::string_view& file, const std::string_view& keyword)
{
	std::ifstream f;
	f.open(file, std::ios_base::binary | std::ios_base::in);

	size_t consecutive_characters = 0ull;
	const size_t num_chars = keyword.size();
	std::string buffer{};

	size_t column = 1;
	size_t line = 1;

	if (!f.is_open()) {
		results.num_skips++;
		return std::nullopt;
	}

	search_positions positions;

	while (f.is_open() && !f.eof() && f.good()) {
		try {
			auto ch = f.get();


			if (ch == '\n') {
				line++;
				column = 1ull;
			}
			else {
				if (ch == '\t')
					column += 4ull;
				else
					column++;
			}

			if (consecutive_characters && keyword == buffer) {
				positions.push_back({ line, column });

			}
			else if (ch == keyword[consecutive_characters] && consecutive_characters < num_chars) {

				buffer.push_back(static_cast<char>(ch));
				consecutive_characters++;
				continue;
			}

			consecutive_characters = 0ull;
			buffer.clear();


		}
		catch ([[maybe_unused]]std::ios_base::failure& ex) {
			results.num_skips++;
		}
	}

	f.close();
	return positions.empty() ? std::nullopt : std::make_optional<search_positions>(positions);

}



std::optional<search_positions> parse_file_regex(seek_results_t& results, const std::string_view& file, const RegexData& target)
{
	std::ifstream f;
	f.open(file, std::ios_base::binary | std::ios_base::in);

	if (!f.is_open()) {
		return std::nullopt;
	}

	search_positions matches;

	std::ostringstream oss;
	oss << f.rdbuf();
	std::string buff = oss.str();

	try {

		auto begin_it = std::sregex_iterator(buff.cbegin(), buff.cend(), target.regex);
		auto end_it = std::sregex_iterator();

		matches.reserve(std::distance(begin_it, end_it)); // Reserve memory for performance

		for (auto it = begin_it; it != end_it; ++it) {
			matches.push_back({ 1, it->position() });
		}

	}
	catch ([[maybe_unused]] std::ios_base::failure& ex) {
		results.num_skips++;
	}
	

	f.close();
	return matches.empty() ? std::nullopt : std::optional<search_positions>{ matches };

}
