// Start of wxWidgets "Hello World" Program

#include "result_wnd.hpp"

#include "seeker_gui.hpp"
#include "seek_file.hpp"
#include "seek_folder.hpp"
#include "seek_string.hpp"
#include "utils.hpp"

#include <sstream>
#include <regex>
#include "file_io.hpp"

constexpr int TEXTBOX_PADDING = 42;
constexpr int FRAME_PADDING = 2;

//filename is not a reference due to the object lifetime
using file_func_t = std::function<void(data_thread&, const std::string&, const std::string)>;

struct search_modes_t
{
    std::string status_text;
    file_func_t func;

};

using func_map = std::unordered_map<std::string, search_modes_t>;

static func_map function_table =
{
    {"file",  { "File Search", &find_file} },
    {"folder", { "Folder Search", &find_folder} },
    {"string", { "String Search", &find_string} }
};

file_func_t* target_function = 0;

class MyApp : public wxApp
{
public:

    bool OnInit() override;

    void SwitchWindow(const std::string& wd, const std::string& search, const std::string& window_type, shared_data_thread& data)
    {
        ResultFrame* frame = 0;

        if (window_type == "file") {
            frame = new FileFrame(data, std::format("Find file: {} (\"{}\")", wd, search) );
        }else if (window_type == "folder") {
            frame = new FolderFrame(data, std::format("Find folder: {} (\"{}\")", wd, search));
        }else if (window_type == "string") {
            frame = new StringFrame(data, std::format("Find string: {} (\"{}\")", wd, search));
        }

        if (GetTopWindow()) {
            GetTopWindow()->Destroy();
        }

        SetTopWindow(frame);
        frame->Show(true);
    }

};

wxIMPLEMENT_APP(MyApp);

class MyFrame : public wxFrame
{
public:
    MyFrame();

    std::string source_loc;
    std::string open_mode;

private:
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    void OnResize(wxSizeEvent& event);

    void SearchButtonClicked(wxCommandEvent& event);
    void OnEnterPressed(wxCommandEvent& event);

    wxTextCtrl* input_box = 0;
    wxTextCtrl* regex = 0;
    wxButton* search_button = 0;
    shared_data_thread seek_data;

};

enum
{
    ID_TEXTBOX = 1,
    ID_SEARCH_BUTTON = 2
};

bool MyApp::OnInit()
{
#ifndef _DEBUG
    if (argc != 3) {

        std::wstringstream ss; 
        ss << L"bad args! (expected 3 but got " << argc << ")\n";

        for (auto i = 0; i < argc; i++)
            ss << argv[i] << '\n';

        auto str = ss.str();

        MessageBox(NULL, str.c_str(), L"error", MB_ICONERROR);
        puts("usage: <file_path> <argument>");
        system("pause");
        return 0;
    }
#endif
    constexpr auto real_w = 640;
    constexpr auto real_h = 480;

    constexpr auto w = real_w / 5;
    constexpr auto h = real_h / 5;

    constexpr auto x = real_w / 2;
    constexpr auto y = real_h / 2;

    constexpr auto diff_x = ce_abs(x - w) / 2;
    constexpr auto diff_y = ce_abs(y - h) / 2;

    constexpr auto new_x = x - diff_x;
    constexpr auto new_y = y - diff_y;

    const auto fixed = adjust_from_640x480({ new_x, new_y, w, h });

    MyFrame* frame = new MyFrame();

#ifdef _DEBUG
    frame->source_loc = "C:\\";
    frame->open_mode = "file";
#else
    frame->open_mode = argv[1];
    frame->source_loc = argv[2];

    if (frame->source_loc.empty()) {
        MessageBox(NULL, L"no target directory specified", L"error", MB_ICONERROR);
        return 0;
    }

    if (frame->source_loc.back() == '\"')
        frame->source_loc.pop_back();

    frame->source_loc += "\\";

#endif

    auto v = function_table.find(frame->open_mode);

    if (v == function_table.end()) {
        std::string s = std::format("\"{}\" is not a valid argument", frame->open_mode);

        auto wtf = convertToWideString(s);

        MessageBox(NULL, wtf.c_str(), L"error", MB_ICONERROR);


        puts(s.c_str());
        system("pause");
        return 0;
    }

    target_function = &v->second.func;


    frame->SetPosition({ fixed.x, fixed.y });
    frame->SetSize(fixed.w, fixed.h);
    
    frame->SetTitle("Working directory: " + frame->source_loc);

    frame->CreateStatusBar();
    frame->SetStatusText(function_table.find(frame->open_mode)->second.status_text);

    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "")
{

    auto size = GetSize();

    const auto flags = open_mode == "string" ? (wxTE_PROCESS_ENTER | wxTE_MULTILINE | wxHSCROLL) : wxTE_PROCESS_ENTER;

    wxPanel* panel = new wxPanel(this, wxID_ANY);

    input_box = new wxTextCtrl(panel, ID_TEXTBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, flags);
    input_box->SetHint("Search...");

    regex = new wxTextCtrl(panel, ID_TEXTBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_CENTER);
    regex->SetHint("Regex...");
    regex->SetToolTip("Filter files and directories with a regular expression");

    search_button = new wxButton(panel, ID_SEARCH_BUTTON, "Search");
    
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(wxID_EXIT);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);

    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_SIZE, &MyFrame::OnResize, this);
    
    search_button->Bind(wxEVT_BUTTON, &MyFrame::SearchButtonClicked, this);
    input_box->Bind(wxEVT_TEXT_ENTER, &MyFrame::OnEnterPressed, this);
}

void MyFrame::OnExit([[maybe_unused]] wxCommandEvent& event)
{
    Close(true);
}

void MyFrame::OnAbout([[maybe_unused]] wxCommandEvent& event)
{
    wxMessageBox("Made by xkejj\nhttps://github.com/kejjjjj/seeker_gui", "About", wxOK | wxICON_INFORMATION);
}
void MyFrame::SearchButtonClicked([[maybe_unused]] wxCommandEvent& event)
{
    auto str = input_box->GetValue();

    if (str.empty()) {
        wxMessageBox("Passed invalid input to the text field", "Error", wxOK | wxICON_ERROR);
        return;
    }

    seek_data = std::make_shared<data_thread>();
    seek_data->active = true;
    seek_data->regexStr = regex->GetValue().ToStdString();

    auto result_string = str.ToStdString();

    search_button->Disable();
    seek_data->thread = std::thread([&](){
        (*target_function)(*seek_data, source_loc, result_string);
    });

    seek_data->thread.detach();
    wxGetApp().SwitchWindow(source_loc, result_string, open_mode, seek_data);

}
void MyFrame::OnEnterPressed(wxCommandEvent& event)
{
    return SearchButtonClicked(event);
}
void MyFrame::OnResize(wxSizeEvent& event)
{
    const auto adjustment = adjust_from_480(TEXTBOX_PADDING);
    const auto frame_adjustment = adjust_from_480(FRAME_PADDING);

    auto singleLineInputBoxHeight = regex->GetSize().y;

    input_box->SetSize({ event.m_size.x, event.m_size.y - adjustment - singleLineInputBoxHeight * 2 });

    auto ibox_y = input_box->GetPosition().y + input_box->GetSize().y;

    regex->SetPosition({ regex->GetPosition().x, ibox_y + frame_adjustment });
    regex->SetSize({ event.m_size.x, regex->GetSize().y });

    auto iregex_y = regex->GetPosition().y + regex->GetSize().y;

    const auto button_size = static_cast<float>(search_button->GetSize().x) / 1.5f;
    search_button->SetPosition({ event.m_size.x / 2 - static_cast<int>(button_size), iregex_y + frame_adjustment });

    event.Skip();
}
