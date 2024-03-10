#include "file_io.hpp"

//std::string fs::exe_file_name()
//{
//	wchar_t buffer[MAX_PATH];
//	GetModuleFileName(NULL, buffer, MAX_PATH);
//	return std::string(buffer);
//}
//std::string fs::exe_path()
//{
//	std::string f = exe_file_name();
//	return f.substr(0, f.find_last_of("\\/"));
//}
//std::string fs::root_path()
//{
//	return exe_path() + "\\agent";
//}
std::string fs::get_extension(const std::string& file)
{
	size_t const extensionPos = file.find_last_of(".");

	if (extensionPos == std::string::npos)
		return "";

	return file.substr(extensionPos);
}
std::string fs::previous_directory(std::string& directory)
{
	size_t pos = directory.find_last_of('\\');
	if (pos < 1 || pos == std::string::npos)
		return directory;

	return directory.substr(0, pos);
}
std::string fs::get_file_name(const std::string& fullpath)
{
	size_t pos = fullpath.find_last_of('\\');

	if (pos < 1 || pos == std::string::npos)
		return fullpath;

	return fullpath.substr(pos + 1);
}
std::string fs::get_file_name_no_extension(const std::string& fullpath)
{
	auto file = get_file_name(fullpath);
	auto extension = get_extension(file);
	return file.substr(0, file.size() - extension.size());

}
bool fs::io_open(std::ifstream& fp, const std::string& path, const fs::fileopen type)
{
	if (fp.is_open())
		return true;

	fp.open(path, static_cast<std::ios_base::openmode>(std::underlying_type_t<fs::fileopen>(type)));

	if (!fp.is_open())
		return false;


	return true;
}
void fs::io_close(std::ifstream& f)
{
	if (f.is_open())
		f.close();
}

void fs::create_file(const std::string& path)
{
	std::fstream* nf = new std::fstream(path, std::ios_base::out);
	*nf << "";
	if (nf->is_open())
		nf->close();
	delete nf;
}
bool fs::create_directory(const std::string& path)
{
	return _mkdir((path).c_str()) != -1;
}

std::vector<std::string> fs::files_in_directory(const std::string& path)
{
	std::vector<std::string> files;

	if (!_fs::exists(path)) {
		return {};
	}

	for (const auto& entry : _fs::directory_iterator(path)) {
		if (entry.is_directory())
			continue;

		std::string str = entry.path().string();
		files.push_back(std::move(str));
	}

	return (files); //compiler I hope you optimize this! 
}
std::vector<std::string> fs::directories_in_directory(const std::string& path)
{
	std::vector<std::string> directories;

	if (!_fs::exists(path)) {
		return {};
	}

	for (const auto& entry : _fs::directory_iterator(path)) {
		if (!entry.is_directory())
			continue;

		std::string str = entry.path().string();
		directories.push_back(std::move(str));
	}

	return (directories); //compiler I hope you optimize this! 
}
bool fs::valid_file_name(const std::string& name)
{
	if (name.empty())
		return false;

	for (const auto& i : name) {
		if (!std::isalnum(i) && i != '-' && i != '_' && i != ' ')
			return false;

	}
	return true;
}
std::vector<std::vector<std::string>> fs::split_directory_into_directory_sections(const std::string& src, const int num_sections)
{
	auto dirs = fs::directories_in_directory(src);

	if (dirs.empty())
		return { dirs };

	std::vector<std::vector<std::string>> result;

	if (num_sections > dirs.size()) {

		for (auto& dir : dirs)
			result.push_back({ dir });

		return result;
	}

	const size_t items_per_section = dirs.size() / num_sections;

	size_t num_iterations = 0ull;

	for (size_t section = 0ull; section < num_sections; num_iterations = section) {

		std::vector<std::string> s;

		for (size_t i = 0ull; i < items_per_section; i++) {

			s.push_back(dirs[section + items_per_section]);
		}
		
		result.push_back(s);
		s.clear();
		section += items_per_section;

	}

	if (num_iterations < dirs.size()) {
		for (size_t i = num_iterations; i < dirs.size(); i++)
			result.back().push_back(dirs[i]);
	}

	return result;

}
std::string fs::get_last_error()
{
	const DWORD errorMessageID = ::GetLastError();
	char* messageBuffer = nullptr;

	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&messageBuffer, 0, NULL);

	//Com_PrintError(CON_CHANNEL_CONSOLEONLY, "IO_WriteData failed with: %s\n", messageBuffer);

	std::string output = std::string(messageBuffer, size);

	LocalFree(messageBuffer);
	return output;
}