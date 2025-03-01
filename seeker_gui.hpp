#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <thread>
#include <sstream>
#include <direct.h>
#include <functional>
#include <optional>
#include <mutex>
#include <queue>
#include <ShlObj.h>

#include "utils.hpp"
#include "seek_main.hpp"

#define NO_COPY_CONSTRUCTOR(CLASS) const CLASS& operator=(const CLASS&) = delete; \
CLASS(const CLASS&) = delete

using search_position = std::pair<std::size_t, std::size_t>;
using search_positions = std::vector<search_position>;

struct seek_item_t
{
	std::string path;
	HICON icon;
	std::optional<search_position> l;
};

struct seek_results_t
{
	std::list<seek_item_t> results;
	float duration = 0.f;
	std::size_t num_searches = 0ull;
	std::size_t num_skips = 0ull;
	std::string current_file;
};

struct data_thread
{
	std::mutex mtx;
	std::thread thread;
	seek_results_t data = {};
	bool active = false;
	CSearchData searchData;
};

using steady_clock = std::chrono::steady_clock;
using seek_results = std::vector<seek_results_t>;
using shared_data_thread = std::shared_ptr<data_thread>;
using namespace std::chrono_literals;
