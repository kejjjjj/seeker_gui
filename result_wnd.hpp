#pragma once

#include <wx/wx.h>
#include <msvc/wx/setup.h>
#include <wx/listctrl.h>


#include "seeker_gui.hpp"

enum class frame_e : std::int8_t
{
	file,
	folder,
	string
};

class ResultFrame : public wxFrame
{
public:
	ResultFrame() = delete;
	explicit ResultFrame(shared_data_thread& d, const std::string& title, int timer=100)
		: wxFrame(nullptr, wxID_ANY, title), result_data(d)
	{
		data_list = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, { GetSize().x, wxDefaultSize.y }, wxLC_REPORT | wxLC_SINGLE_SEL);

		result_timer = new wxTimer(this);
		result_timer->Start(timer);
		Bind(wxEVT_TIMER, &ResultFrame::OnTimer, this);
		data_list->Bind(wxEVT_LIST_ITEM_ACTIVATED, &ResultFrame::OnItemClicked, this);

	};

	virtual frame_e type() = 0;

protected:
	shared_data_thread result_data;
	wxTimer* result_timer = 0;
	wxListCtrl* data_list = 0;
	wxImageList* icon_list = 0;
	long num_items = 0;

	void MakeWindowCentered();

	virtual void OnItemClicked(wxListEvent& event);
	virtual void InsertItemToList(const seek_item_t& data);
	virtual void OnTimer(wxTimerEvent& event);

	virtual void OnResize(wxSizeEvent& event) = 0;

	[[nodiscard]] constexpr auto TooManyItems() const noexcept {
		return num_items + 1 > std::numeric_limits<long>::max();
	}

	NO_COPY_CONSTRUCTOR(ResultFrame);
};

class FileFrame final : public ResultFrame
{
public:
	FileFrame() = delete;
	explicit FileFrame(shared_data_thread& d, const std::string& title);

	frame_e type() override { return frame_e::file; };

private:
	void OnResize(wxSizeEvent& event) override;
};

class FolderFrame final : public ResultFrame
{
public:
	FolderFrame() = delete;
	explicit FolderFrame(shared_data_thread& d, const std::string& title);

	frame_e type() override { return frame_e::folder; };

private:
	void OnResize(wxSizeEvent& event) override;
};
class StringFrame final : public ResultFrame
{
public:
	StringFrame() = delete;
	explicit StringFrame(shared_data_thread& d, const std::string& title);

	frame_e type() override { return frame_e::string; };

private:
	void InsertItemToList(const seek_item_t& data) override;
	void OnResize(wxSizeEvent& event) override;
};