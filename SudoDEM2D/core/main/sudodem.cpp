#include <Python.h>
#include <iostream>

#include <stdexcept>
#include <filesystem>
#include <algorithm>
#include <sstream>

#include <omp.h>

#include "sudodemcfg.h"

#ifdef SUDODEM_OPENGL
#include <QApplication>
#include "GLViewer.hpp"
#include "OpenGLManager.hpp"
#include "Controller.hpp"

// Global variables
static QApplication* g_qApp = nullptr;
#endif


using namespace std;


// Explicit template instantiation of Singleton<ClassRegistry>::self
// This must be in the executable, not in shared libraries, to avoid ODR violations
#include<sudodem/lib/factory/ClassRegistry.hpp>

template class Singleton<ClassRegistry>;


// Cross-platform search_path implementation
namespace {
    std::filesystem::path search_path(const std::filesystem::path& filename) {
        // Check if it's an absolute path
        if (filename.is_absolute()) {
            if (std::filesystem::exists(filename)) {
                return filename;
            }
            return std::filesystem::path();
        }

        // Check current directory first
        if (std::filesystem::exists(filename)) {
            return std::filesystem::absolute(filename);
        }

        // Search PATH environment variable
        const char* path_env = std::getenv("PATH");
        if (path_env) {
            std::string path_str(path_env);
            std::istringstream iss(path_str);
            std::string dir;

#ifdef _WIN32
            char delimiter = ';';
#else
            char delimiter = ':';
#endif

            while (std::getline(iss, dir, delimiter)) {
                if (!dir.empty()) {
                    std::filesystem::path full_path = std::filesystem::path(dir) / filename;
                    if (std::filesystem::exists(full_path)) {
                        return full_path;
                    }
                }
            }
        }

        return std::filesystem::path();
    }

    std::filesystem::path search_path(const std::filesystem::path& filename,
                                     const std::vector<std::filesystem::path>& searchpath) {
        // Check each path in searchpath
        for (const auto& dir : searchpath) {
            std::filesystem::path full_path = dir / filename;
            if (std::filesystem::exists(full_path)) {
                return full_path;
            }
        }
        // If not found in searchpath, fall back to PATH search
        return search_path(filename);
    }
}

void sudodem_print_help();
void sudodem_print_version();

int main( int argc, char **argv )
{
  cout<<"Welcome to SudoDEM!"<<endl;

  //setPathEnv(argc, argv);
  //set environment Variables
  filesystem::path exePath;
  if (argc > 0)
  {
    std::string exeName = argv[0];
    if (exeName.size() > 0)
    {
      std::string origPathValue = "";
      const char *getenvVal = getenv("PATH");
      if (getenvVal != NULL)
      {
        origPathValue = getenvVal;
      }
      std::string origLDPathValue = "";
      const char *getenvLDVal = getenv("LD_LIBRARY_PATH");
      if (getenvLDVal != NULL)
      {
        origLDPathValue = getenvLDVal;
      }

      std::vector<std::filesystem::path> searchpath;
      searchpath.push_back(std::filesystem::current_path());
      exePath = ::search_path(std::filesystem::path(exeName), searchpath);
      if(exePath.empty()){
        exePath = ::search_path(std::filesystem::path(exeName));
      }

      // Normalize path to handle ./ and ../
      std::filesystem::path exePathNorm = exePath.lexically_normal();
      std::string prefix= (exePathNorm.parent_path().parent_path().string());
      std::string newPathValue = origPathValue + ":" + (exePath.parent_path().string());
      std::string libPathValue = (exePath.parent_path().string()) + "/../lib/sudodem/";
      libPathValue = libPathValue + ":" + (exePath.parent_path().string()) + "/../lib/sudodem/py";
      libPathValue = libPathValue + ":" + (exePath.parent_path().string()) + "/../lib/sudodem/py/sudodem";
      libPathValue = libPathValue + ":" + (exePath.parent_path().string()) + "/../lib/sudodem/py/sudodem/qt";
      libPathValue = libPathValue + ":" + (exePath.parent_path().string()) + "/../lib/3rdlibs/py";
      std::string libLDPathValue = (exePath.parent_path().string()) + "/../lib/3rdlibs/";

      setenv("PATH", newPathValue.c_str(),1);
      setenv("PYTHONPATH",libPathValue.c_str(),1);
      setenv("LD_LIBRARY_PATH",libLDPathValue.c_str(),1);
      setenv("SUDODEM_PREFIX",prefix.c_str(),1);
    }
  }

  // options
  setenv("OMP_NUM_THREADS", "1", 1);
  omp_set_num_threads(1);
    std::vector< const char * >modulesArgs;
    bool useGUI = true;   // Whether to launch GUI (default: true)
    bool scriptOnly = false; // Whether to skip interactive REPL (default: false)

  if(argc != 1){
    for(int i=1;i < argc; i++){
      if ( strcmp(argv[i], "-v") == 0 ){//version
        sudodem_print_version();
        if(argc == 2){
          exit(1);
        }
      }else if (strcmp(argv[i], "-h") == 0 ){//help
        sudodem_print_help();
        if(argc == 2){
          exit(1);
        }
      }
      else if( strncmp(argv[i], "-j",2) == 0 ){//this first two characters are '-j'
        int len = strlen(argv[i]);
        if(len > 2){
          // -jN format (e.g., -j3, -j8)
          char cores[16];
          strncpy(cores, argv[i]+2, sizeof(cores)-1);
          cores[sizeof(cores)-1] = '\0';
          setenv("OMP_NUM_THREADS", cores, 1);
          omp_set_num_threads(atoi(cores));
        }else if ( i + 1 < argc ) {
          // -j N format (e.g., -j 3)
          i++;
          setenv("OMP_NUM_THREADS", argv[i], 1);
          omp_set_num_threads(atoi(argv[i]));
        }else{
          cout<<"Invalid thread count. Using 1 thread."<<endl;
          setenv("OMP_NUM_THREADS", "1", 1);
          omp_set_num_threads(1);
        }
      }
      else if(strcmp(argv[i], "-n") == 0){
        // no gui - still runs REPL
        useGUI = false;
      }
      else if(strcmp(argv[i], "-x") == 0){
        // script only - runs script and exits, no REPL, no GUI
        useGUI = false;
        scriptOnly = true;
      }
      else{
        modulesArgs.push_back(argv[i]);
      }
    }
  }else{
    sudodem_print_help();
  }

  //program features
  std::string feat = PRG_FEATURE;

  std::istringstream iss(feat);
  std::vector<std::string> prg_feats((std::istream_iterator<std::string>(iss)),
                                 std::istream_iterator<std::string>());

  wchar_t* program = Py_DecodeLocale(argv[0], NULL);
  Py_SetProgramName(program);

  //cout<<"omp_get_max_threads="<<omp_get_max_threads()<<endl;
  Py_Initialize();
  PyEval_InitThreads(); // Initialize threading support for Python
  PyRun_SimpleString("import sys");


  std::string cmd = "sysArgv =[";
  for(int i=0;i<argc;i++){
    cmd = cmd +"'"+argv[i]+"',";
  }
  cmd +="]";
  PyRun_SimpleString(cmd.c_str());

  cmd = "args =[";
  for(int i=0;i<modulesArgs.size();i++){
    cmd = cmd +"'"+modulesArgs[i]+"',";
  }
  cmd +="]";
  PyRun_SimpleString(cmd.c_str());

  // Setup Python environment
  PyRun_SimpleString(
    //initialization and c++ plugins import
    "import sudodem\n"
    "import sudodem.wrapper\n"
    "import sudodem.system\n"
    "import sudodem.runtime\n"
    "sys.argv=sudodem.runtime.argv=args\n"
    "from sudodem import utils\n"
    "from sudodem.utils import *\n"
    "from math import *\n"
  );

#ifdef SUDODEM_OPENGL
  if (useGUI) {
    // Run with GUI + interactive REPL (default)
    PyRun_SimpleString(
      "gui='qt6'\n"
    );
  } else {
    // -n or -x flag: no GUI
    PyRun_SimpleString(
      "gui=None\n"
    );
  }
#endif

  {
    std::string scriptOnlyCmd = "sudodem.runtime.scriptOnly = " + std::string(scriptOnly ? "True" : "False") + "\n";
    PyRun_SimpleString(scriptOnlyCmd.c_str());
  }

  PyRun_SimpleString(
"def userSession():\n"
    "    import sudodem.runtime,sys\n"
    "    if len(sys.argv)>0 and sys.argv[0].endswith('.py'):\n"
    "        def runScript(script):\n"
    "            sys.stderr.write('Running script '+script+'\\n')\n"
    "            try: exec(open(script).read(),globals())\n"
    "            except SystemExit: raise\n"
    "            except: # all other exceptions\n"
    "                import traceback\n"
    "                traceback.print_exc()\n"
    "        runScript(sys.argv[0])\n"
    "    # Check if we should start REPL\n"
    "    if sudodem.runtime.scriptOnly:\n"
    "        sys.exit(0)\n"
    "    # Use standard Python REPL\n"
    "    import code\n"
    "    banner='SudoDEM Python Shell\\\\nType exit() or quit() to exit.'\n"
    "    code.interact(banner=banner, local=globals())\n"
  );

  cout<<"Tips: Ctrl+L clears screen, Ctrl+U kills line. F12 controller, use h-key for 3d view help,F9 generator, F8 plot"<<endl;

#ifdef SUDODEM_OPENGL
  cout<<"GUI: Qt6 enabled, SUDODEM_OPENGL is defined"<<endl;

  if (useGUI) {
    // Run with Qt6 GUI on main thread (required by macOS)
    cout<<"Starting SudoDEM with Qt6 GUI..."<<endl;

    // Initialize Qt6 application on main thread (required by macOS)
    QApplication app(argc, argv);
    g_qApp = &app;

    // Initialize OpenGL manager
    OpenGLManager* glm = new OpenGLManager();

    // Create view - the GLViewer will be created but NOT shown yet
    // (GLViewer constructor defers show() via timer, which we'll override)
    int viewId = glm->waitForNewView(5.0, false); // Don't center yet
    if (viewId < 0) {
      cerr << "Failed to create OpenGL view" << endl;
      return 1;
    }

    // Get the viewer and configure it
    auto viewer = glm->views[0];
    if (viewer) {
      viewer->resize(800, 600);
      viewer->setWindowTitle("SudoDEM 2D - Qt6 Viewer");

      // Show the viewer NOW (on main thread, with proper GL context)
      // This ensures the OpenGL context is valid before any rendering happens
      viewer->makeCurrent();
      viewer->show();
      viewer->centerScene();
    }

    // Start refresh timer
    glm->emitStartTimer();

    // Create and show Controller
    cout<<"Creating Controller dialog..."<<endl;
    Controller* controller = new Controller();
    if (viewer) {
      controller->setViewer(viewer);
    }

    // Show controller immediately (no timer delay needed)
    controller->show();
    controller->raise();
    controller->activateWindow();

    cout<<"Controller dialog created and shown"<<endl;

    // Process any pending events before running script
    app.processEvents();

    // Run the script file if provided
    cout<<"Running script..."<<endl;
    int pyResult = PyRun_SimpleString(
"import sys\n"
"import sudodem.runtime\n"
"\n"
"if len(sys.argv)>0 and sys.argv[0].endswith('.py'):\n"
"    script = sys.argv[0]\n"
"    sys.stderr.write('Running script '+script+'\\\\n')\n"
"    try:\n"
"        exec(open(script).read(), globals())\n"
"    except SystemExit:\n"
"        pass\n"
"    except:\n"
"        import traceback\n"
"        traceback.print_exc()\n"
"    \n"
"    if sudodem.runtime.scriptOnly:\n"
"        sys.exit(0)\n"
    );

    if (pyResult != 0) {
      cerr << "Error running script, result: " << pyResult << endl;
    }

    cout<<"Script execution complete. Use the Python tab in the GUI for interactive commands."<<endl;

    // Enter Qt event loop - this will keep the GUI and the Python console responsive
    cout<<"Entering Qt event loop..."<<endl;
    return app.exec();
  } else {
    // No GUI mode (-n or -x flags): run script and REPL via userSession
    cout<<"Running script without GUI, starting interactive shell..."<<endl;

    int pyResult = PyRun_SimpleString("userSession()\n");

    if (pyResult != 0) {
      cerr << "Error in user session, result: " << pyResult << endl;
    }

    cout<<"User session finished."<<endl;
    return pyResult;
  }
#endif

  Py_Finalize();
  return 0;
}


void sudodem_print_help(){
  cout<<"\nUsage:\n"<<endl;
  cout<<"sudodemexe [options] user-script.py"<<endl;
  cout<<"\nOptions:\n"<<endl;
  cout<<"  -v  prints SudoDEM version"<<endl;
  cout<<"  -j  (int) Number of OpenMP threads to run (default 1). Equivalent to setting OMP_NUM_THREADS environment variable."<<endl;
  cout<<"  -n  Run without graphical interface (still starts interactive shell)"<<endl;
  cout<<"  -x  Run script and exit (no interactive shell, no GUI)"<<endl;
}

void sudodem_print_version(){
   printf("\n%s\n",PRG_VERSION);
}
