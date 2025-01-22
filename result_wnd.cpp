#include "result_wnd.hpp"
#include "utils.hpp"
#include "file_io.hpp"

#include <type_traits>

void ResultFrame::MakeWindowCentered()
{
    constexpr auto x = 320;
    constexpr auto y = 240;

    const auto w = adjust_to_640(GetSize().x);
    const auto h = adjust_to_480(GetSize().y);

    const auto diff_x = std::abs(x - w) / 2;
    const auto diff_y = std::abs(y - h) / 2;

    const auto new_x = x - diff_x;
    const auto new_y = y - diff_y;

    const auto fixed = adjust_from_640x480({ new_x, new_y, w, h });

    SetPosition({ fixed.x, fixed.y });
    //SetSize({ fixed.w, fixed.h });

}
void ResultFrame::OnTimer([[maybe_unused]] wxTimerEvent& event)
{
    if (TooManyItems() || !result_data->active && result_data->data.results.empty()) {

        if(TooManyItems())
            SetStatusText(std::format("Exceeded: {} items", num_items));
        else
            SetStatusText(std::format("Found: {}", num_items));

        result_timer->Stop();
        return;
    }


    SetStatusText(std::format("Found: {} | Searched: {} | {}", num_items, result_data->data.num_searches, result_data->data.current_file));

    if (result_data->data.results.empty())
        return;

    std::lock_guard<std::mutex> lock(result_data->mtx);

    for (auto& q : result_data->data.results) {
        InsertItemToList(q);
    }

    result_data->data.results.clear();
}
FileFrame::FileFrame(shared_data_thread &d, const std::string& title) : ResultFrame(d, title)
{
    data_list->InsertColumn(0, "Name", 0, GetSize().x);
    
    CreateStatusBar();

    icon_list = new wxImageList(16, 16);
    data_list->AssignImageList(icon_list, wxIMAGE_LIST_SMALL);

    MakeWindowCentered();


}
void ResultFrame::InsertItemToList(const seek_item_t& data)
{
    wxIcon icon;

    if (TooManyItems())
        return;

    icon_list = data_list->GetImageList(wxIMAGE_LIST_SMALL);

    icon.CreateFromHICON(data.icon != nullptr ? data.icon : LoadIcon(NULL, IDI_ERROR));
    auto idx = icon_list->Add(icon);

    //data_list->AssignImageList(icon_list, wxIMAGE_LIST_SMALL);
    data_list->InsertItem(num_items++, data.path, idx);

}
#define SE_SUCCEEDED(h) ((std::size_t(h)) > 32)

void ResultFrame::OnItemClicked(wxListEvent& event)
{
    int index = event.GetIndex();
    auto selectedFile = data_list->GetItemText(index);
    auto s = selectedFile.ToStdString();

    if(!fs::is_directory(s))
        s = fs::previous_directory(s);

    auto wide = convertToWideString(s);

    if (!SE_SUCCEEDED(ShellExecute(NULL, TEXT("open"), TEXT("explorer.exe"), wide.c_str(), NULL, SW_SHOWDEFAULT))) {
        wxMessageBox(std::format("ShellExecute failed with: {}", fs::get_last_error()), "Error", wxOK | wxICON_ERROR);
    }

}

void FileFrame::OnResize(wxSizeEvent& event)
{
	event.Skip();
}

FolderFrame::FolderFrame(shared_data_thread& d, const std::string& title) : ResultFrame(d, title)
{
    data_list->InsertColumn(0, "Name", 0, GetSize().x);

    CreateStatusBar();

    icon_list = new wxImageList(16, 16);
    data_list->AssignImageList(icon_list, wxIMAGE_LIST_SMALL);

    MakeWindowCentered();

}
void FolderFrame::OnResize(wxSizeEvent& event)
{
    event.Skip();
}


StringFrame::StringFrame(shared_data_thread& d, const std::string& title) : ResultFrame(d, title, 2000)
{
    const auto LINE_COLUMN = adjust_from_640(20);

    data_list->InsertColumn(0, "Name", 0, GetSize().x - LINE_COLUMN);
    data_list->InsertColumn(1, "Location", 0, LINE_COLUMN);

    CreateStatusBar();

    icon_list = new wxImageList(16, 16);
    data_list->AssignImageList(icon_list, wxIMAGE_LIST_SMALL);

    MakeWindowCentered();

}
void StringFrame::OnResize(wxSizeEvent& event)
{
    event.Skip();
}
void StringFrame::InsertItemToList(const seek_item_t& data)
{
    static std::unordered_map<HICON, int> used_icons;

    int icon_idx = 0;

    auto r = used_icons.find(data.icon);
    if (r == used_icons.end()) {
        wxIcon icon;
        icon_list = data_list->GetImageList(wxIMAGE_LIST_SMALL);

        if (!icon.CreateFromHICON(data.icon != nullptr ? data.icon : LoadIcon(NULL, IDI_ERROR)))
            return;

        if (!wxBitmap(icon).GetRefData()) {
            icon.CreateFromHICON(LoadIcon(NULL, IDI_ERROR));
        }
        icon_idx = icon_list->Add(icon);
        used_icons.insert({ data.icon, static_cast<int>(used_icons.size()) });
    }
    else {
        icon_idx = r->second;
    }
    



    wxListItem item;
    item.SetId(num_items++);
    item.SetColumn(0);
    item.SetText(data.path);
    item.SetImage(icon_idx);
    data_list->InsertItem(item);

    item.m_image = -1;
    item.SetColumn(1);
    item.SetText(std::format("{}, {}", data.l->first, data.l->second));

    data_list->SetItem(item);

}