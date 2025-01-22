#include "seek_folder.hpp"
#include "seeker_gui.hpp"

#include "file_io.hpp"

#include <Windows.h>
#include <vector>


SHFILEINFO shfi_folder;

static void find_folder_recursively(data_thread& data, const std::string_view& source, 
	const std::string_view& filename, const std::regex* regex)
{
	std::vector<std::string> directories;
	auto& r = data.data;

	if (!data.active)
		return;

	try {
		for (const auto& entry : _fs::directory_iterator(source)) {

			if (entry.is_directory()) {

				if (containsNonASCII(entry.path().wstring()))
					continue;

				auto str = entry.path().string();

				std::lock_guard<std::mutex> lock(data.mtx);

				r.current_file = str;
				r.num_searches++;

				if (regex && !std::regex_search(str, *regex)) {
					continue;
				}

				if (filename == fs::get_file_name(str)) {
					r.current_file = str;

					auto name = convertToWideString(str.c_str());

					if (SHGetFileInfo(name.c_str(), 0, &shfi_folder, sizeof(shfi_folder), SHGFI_ICON | SHGFI_SMALLICON)) {
						r.results.push_back({ str, shfi_folder.hIcon, std::nullopt });

					}
				}

				directories.push_back(str);
			}
		}
	}
	catch ([[maybe_unused]] std::filesystem::filesystem_error&) {
		
	}

	for (auto& dir : directories) {
		find_folder_recursively(data, dir, filename, regex);
	}

	return;
}

void find_folder(data_thread& data, const std::string& source, const std::string filename)
{
	auto old = steady_clock::now();

	auto regex = std::regex(data.regexStr);
	find_folder_recursively(data, source, filename, data.regexStr.size() ? &regex : nullptr);

	auto now = steady_clock::now();
	std::chrono::duration<decltype(data.data.duration)> difference = now - old;
	
	std::lock_guard<std::mutex> lock(data.mtx);
	data.data.duration = difference.count();
	data.active = false;


}