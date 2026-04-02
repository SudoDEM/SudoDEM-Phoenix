#include <Python.h>
#include <iostream>

#include <stdexcept>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <omp.h>
#include <codecvt>

#include "sudodemcfg.h"

#ifdef SUDODEM_OPENGL
#include <QApplication>
#include "GLViewer.hpp"
#include "OpenGLManager.hpp"
#include "Controller.hpp"

// Global variables
static QApplication* g_qApp = nullptr;
#endif

#ifdef _WIN32
  #include <process.h>  
#else
  #include <unistd.h>    
  #include <sys/wait.h>  
#endif

using namespace std;


static int sudodem_run_process(const filesystem::path& exe, const std::vector<std::string>& args) 
{
    if (!filesystem::exists(exe)) {
        cerr << "Python executable not found: " << exe.string() << endl;
        return 1;
    }

    std::vector<std::string> all;
    all.push_back(exe.string());
    for (const auto& a : args) all.push_back(a);

  #ifdef _WIN32
      std::vector<char*> argvVec;
      argvVec.reserve(all.size() + 1);
      for (auto& s : all) argvVec.push_back(const_cast<char*>(s.c_str()));
      argvVec.push_back(nullptr);

      int rc = _spawnv(_P_WAIT, exe.string().c_str(), argvVec.data());
      if (rc == -1) {
          cerr << "Failed to launch: " << exe.string() << endl;
          return 1;
      }
      return rc;
  #else
      pid_t pid = fork();
      if (pid < 0) {
          cerr << "fork() failed." << endl;
          return 1;
      }

      if (pid == 0) {
          std::vector<char*> argvVec;
          argvVec.reserve(all.size() + 1);
          for (auto& s : all) argvVec.push_back(const_cast<char*>(s.c_str()));
          argvVec.push_back(nullptr);

          execv(exe.string().c_str(), argvVec.data());
          _exit(127); // exec failed
      }

      int status = 0;
      if (waitpid(pid, &status, 0) < 0) {
          cerr << "waitpid() failed." << endl;
          return 1;
      }

      if (WIFEXITED(status)) return WEXITSTATUS(status);
      return 1;
  #endif
}

static bool sudodem_ensure_pip(const filesystem::path& pythonExe) 
{
    int rc = sudodem_run_process(
        pythonExe,
        {"-m", "pip", "--version"}
    );

    if (rc == 0) return true;

    cout << "pip not found. Trying to bootstrap pip with ensurepip..." << endl;

    rc = sudodem_run_process(
        pythonExe,
        {"-m", "ensurepip", "--upgrade"}
    );

    if (rc != 0) {
        cerr << "Failed to install pip via ensurepip." << endl;
        cerr << "Your bundled Python may not include ensurepip." << endl;
        return false;
    }

    rc = sudodem_run_process(
        pythonExe,
        {"-m", "pip", "--version"}
    );

    return (rc == 0);
}

static int sudodem_handle_pkg_command(const char* argv0,
                                      const std::string& action,
                                      const std::string& pkgName) {
    if (action != "install" && action != "uninstall") {
        cerr << "Unsupported -pkg action: " << action << endl;
        cerr << "Supported actions are: install, uninstall" << endl;
        return 1;
    }

    filesystem::path exeDir = filesystem::absolute(filesystem::path(argv0)).lexically_normal().parent_path();
    filesystem::path pythonExe;

    #ifdef _WIN32
        pythonExe = exeDir / "pythonhome" / "python.exe";
    #elif(__linux__)
        pythonExe = exeDir.parent_path() / "lib" / "3rdlibs" / "pythonhome" / "bin" / "python3";
    #elif(__APPLE__)
        pythonExe = exeDir.parent_path() / "Frameworks" / "python" / "bin" / "python3";
    #endif

    if (!filesystem::exists(pythonExe)) {
        cerr << "Cannot find bundled Python: " << pythonExe.string() << endl;
        return 1;
    }

    if (!sudodem_ensure_pip(pythonExe)) {
        return 1;
    }

    std::vector<std::string> cmd = {
        "-m", "pip",
        "--disable-pip-version-check",
        action,
        pkgName
    };

    if (action == "uninstall") {
        cmd.push_back("-y");
    }

    return sudodem_run_process(pythonExe, cmd);
}

#ifdef _WIN32
  std::wstring find_python_exe_on_path() {
      DWORD len = SearchPathW(
          nullptr,           // search current PATH
          L"python.exe",
          nullptr,
          0,
          nullptr,
          nullptr
      );

      if (len == 0) {
          throw std::runtime_error("python.exe not found on PATH");
      }

      std::vector<wchar_t> buf(len);
      DWORD ret = SearchPathW(
          nullptr,
          L"python.exe",
          nullptr,
          static_cast<DWORD>(buf.size()),
          buf.data(),
          nullptr
      );

      if (ret == 0 || ret >= buf.size()) {
          throw std::runtime_error("SearchPathW failed");
      }

      return std::wstring(buf.data());
  }

  std::wstring get_python_home_from_python_exe() {
      std::filesystem::path pyexe = find_python_exe_on_path();
      return pyexe.parent_path().wstring();
  }
#endif

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
  
  std::string exeName = argv[0];

  std::vector<std::filesystem::path> searchpath;
  searchpath.push_back(std::filesystem::current_path());
  filesystem::path exePath = ::search_path(std::filesystem::path(exeName), searchpath);
  if(exePath.empty()){
    exePath = ::search_path(std::filesystem::path(exeName));
  }
  std::filesystem::path exePathNorm = exePath.lexically_normal();

  if (argc > 0)
  {
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


      #ifdef _WIN32

          std::string prefix= (exePathNorm.parent_path().parent_path().string());
          // std::cout<<"prefix "<<prefix<<std::endl;

          std::string newPathValue = origPathValue + ";" + (exePath.parent_path().string());

          std::filesystem::path libpath_base = exePath.parent_path().parent_path();

          std::filesystem::path lib_p1 = libpath_base / "lib" / "sudodem";
          std::filesystem::path lib_p2 = libpath_base / "lib" / "sudodem" / "py";
          std::filesystem::path lib_p3 = libpath_base / "lib" / "sudodem" / "py" / "sudodem";
          std::filesystem::path lib_p4 = libpath_base / "lib" / "sudodem" / "py" / "sudodem" / "qt";
          std::filesystem::path lib_p5 = libpath_base / "lib" / "3rdlibs" / "py";

          std::string libPathValue =  lib_p1.string() + ";" +
                                      lib_p2.string() + ";" +
                                      lib_p3.string() + ";" +
                                      lib_p4.string() + ";" +
                                      lib_p5.string();

          std::string libLDPathValue = (exePath.parent_path().parent_path() / "lib" / "3rdlibs").string();

          _putenv_s("PATH", newPathValue.c_str());
          _putenv_s("PYTHONPATH",libPathValue.c_str());
          _putenv_s("LD_LIBRARY_PATH",libLDPathValue.c_str());
          _putenv_s("SUDODEM_PREFIX",prefix.c_str());

          _putenv_s("OMP_NUM_THREADS", "1");
      #else

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
      #endif

    }
  }

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

          #ifdef _WIN32
            _putenv_s("OMP_NUM_THREADS", cores);
          #else
            setenv("OMP_NUM_THREADS", cores, 1);
          #endif

          omp_set_num_threads(atoi(cores));

        }else if ( i + 1 < argc ) {
          // -j N format (e.g., -j 3)
          i++;

          #ifdef _WIN32
            _putenv_s("OMP_NUM_THREADS", argv[i]);
          #else
            setenv("OMP_NUM_THREADS", argv[i], 1);
          #endif

          omp_set_num_threads(atoi(argv[i]));
        }else{
          cout<<"Invalid thread count. Using 1 thread."<<endl;

          #ifdef _WIN32
            _putenv_s("OMP_NUM_THREADS", "1");
          #else
            setenv("OMP_NUM_THREADS", "1", 1);
          #endif

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

      }else if(strcmp(argv[i], "-pkg") == 0){
        // usage:
        //   sudodem -pkg install somepkg
        //   sudodem -pkg uninstall somepkg
        if(i + 2 >= argc){
          cerr << "Usage: sudodem -pkg <install|uninstall> <package>" << endl;
          exit(1);
        }

        std::string action = argv[++i];
        std::string pkgName = argv[++i];

        int rc = sudodem_handle_pkg_command(argv[0], action, pkgName);
        exit(rc);
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


  #if (defined(__APPLE__) && BUILD_STANDALONE_MACOS)
    std::filesystem::path this_exe = std::filesystem::weakly_canonical(std::filesystem::path(argv[0]));
    std::filesystem::path this_macos_dir = this_exe.parent_path();
    std::filesystem::path this_content_dir = this_macos_dir.parent_path();
    std::filesystem::path this_frame_dir = this_content_dir / "Frameworks";
    std::filesystem::path this_pyhome = this_content_dir / "Resources" / "python";
    std::filesystem::path this_pylib = this_pyhome / "lib" / "python3.13";
    std::filesystem::path this_py_dynlib = this_pylib / "lib-dynload";
    std::filesystem::path this_py_sitepkg = this_pylib / "site-packages";
    std::filesystem::path this_py_sudodempkg = this_content_dir / "Resources" / "sudodempython";
    
    PyConfig config;
    PyConfig_InitPythonConfig(&config);


    config.isolated = 1;
    config.use_environment = 0;
    config.site_import = 1;
    config.module_search_paths_set = 1;

    PyStatus st;

    std::string s1 = this_exe.string();
    std::wstring ws1(s1.begin(), s1.end());
    st = PyConfig_SetString(&config, &config.program_name, ws1.c_str());

    std::string s2 = this_pyhome.string();
    std::wstring ws2(s2.begin(), s2.end());
    st = PyConfig_SetString(&config, &config.home, ws2.c_str());

    std::string s3 = this_pylib.string();
    std::wstring ws3(s3.begin(), s3.end());
    st = PyWideStringList_Append(&config.module_search_paths, ws3.c_str());

    std::string s4 = this_py_dynlib.string();
    std::wstring ws4(s4.begin(), s4.end());
    st = PyWideStringList_Append(&config.module_search_paths, ws4.c_str());

    std::string s5 = this_py_sitepkg.string();
    std::wstring ws5(s5.begin(), s5.end());
    st = PyWideStringList_Append(&config.module_search_paths, ws5.c_str());

    std::string s6 = this_py_sudodempkg.string();
    std::wstring ws6(s6.begin(), s6.end());
    st = PyWideStringList_Append(&config.module_search_paths, ws6.c_str());
    
    
    st = Py_InitializeFromConfig(&config);
    PyEval_InitThreads(); // Initialize threading support for Python


    PyConfig_Clear(&config);

  #elif (defined(__linux__) && BUILD_STANDALONE_LINUX)

    std::filesystem::path this_exe = std::filesystem::weakly_canonical(std::filesystem::path(argv[0]));
    std::filesystem::path this_bin_dir = this_exe.parent_path();
    std::filesystem::path this_lib_dir = this_bin_dir.parent_path() / "lib";
    std::filesystem::path this_3rdlib_dir = this_bin_dir.parent_path() / "lib" / "3rdlibs";

    std::filesystem::path this_pyhome = this_3rdlib_dir / "pythonhome";
    std::filesystem::path this_pylib = this_pyhome / "lib" / "python3.13";
    std::filesystem::path this_py_dynlib = this_pylib / "lib-dynload";
    std::filesystem::path this_py_sitepkg = this_pylib / "site-packages";
    std::filesystem::path this_py_sudodempkg = this_lib_dir / "sudodem" / "py";
    
    PyConfig config;
    PyConfig_InitPythonConfig(&config);


    config.isolated = 1;
    config.use_environment = 0;
    config.site_import = 1;
    config.module_search_paths_set = 1;

    PyStatus st;

    std::string s1 = this_exe.string();
    std::wstring ws1(s1.begin(), s1.end());
    st = PyConfig_SetString(&config, &config.program_name, ws1.c_str());

    std::string s2 = this_pyhome.string();
    std::wstring ws2(s2.begin(), s2.end());
    st = PyConfig_SetString(&config, &config.home, ws2.c_str());

    std::string s3 = this_pylib.string();
    std::wstring ws3(s3.begin(), s3.end());
    st = PyWideStringList_Append(&config.module_search_paths, ws3.c_str());

    std::string s4 = this_py_dynlib.string();
    std::wstring ws4(s4.begin(), s4.end());
    st = PyWideStringList_Append(&config.module_search_paths, ws4.c_str());

    std::string s5 = this_py_sitepkg.string();
    std::wstring ws5(s5.begin(), s5.end());
    st = PyWideStringList_Append(&config.module_search_paths, ws5.c_str());

    std::string s6 = this_py_sudodempkg.string();
    std::wstring ws6(s6.begin(), s6.end());
    st = PyWideStringList_Append(&config.module_search_paths, ws6.c_str());
    
    
    st = Py_InitializeFromConfig(&config);
    PyEval_InitThreads(); // Initialize threading support for Python


    PyConfig_Clear(&config);

  #elif (defined(_WIN32) && BUILD_STANDALONE_WIN)

    std::filesystem::path this_exe = std::filesystem::weakly_canonical(std::filesystem::path(argv[0]));
    std::filesystem::path this_bin_dir = this_exe.parent_path();
    std::filesystem::path this_lib_dir = this_bin_dir.parent_path() / "lib";

    std::filesystem::path this_pyhome = this_bin_dir / "pythonhome";
    std::filesystem::path this_pylib = this_pyhome / "Lib";
    std::filesystem::path this_py_dynlib = this_pyhome / "DLLs";
    std::filesystem::path this_py_sitepkg = this_pyhome / "Lib" / "site-packages";
    std::filesystem::path this_py_sudodempkg = this_lib_dir / "sudodem" / "py";
    
    PyConfig config;
    PyConfig_InitPythonConfig(&config);


    config.isolated = 1;
    config.use_environment = 0;
    config.site_import = 1;
    config.module_search_paths_set = 1;

    PyStatus st;

    std::string s1 = this_exe.string();
    std::wstring ws1(s1.begin(), s1.end());
    st = PyConfig_SetString(&config, &config.program_name, ws1.c_str());

    std::string s2 = this_pyhome.string();
    std::wstring ws2(s2.begin(), s2.end());
    st = PyConfig_SetString(&config, &config.home, ws2.c_str());

    std::string s3 = this_pylib.string();
    std::wstring ws3(s3.begin(), s3.end());
    st = PyWideStringList_Append(&config.module_search_paths, ws3.c_str());

    std::string s4 = this_py_dynlib.string();
    std::wstring ws4(s4.begin(), s4.end());
    st = PyWideStringList_Append(&config.module_search_paths, ws4.c_str());

    std::string s5 = this_py_sitepkg.string();
    std::wstring ws5(s5.begin(), s5.end());
    st = PyWideStringList_Append(&config.module_search_paths, ws5.c_str());

    std::string s6 = this_py_sudodempkg.string();
    std::wstring ws6(s6.begin(), s6.end());
    st = PyWideStringList_Append(&config.module_search_paths, ws6.c_str());
    
    
    st = Py_InitializeFromConfig(&config);
    PyEval_InitThreads(); // Initialize threading support for Python


    PyConfig_Clear(&config);

  #elif _WIN32
    std::wstring py_home_forsudodem = get_python_home_from_python_exe();

    if (!std::filesystem::exists(py_home_forsudodem))
    {
      throw std::runtime_error("Python home path does not exist.");
    }

    _wputenv_s(L"PYTHONHOME", py_home_forsudodem.c_str());
    Py_SetPythonHome(py_home_forsudodem.data());

    std::filesystem::path py_dll_basepath(py_home_forsudodem);


    if (!SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS |
                                  LOAD_LIBRARY_SEARCH_USER_DIRS)) {
        throw std::runtime_error("SetDefaultDllDirectories failed.");
    }

    auto addDir = [](const std::filesystem::path& p) {
        if (std::filesystem::exists(p)) {
            if (AddDllDirectory(p.c_str()) == nullptr) {
                throw std::runtime_error("AddDllDirectory failed.");
            }
        }
    };

    addDir(py_dll_basepath);
    addDir(py_dll_basepath / L"DLLs");
    addDir(py_dll_basepath / L"Library" / L"bin");
    addDir(exePathNorm.parent_path());

    std::cout<<"path "<<exePathNorm.parent_path()<<std::endl;



  #else
    Py_Initialize();
    PyEval_InitThreads(); // Initialize threading support for Python

  #endif


  std::string cmd = "sysArgv =[";
  for(int i=0;i<argc;i++){
    std::string thisarg = argv[i];
    std::replace(thisarg.begin(), thisarg.end(), '\\', '/');
    cmd = cmd +"'"+thisarg+"',";
  }
  cmd +="]";
  PyRun_SimpleString(cmd.c_str());

  cmd = "args =[";
  for(int i=0;i<modulesArgs.size();i++){
    std::string thisarg = modulesArgs[i];
    std::replace(thisarg.begin(), thisarg.end(), '\\', '/');
    cmd = cmd +"'"+thisarg+"',";
  }
  cmd +="]";
  PyRun_SimpleString(cmd.c_str());


  // Setup Python environment
  PyRun_SimpleString(
    //initialization and c++ plugins import
    "import sys\n"
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

    #if (defined(_WIN32) && BUILD_STANDALONE_WIN) || \
        (defined(__linux__) && BUILD_STANDALONE_LINUX) || \
        (defined(__APPLE__) && BUILD_STANDALONE_MACOS)

      std::filesystem::path qtPluginPath;
      
      #if defined(_WIN32)
          qtPluginPath = exePath.parent_path().parent_path();
      #elif defined(__APPLE__)
          qtPluginPath = exePath.parent_path().parent_path() / "PlugIns";
      #else
          qtPluginPath = exePath.parent_path().parent_path() / "plugins";
      #endif

      QCoreApplication::addLibraryPath(QString::fromStdWString(qtPluginPath.wstring()));

    #else
      const char* qtRootEnv = std::getenv("Qt6_ROOT");

      if (!qtRootEnv) 
      {
        throw std::runtime_error("Error: environment variable Qt6_ROOT is not set.");
      }

      namespace fs = std::filesystem;
      fs::path qtRoot = fs::u8path(qtRootEnv);
      fs::path pluginRoot = qtRoot / "plugins";

      if (!fs::exists(pluginRoot)) {
          throw std::runtime_error("Qt plugin path does not exist: " + pluginRoot.string());
      }

      QCoreApplication::addLibraryPath(QString::fromStdWString(pluginRoot.wstring()));
    #endif

    
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
