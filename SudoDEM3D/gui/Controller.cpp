#include "Controller.hpp"
#include "GLViewer.hpp"
#include "SudoStyle.hpp"

Controller::Controller(QWidget* parent)
    : QWidget(parent), m_viewer(nullptr), m_isRunning(false),
      m_historyIndex(-1)
{
    // Set window properties to behave like a main window
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    setWindowTitle("SudoDEM3D");
    setMinimumSize(800, 600);
    resize(1200, 800);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Apply modern styling
    setStyleSheet(getStyleSheet());

    // Create custom style for widgets (avoids complex CSS parsing issues)
    m_customStyle = new SudoStyle(this->style());

    // Create main tab widget
    m_tabs = new QTabWidget(this);
    m_tabs->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);

    // Setup all tabs
    setupSimulationTab();
    setupDisplayTab();
    setupGenerateTab();

    // Setup Python console (always visible)
    setupPythonConsole();

    // Setup the new layout
    setupLayout();

    // Connect signals
    connectSignals();

    // Setup update timer (10Hz)
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updateDisplay()));
    m_updateTimer->start(100);

    // Setup simulation timer (60Hz)
    m_simulationTimer = new QTimer(this);
    connect(m_simulationTimer, SIGNAL(timeout()), this, SLOT(runSimulationStep()));

    // Start real time timer
    m_realTimeTimer.start();
}

Controller::~Controller()
{
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
    if (m_simulationTimer) {
        m_simulationTimer->stop();
    }
}

void Controller::updateDisplay()
{
    updateTimeDisplay();
    updateStatusBar();
}

void Controller::updateTimeDisplay()
{
    // Update real time
    qint64 elapsed = m_realTimeTimer.elapsed() / 1000;
    int hours = elapsed / 3600;
    int minutes = (elapsed % 3600) / 60;
    int seconds = elapsed % 60;
    m_realTimeLabel->setText(QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0')));

    // Update virtual time, iteration count, and time step from simulation
    PyGILState_STATE gstate = PyGILState_Ensure();
    
    // Get simulation values from Python
    PyObject* mainModule = PyImport_AddModule("__main__");
    PyObject* mainDict = PyModule_GetDict(mainModule);
    
    // Get O.time (virtual time)
    PyObject* timeObj = PyRun_String("O.time", Py_eval_input, mainDict, mainDict);
    if (timeObj && PyFloat_Check(timeObj)) {
        double virtTime = PyFloat_AsDouble(timeObj);
        // Format: show seconds normally, use scientific notation for very small values
        if (virtTime < 0.001 && virtTime > 0) {
            m_virtTimeLabel->setText(QString("%1s").arg(virtTime, 0, 'e', 2));
        } else {
            m_virtTimeLabel->setText(QString("%1s").arg(virtTime, 0, 'f', 3));
        }
    }
    Py_XDECREF(timeObj);
    
    // Get O.iter (iteration count)
    PyObject* iterObj = PyRun_String("O.iter", Py_eval_input, mainDict, mainDict);
    if (iterObj && PyLong_Check(iterObj)) {
        long iterCount = PyLong_AsLong(iterObj);
        
        // Calculate iteration rate (iterations per second)
        static long lastIterCount = 0;
        static qint64 lastUpdateTime = 0;
        qint64 currentTime = m_realTimeTimer.elapsed();
        
        double iterRate = 0.0;
        if (lastUpdateTime > 0 && currentTime > lastUpdateTime) {
            double deltaTime = (currentTime - lastUpdateTime) / 1000.0; // seconds
            long deltaIter = iterCount - lastIterCount;
            iterRate = deltaIter / deltaTime;
        }
        
        // Update separate labels for count and speed
        m_iterLabel->setText(QString("#%1").arg(iterCount));
        m_iterSpeedLabel->setText(QString("%1/s").arg(iterRate, 0, 'f', 1));
        
        lastIterCount = iterCount;
        lastUpdateTime = currentTime;
    }
    Py_XDECREF(iterObj);
    
    // Get O.dt (time step) and update the edit field if not being edited
    if (!m_dtEdit->hasFocus()) {
        PyObject* dtObj = PyRun_String("O.dt", Py_eval_input, mainDict, mainDict);
        if (dtObj && PyFloat_Check(dtObj)) {
            double dt = PyFloat_AsDouble(dtObj);
            // Format time step with appropriate precision
            if (dt < 0.0001 && dt > 0) {
                m_dtEdit->setText(QString::number(dt, 'e', 2));
            } else {
                m_dtEdit->setText(QString::number(dt, 'f', 6));
            }
        }
        Py_XDECREF(dtObj);
    }
    
    PyGILState_Release(gstate);
}

void Controller::updateStatusBar()
{
    // Update body count from simulation
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyObject* mainModule = PyImport_AddModule("__main__");
    PyObject* mainDict = PyModule_GetDict(mainModule);

    // Get body count
    PyObject* bodiesObj = PyRun_String("len(O.bodies)", Py_eval_input, mainDict, mainDict);
    if (bodiesObj && PyLong_Check(bodiesObj)) {
        long bodyCount = PyLong_AsLong(bodiesObj);
        m_bodyCountLabel->setText(QString("Bodies: %1").arg(bodyCount));
    }
    Py_XDECREF(bodiesObj);

    // Calculate FPS (viewer refresh rate)
    static qint64 lastFpsTime = 0;
    static int frameCount = 0;
    qint64 currentTime = m_realTimeTimer.elapsed();
    frameCount++;

    if (lastFpsTime > 0 && (currentTime - lastFpsTime) >= 1000) { // Update every second
        double fps = frameCount * 1000.0 / (currentTime - lastFpsTime);
        m_fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));
        frameCount = 0;
        lastFpsTime = currentTime;
    } else if (lastFpsTime == 0) {
        lastFpsTime = currentTime;
    }

    // Get memory usage (using Python's psutil if available, otherwise skip)
    // Check if psutil is available using importlib
    PyRun_SimpleString(
        "import importlib\n"
        "import __main__\n"
        "spec = importlib.util.find_spec('psutil')\n"
        "if spec is not None:\n"
        "    import psutil\n"
        "    process = psutil.Process()\n"
        "    __main__._mem_mb = process.memory_info().rss / 1024 / 1024\n"
        "else:\n"
        "    __main__._mem_mb = -1\n");
    PyObject* memObj = PyDict_GetItemString(mainDict, "_mem_mb");
    if (memObj && PyFloat_Check(memObj)) {
        double memMB = PyFloat_AsDouble(memObj);
        if (memMB >= 0) {
            m_memoryLabel->setText(QString("Memory: %1 MB").arg(memMB, 0, 'f', 1));
        } else {
            m_memoryLabel->setText("Memory: --");
        }
    } else if (memObj && PyLong_Check(memObj)) {
        long memMB = PyLong_AsLong(memObj);
        if (memMB >= 0) {
            m_memoryLabel->setText(QString("Memory: %1 MB").arg(memMB));
        } else {
            m_memoryLabel->setText("Memory: --");
        }
    } else {
        m_memoryLabel->setText("Memory: --");
    }

    PyGILState_Release(gstate);
}

void Controller::setViewer(std::shared_ptr<GLViewer> viewer)
{
    m_viewer = viewer;

    // Embed the viewer widget in the viewerContainer
    if (m_viewer && m_viewerContainer) {
        // GLViewer inherits from QGLViewer which inherits from QWidget
        QWidget* viewerWidget = m_viewer.get();
        if (viewerWidget) {
            viewerWidget->setParent(m_viewerContainer);
            QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(m_viewerContainer->layout());
            if (layout) {
                layout->addWidget(viewerWidget);
            }
        }
        // Center the scene automatically only if there are bodies present
        PyGILState_STATE gstate = PyGILState_Ensure();
        PyObject* mainModule = PyImport_AddModule("__main__");
        PyObject* mainDict = PyModule_GetDict(mainModule);
        PyObject* bodiesObj = PyRun_String("len(O.bodies)", Py_eval_input, mainDict, mainDict);
        if (bodiesObj && PyLong_Check(bodiesObj)) {
            long bodyCount = PyLong_AsLong(bodiesObj);
            if (bodyCount > 0) {
                m_viewer->centerScene();
            }
        }
        Py_XDECREF(bodiesObj);
        PyGILState_Release(gstate);
        refreshDisplayCombo();
    }
}