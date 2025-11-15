#include "pch.h"
#include "cMain.h"
#include"FixFilePerms.h"
#include "inject.h"
#include "cApp.h"
#include "config.h"
#include "icon/icon.xpm"

enum IDs {
    INJECT_BUTTON_ID = 1,
    HIDE_BUTTON_ID,
    SELECT_BUTTON_ID,
    CUSTOM_CHECKBOX_ID,
    AUTO_CHECKBOX_ID,
};

wxBEGIN_EVENT_TABLE(cMain, wxFrame)
EVT_BUTTON(INJECT_BUTTON_ID, OnInjectButton)
EVT_BUTTON(HIDE_BUTTON_ID, OnHideButton)
EVT_BUTTON(SELECT_BUTTON_ID, OnSelectButton)

EVT_CHECKBOX(CUSTOM_CHECKBOX_ID, OnCustomCheckBox)
EVT_CHECKBOX(AUTO_CHECKBOX_ID, OnAutoCheckBox)

wxEND_EVENT_TABLE();


cMain::cMain() : wxFrame(nullptr, wxID_ANY, "Fate Client Injector", wxDefaultPosition, wxSize(297.5, 162.5), wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN) {
    wxIcon icon(icon_xpm); // icons are shit to do but the technike is actually quite nice
    this->SetIcon(icon);
    this->SetBackgroundColour(wxColour(255, 255, 255, 255));

    mainPanel = new wxPanel(this, wxID_ANY);
	btn_Inject = new wxButton(mainPanel, INJECT_BUTTON_ID, "Inject", wxPoint(5, 5), wxSize(100, 40));
	btn_Hide = new wxButton(mainPanel, HIDE_BUTTON_ID, "Hide Menu", wxPoint(5, 50), wxSize(100, 20));
    btn_Select = new wxButton(mainPanel, SELECT_BUTTON_ID, "Select", wxPoint(5, 75), wxSize(60, 20));
	txt_Name = new wxTextCtrl(mainPanel, wxID_ANY, "minecraft.windows.exe", wxPoint(110, 5), wxSize(165, 20));
	check_Custom = new wxCheckBox(mainPanel, CUSTOM_CHECKBOX_ID, "Custom Target", wxPoint(110, 30), wxSize(165, 20));
	check_Auto = new wxCheckBox(mainPanel, AUTO_CHECKBOX_ID, "Auto Inject", wxPoint(110, 50), wxSize(130, 20));
	txt_Delay = new wxTextCtrl(mainPanel, wxID_ANY, "5", wxPoint(245, 50), wxSize(30, 20), wxTE_CENTRE, wxTextValidator(wxFILTER_NUMERIC));
    txt_Delay->SetMaxLength(2);
	txt_Path = new wxTextCtrl(mainPanel, wxID_ANY, "Click \"Select\" to select the dll file", wxPoint(70, 75), wxSize(205, 20));
    CreateStatusBar(1);
    SetStatusText("Version 1.0 | Made by youtube.com/fligger", 0);

    
    check_Custom->SetValue(customProcName);
    txt_Delay->SetLabel(delaystr);
    txt_Path->SetLabel(dllPath);
    if (customProcName) {
        txt_Name->SetLabel(procName);
    }
    else {
        txt_Name->Disable();
    }
    
    taskBarControl.SetIcon(icon, "Double-Click to show Fate Injector");

    notification = new wxNotificationMessage("Fate Client Injector", "Fate Client Injector is now hidden in your system tray", this, 0);
    notification->UseTaskBarIcon(&taskBarControl);
    openDialog = new wxFileDialog(this, "Select the .dll file", working_dir, "Fate.Client.dll" ,"Dynamic link library (*.dll)|*.dll", wxFD_OPEN);
}


cMain::~cMain() {
    openDialog->Destroy();
    notification->Close();
    delete(notification);
}


void cMain::OnInjectButton(wxCommandEvent& evt) {
    std::string debug;

    DWORD procId = 0;

    procId = GetProcId(txt_Name->GetValue().mb_str());


    if (procId == 0) {

        debug = "Can't find process! | " + std::to_string(procId);
        SetStatusText(debug, 0);
        return;
    }
    wxString wxStrPath = txt_Path->GetValue();
    std::wstring wStrPath = wxStrPath.ToStdWstring(); // converting wxstr to wstr
    std::ifstream test(wStrPath.c_str()); // test if file path is valid
    if (!test) {

        debug = "Process found! | " + std::to_string(procId) + " | invalid file path";
        SetStatusText(debug, 0);
        return;
    }

    SetAccessControl(wStrPath, L"S-1-15-2-1");
    performInjection(procId, wStrPath.c_str());
    debug = "Process found! | " + std::to_string(procId) + " | valid file path | Injected!";
    SetStatusText(debug, 0);


    customProcName = check_Custom->GetValue();
    delaystr = txt_Delay->GetValue();
    dllPath = txt_Path->GetValue();
    procName = txt_Name->GetValue();

    config cfg;
    cfg.saveConfig();

	evt.Skip();
}


void cMain::OnHideButton(wxCommandEvent& evt) {

    notification->Show();
    this->Hide();

    // Fate Client Injector is now hidden in system tray
	evt.Skip();
}
void cMain::OnSelectButton(wxCommandEvent& evt) {


    if (openDialog->ShowModal() == wxID_OK) // if the user click "Open" instead of "Cancel"
    {
        txt_Path->SetLabel(openDialog->GetPath());
    }

	evt.Skip();
}

void cMain::OnCustomCheckBox(wxCommandEvent& evt) {


    if (check_Custom->IsChecked()) {
        txt_Name->Enable(true);
    }
    else {
        txt_Name->Enable(false);
        txt_Name->SetLabel("minecraft.windows.exe");
    }
    evt.Skip();
}


void cMain::OnAutoCheckBox(wxCommandEvent& evt) {
    if (check_Auto->IsChecked()) {
        txt_Name->Disable();
        txt_Path->Disable();
        txt_Delay->Disable();
        btn_Select->Disable();
        check_Custom->Disable();
        std::thread loopthread(&cMain::loopInject, this);
        loopthread.detach();
    }
    else {
        disableAutoInject();
    }

    evt.Skip();
}

void cMain::disableAutoInject() {
    check_Custom->Enable();
    if (check_Custom->IsChecked()) {
        txt_Name->Enable();
    }
    txt_Path->Enable();
    txt_Delay->Enable();
    btn_Select->Enable();
}

void cMain::UpdateStatus(const wxString& newStatus) {
    SetStatusText(newStatus, 0);
}

void cMain::UpdateDelayLabel(const wxString& newLabel) {
    txt_Delay->SetLabel(newLabel);
}

void cMain::loopInject() {
    std::string debug;

    int delay = atoi(txt_Delay->GetValue().mb_str());

    if (delay <= 1) {
        delay = 1;
        CallAfter(&cMain::UpdateDelayLabel, "1");
        debug = "AutoInject: Enabled | trying every second";
    }
    else {
        debug = "AutoInject: Enabled | trying every " + std::to_string(delay) + " seconds";
    }
    CallAfter(&cMain::UpdateStatus, debug);


    DWORD procId = 0;
    DWORD oldProcId = 0;

    delay *= 1000;

    while (check_Auto->IsChecked()) {

        while (procId == oldProcId || procId == 0) {
            Sleep(delay);
            procId = GetProcId(txt_Name->GetValue().mb_str());
            if (procId == 0) {
                debug = "AutoInject: Can't find process! | " + std::to_string(procId);
                CallAfter(&cMain::UpdateStatus, debug);
            }
            else if (procId == oldProcId) {
                debug = "AutoInject: Already Injected! | " + std::to_string(procId);
                CallAfter(&cMain::UpdateStatus, debug);
            }
            if (!check_Auto->IsChecked()) {
                return;
            }
        }
        wxString wxStrPath = txt_Path->GetValue();
        std::wstring wStrPath = wxStrPath.ToStdWstring(); // converting wxstr to wstr
        std::ifstream test(wStrPath.c_str()); // test if file path is valid
        if (!test) {
            debug = "AutoInject: Process found! | " + std::to_string(procId) + " | invalid file path";
            CallAfter(&cMain::UpdateStatus, debug);
            check_Auto->SetValue(false); // wxWidgets does that automaticly on click so we only need this here
            disableAutoInject();
            return;
        }
        SetAccessControl(wStrPath, L"S-1-15-2-1");
        performInjection(procId, wStrPath.c_str());
        debug = "Process found! | " + std::to_string(procId) + " | valid file path | Injected!";
        CallAfter(&cMain::UpdateStatus, debug);
        oldProcId = procId;

        customProcName = check_Custom->GetValue();
        delaystr = txt_Delay->GetValue();
        dllPath = txt_Path->GetValue();
        procName = txt_Name->GetValue();

        config cfg;
        cfg.saveConfig();
    }
}

