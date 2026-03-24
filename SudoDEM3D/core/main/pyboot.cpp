#include<sudodem/core/Omega.hpp>
#include<sudodem/lib/base/Logging.hpp>
#include<pybind11/pybind11.h>
#include<pybind11/stl.h>
#include<signal.h>

#ifdef SUDODEM_DEBUG
	void crashHandler(int sig){
	switch(sig){
		case SIGABRT:
		case SIGSEGV:
			signal(SIGSEGV,SIG_DFL); signal(SIGABRT,SIG_DFL); // prevent loops - default handlers
			cerr<<"SIGSEGV/SIGABRT handler called; gdb batch file is `"<<Omega::instance().gdbCrashBatch<<"'"<<endl;
			std::system((string("gdb -x ")+Omega::instance().gdbCrashBatch).c_str());
			raise(sig); // reemit signal after exiting gdb
			break;
		}
	}
#endif

/* Initialize sudodem, loading given plugins */
void sudodemInitialize(pybind11::list& pp, const std::string& confDir){
	// Note: PyEval_InitThreads() removed - deprecated in Python 3.9+ and does nothing
	// Python now handles thread initialization automatically

	Omega& O(Omega::instance());
	// Only init if not already initialized (wrapper import may have already done it)
	if(!O.getScene()) {
		O.init();
	}
	O.origArgv=NULL; O.origArgc=0;
	O.confDir=confDir;
	O.initTemps();

	vector<string> ppp;
	for(size_t i=0; i<pp.size(); i++) {
		ppp.push_back(pp[i].cast<string>());
	}
	Omega::instance().loadPlugins(ppp);
}
void sudodemFinalize(){ Omega::instance().cleanupTemps(); }

PYBIND11_MODULE(boot, m){
    m.def("initialize", &sudodemInitialize, "Initialize SudoDEM with plugins and config directory");
    m.def("finalize", &sudodemFinalize, "Finalize SudoDEM and cleanup");
}
