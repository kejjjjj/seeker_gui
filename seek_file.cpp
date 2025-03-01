#include "seek_file.hpp"
#include "seeker_gui.hpp"

#include "file_io.hpp"

#include <Windows.h>
#include <vector>

SHFILEINFO shfi;
#include <shellapi.h>
static void find_file_recursively(data_thread& data, const std::string_view& source, const std::string_view& filename)
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

			str = entry.path().string();
			r.num_searches++;
			r.current_file = str;

			const auto regexOk = data.searchData.type == ESearchType::regex && std::regex_search(str, std::get<1>(data.searchData.variant).regex);
			
			const auto fn = fs::get_file_name(str);
			const auto searchOk = data.searchData.type == ESearchType::standard && fn.contains(filename);
			
			if (regexOk || searchOk) {
				
				auto name = convertToWideString(str.c_str());

				if (SHGetFileInfo(name.c_str(), 0, &shfi, sizeof(shfi), SHGFI_ICON | SHGFI_SMALLICON)) {
					r.results.push_back({ str, shfi.hIcon, std::nullopt });
				}
			}
		}
	}
	catch ([[maybe_unused]] std::filesystem::filesystem_error& ex) {
		//MessageBox(NULL, ex.path1().wstring().c_str(), L"ok", MB_ICONERROR);
		//return;
	}
	
	for (auto& dir : directories) {
		find_file_recursively(data, dir, filename);
	}

	return;
}

void find_file(data_thread& data, const std::string& source, const std::string filename)
{

	auto old = steady_clock::now();

	find_file_recursively(data, source, filename);
		
	auto now = steady_clock::now();
	std::chrono::duration<decltype(data.data.duration)> difference = now - old;
	
	std::lock_guard<std::mutex> lock(data.mtx);
	data.data.duration = difference.count();
	data.active = false;
}