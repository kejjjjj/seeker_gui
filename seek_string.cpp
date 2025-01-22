#include "seek_file.hpp"
#include "seeker_gui.hpp"

#include "file_io.hpp"

#include <Windows.h>
#include <vector>

SHFILEINFO shfi_string;


static std::optional<search_positions> parse_file(seek_results_t& results, const std::string_view& file, const std::string_view& target);

static void find_string_recursively(data_thread& data, const std::string_view& source,
	const std::string_view& target, const std::regex* regex)
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

			if (regex && !std::regex_search(str, *regex)) {
				continue;
			}


			if (auto v = parse_file(r, str, target)) {
				auto name = convertToWideString(str.c_str());

				HICON icon = 0;

				if (SHGetFileInfo(name.c_str(), 0, &shfi_string, sizeof(shfi_string), SHGFI_ICON | SHGFI_SMALLICON)) {
					icon = shfi_string.hIcon;
				}

				for (auto& value : v.value())
					r.results.push_back({ str, icon, value });
			}

		}

		for (auto& dir : directories) {
			find_string_recursively(data, dir, target, regex);
		}
	}
	catch ([[maybe_unused]] std::filesystem::filesystem_error& ex) {
	}

	return;
}

void find_string(data_thread& data, const std::string& source, const std::string filename)
{
	auto old = steady_clock::now();

	auto regex = std::regex(data.regexStr);

	find_string_recursively(data, source, filename, data.regexStr.size() ? &regex : nullptr);

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