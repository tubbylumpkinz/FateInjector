#include "pch.h"
#include "cApp.h"
#include "config.h"

wxIMPLEMENT_APP(cApp);

cApp::cApp() {

}

cApp::~cApp() {

}
bool cApp::OnInit() {

    char* shut_up_VS = _getcwd(working_dir, FILENAME_MAX);
    config cfg;
    if (cfg.loadConfig()) { //if error
        std::cout << "can't find file\n";
        cfg.saveConfig();
    }

	m_frame1 = new cMain();
	m_frame1->Show();
	return true;
}

