#include "Controller.hpp"

#include <QTextDocument>
#include <iostream>
#include <mutex>
#include <chrono>
#include <algorithm>

// Include Omega for renderMutex access
#include <sudodem/core/Omega.hpp>

// Include ClassRegistry and GLDrawFunctors for dynamic display combo population
#include <sudodem/lib/factory/ClassRegistry.hpp>
#include <sudodem/pkg/common/GLDrawFunctors.hpp>

// Include Python.h after Qt headers to avoid conflicts
#ifdef slots
#undef slots
#endif
#include <Python.h>
#ifdef slots
#define slots Q_SLOTS
#endif

void Controller::setupDisplayTab()
{
    m_displayTab = new QWidget();

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(6);

    // Display preset combo for shape functors - simple styling to avoid bus error
    QLabel* presetLabel = new QLabel("Shape Display Settings:");
    
    m_displayCombo = new QComboBox();
    m_displayCombo->setToolTip("Select a shape functor to configure its display settings");

    // Display area for shape functor settings - simple styling
    m_displayArea = new QScrollArea();
    m_displayArea->setWidgetResizable(true);
    m_displayAreaWidget = new QWidget();
    QVBoxLayout* displayLayout = new QVBoxLayout(m_displayAreaWidget);
    displayLayout->setContentsMargins(0, 0, 0, 0);
    displayLayout->addStretch();
    m_displayArea->setWidget(m_displayAreaWidget);

    // Add to main layout
    mainLayout->addWidget(presetLabel);
    mainLayout->addWidget(m_displayCombo);
    mainLayout->addWidget(m_displayArea);
    mainLayout->addStretch();

    m_displayTab->setLayout(mainLayout);
    m_tabs->addTab(m_displayTab, "Display");

    // Populate the display combo with shape functors
    refreshDisplayCombo();
}

void Controller::refreshDisplayCombo()
{
    m_displayCombo->clear();
    m_displayCombo->addItem("(Select shape functor)");

    // Dynamically query ClassRegistry for all GlShapeFunctor-derived classes
    auto& registry = ClassRegistry::instance();
    std::string glShapeFunctorTypeName = typeid(GlShapeFunctor).name();

    // Get all registered classes
    std::vector<std::string> allClasses = registry.getRegisteredClasses();

    // Filter classes that inherit from GlShapeFunctor
    std::vector<std::string> shapeFunctors;
    for (const auto& className : allClasses) {
        if (registry.isInheritingFrom(className, glShapeFunctorTypeName)) {
            // Get the human-readable name if available
            std::string readableName = ClassRegistry::demangleName(className);

            // Extract just the class name (remove namespace if present)
            size_t lastColon = readableName.rfind(':');
            if (lastColon != std::string::npos && lastColon < readableName.length() - 1) {
                readableName = readableName.substr(lastColon + 1);
            }

            // Only add concrete functor classes (those starting with "Gl1_")
            if (readableName.find("Gl1_") == 0) {
                shapeFunctors.push_back(readableName);
            }
        }
    }

    // Sort alphabetically for consistent display
    std::sort(shapeFunctors.begin(), shapeFunctors.end());

    // Add to combo box
    for (const auto& functorName : shapeFunctors) {
        m_displayCombo->addItem(QString::fromStdString(functorName));
    }

    // If no functors found, add a placeholder message
    if (shapeFunctors.empty()) {
        m_displayCombo->addItem("(No shape functors available)");
    }
}

// Helper function to create display editor for selected shape functor dynamically
// Uses Python introspection to discover all configurable properties
void Controller::createFunctorEditor(const QString& functorName)
{
    // Clear existing widgets in display area
    QLayout* existingLayout = m_displayAreaWidget->layout();
    if (existingLayout) {
        while (QLayoutItem* item = existingLayout->takeAt(0)) {
            if (QWidget* widget = item->widget()) {
                widget->deleteLater();
            }
            delete item;
        }
        delete existingLayout;
    }

    // Create new layout
    QVBoxLayout* layout = new QVBoxLayout(m_displayAreaWidget);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(6);

    // Acquire renderMutex to protect renderer/functor access from simulation thread
    bool lockAcquired = false;
    try {
        std::unique_lock<std::timed_mutex> lock(Omega::instance().renderMutex, std::defer_lock);
        lockAcquired = lock.try_lock_for(std::chrono::milliseconds(50));
        if (!lockAcquired) {
            std::cerr << "Failed to acquire renderMutex in createFunctorEditor (timeout)" << std::endl;
            QLabel* errorLabel = new QLabel("Error: Could not access display settings (simulation running)");
            layout->addWidget(errorLabel);
            layout->addStretch();
            return;
        }

        // Use Python introspection to discover all configurable properties
        PyGILState_STATE gstate = PyGILState_Ensure();

        // Python script to introspect the functor and get its configurable attributes
        // Checks __dict__, dir(), and tries common attribute names for pybind11 static properties
        const char* introspectCode =
            "import __main__\n"
            "__main__._display_props = []\n"
            "__main__._display_prop_is_static = {}\n"
            "if hasattr(__main__, '_current_display_functor'):\n"
            "    functor = __main__._current_display_functor\n"
            "    cls = functor.__class__\n"
            "    seen_attrs = set()\n"
            "    # Get attributes from dir() and __dict__\n"
            "    all_attrs = set(dir(cls))\n"
            "    if hasattr(cls, '__dict__'):\n"
            "        all_attrs.update(cls.__dict__.keys())\n"
            "    # Also try common GL functor property names\n"
            "    all_attrs.update(['wire', 'Slices', 'div', 'quality', 'stripes', 'glutSlices', 'glutStacks',\n"
            "                      'normals', 'maxFn', 'signFilter', 'refRadius', 'maxRadius',\n"
            "                      'slices', 'stacks', 'maxWeakFn', 'weakFilter', 'weakScale'])\n"
            "    for attr in all_attrs:\n"
            "        if attr.startswith('_') or attr in seen_attrs:\n"
            "            continue\n"
            "        try:\n"
            "            # Get value from functor instance\n"
            "            val = None\n"
            "            is_static = False\n"
            "            \n"
            "            if hasattr(functor, attr):\n"
            "                val = getattr(functor, attr)\n"
            "            \n"
            "            # Skip if not a simple type or callable\n"
            "            if val is None or callable(val):\n"
            "                continue\n"
            "            if not isinstance(val, (bool, int, float)):\n"
            "                continue\n"
            "            \n"
            "            # Determine if static: check if attribute is in class's __dict__\n"
            "            # pybind11 static properties are stored directly in __dict__\n"
            "            if attr in cls.__dict__:\n"
            "                is_static = True\n"
            "            # Also check parent classes for static properties\n"
            "            else:\n"
            "                for base in cls.__mro__[1:]:\n"
            "                    if hasattr(base, '__dict__') and attr in base.__dict__:\n"
            "                        is_static = True\n"
            "                        break\n"
            "            \n"
            "            seen_attrs.add(attr)\n"
            "            \n"
            "            # Check for read-only by trying to set it (catch exception)\n"
            "            can_write = True\n"
            "            try:\n"
            "                # For static properties, try setting on class\n"
            "                # For instance properties, try setting on instance\n"
            "                if is_static:\n"
            "                    # Save current value, try to set same value, restore if fails\n"
            "                    old_val = getattr(cls, attr)\n"
            "                    try:\n"
            "                        setattr(cls, attr, old_val)\n"
            "                    except (AttributeError, TypeError):\n"
            "                        can_write = False\n"
            "            except:\n"
            "                pass\n"
            "            \n"
            "            if not can_write:\n"
            "                continue\n"
            "            \n"
            "            if isinstance(val, bool):\n"
            "                __main__._display_props.append(('bool', attr, val))\n"
            "                __main__._display_prop_is_static[attr] = is_static\n"
            "            elif isinstance(val, int) and not isinstance(val, bool):\n"
            "                __main__._display_props.append(('int', attr, val))\n"
            "                __main__._display_prop_is_static[attr] = is_static\n"
            "            elif isinstance(val, float):\n"
            "                __main__._display_props.append(('float', attr, val))\n"
            "                __main__._display_prop_is_static[attr] = is_static\n"
            "        except Exception:\n"
            "            pass\n";
        

        PyRun_SimpleString(introspectCode);

        // Retrieve the properties list from Python
        PyObject* mainModule = PyImport_AddModule("__main__");
        PyObject* mainDict = PyModule_GetDict(mainModule);
        PyObject* propsObj = PyDict_GetItemString(mainDict, "_display_props");

        bool hasProperties = false;

        if (propsObj && PyList_Check(propsObj)) {
            Py_ssize_t numProps = PyList_Size(propsObj);

            if (numProps > 0) {
                hasProperties = true;

                // Add a title label
                QLabel* titleLabel = new QLabel(functorName + " Settings:");
                titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #667eea;");
                layout->addWidget(titleLabel);

                // Process each property
                for (Py_ssize_t i = 0; i < numProps; i++) {
                    PyObject* propTuple = PyList_GetItem(propsObj, i);
                    if (!propTuple || !PyTuple_Check(propTuple) || PyTuple_Size(propTuple) != 3) {
                        continue;
                    }

                    PyObject* typeObj = PyTuple_GetItem(propTuple, 0);
                    PyObject* nameObj = PyTuple_GetItem(propTuple, 1);
                    PyObject* valueObj = PyTuple_GetItem(propTuple, 2);

                    if (!typeObj || !nameObj || !valueObj) continue;

                    const char* typeStr = PyUnicode_AsUTF8(typeObj);
                    const char* nameStr = PyUnicode_AsUTF8(nameObj);
                    if (!typeStr || !nameStr) continue;

                    QString propName = QString::fromUtf8(nameStr);
                    QString type = QString::fromUtf8(typeStr);

                    // Create appropriate editor based on type
                    if (type == "bool") {
                        // Boolean property - use checkbox
                        QCheckBox* checkBox = new QCheckBox(propName);
                        checkBox->setChecked(PyObject_IsTrue(valueObj));

                        connect(checkBox, &QCheckBox::stateChanged, [this, propName](int state) {
                            PyGILState_STATE gstate = PyGILState_Ensure();
                            std::string code = "import __main__\n"
                                "if hasattr(__main__, '_current_display_functor'):\n"
                                "    prop = '" + propName.toStdString() + "'\n"
                                "    if __main__._display_prop_is_static.get(prop, False):\n"
                                "        setattr(__main__._current_display_functor.__class__, prop, " + (state ? "True" : "False") + ")\n"
                                "    else:\n"
                                "        setattr(__main__._current_display_functor, prop, " + (state ? "True" : "False") + ")\n";
                            PyRun_SimpleString(code.c_str());
                            PyGILState_Release(gstate);
                        });

                        layout->addWidget(checkBox);

                    } else if (type == "int") {
                        // Integer property - use spinbox
                        QLabel* label = new QLabel(propName + ":");
                        QSpinBox* spinBox = new QSpinBox();
                        spinBox->setRange(-999999999, 999999999);
                        spinBox->setValue((int)PyLong_AsLong(valueObj));

                        connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                                [this, propName](int value) {
                            PyGILState_STATE gstate = PyGILState_Ensure();
                            std::string code = "import __main__\n"
                                "if hasattr(__main__, '_current_display_functor'):\n"
                                "    prop = '" + propName.toStdString() + "'\n"
                                "    if __main__._display_prop_is_static.get(prop, False):\n"
                                "        setattr(__main__._current_display_functor.__class__, prop, " + std::to_string(value) + ")\n"
                                "    else:\n"
                                "        setattr(__main__._current_display_functor, prop, " + std::to_string(value) + ")\n";
                            PyRun_SimpleString(code.c_str());
                            PyGILState_Release(gstate);
                        });

                        layout->addWidget(label);
                        layout->addWidget(spinBox);

                    } else if (type == "float") {
                        // Float property - use double spinbox
                        QLabel* label = new QLabel(propName + ":");
                        QDoubleSpinBox* spinBox = new QDoubleSpinBox();
                        spinBox->setRange(-1e308, 1e308);
                        spinBox->setDecimals(6);
                        spinBox->setValue(PyFloat_AsDouble(valueObj));

                        connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                                [this, propName](double value) {
                            PyGILState_STATE gstate = PyGILState_Ensure();
                            std::string code = "import __main__\n"
                                "if hasattr(__main__, '_current_display_functor'):\n"
                                "    prop = '" + propName.toStdString() + "'\n"
                                "    if __main__._display_prop_is_static.get(prop, False):\n"
                                "        setattr(__main__._current_display_functor.__class__, prop, " + std::to_string(value) + ")\n"
                                "    else:\n"
                                "        setattr(__main__._current_display_functor, prop, " + std::to_string(value) + ")\n";
                            PyRun_SimpleString(code.c_str());
                            PyGILState_Release(gstate);
                        });

                        layout->addWidget(label);
                        layout->addWidget(spinBox);
                    }
                }
            }
        }

        PyGILState_Release(gstate);

        // If no properties found, show a message
        if (!hasProperties) {
            QLabel* label = new QLabel(functorName + " has no configurable settings.");
            label->setStyleSheet("color: #5f6368; font-style: italic;");
            layout->addWidget(label);
        }

        layout->addStretch();
        // Lock will be released when unique_lock goes out of scope
    } catch (const std::exception& e) {
        std::cerr << "Exception in createFunctorEditor: " << e.what() << std::endl;
        QLabel* errorLabel = new QLabel("Error: Exception occurred while accessing display settings");
        layout->addWidget(errorLabel);
        layout->addStretch();
    }
}

// Display slots
void Controller::displayComboSlot(const QString& text)
{
    std::cout << "Display combo: " << text.toStdString() << std::endl;

    // Skip if "(Select shape functor)" is selected
    if (text == "(Select shape functor)" || text.isEmpty()) {
        return;
    }

    // Get the functor name from the combo box
    std::string functorName = text.toStdString();

    // Acquire renderMutex to protect renderer access from simulation thread
    // Use try_lock_for with a short timeout to avoid blocking the GUI
    bool lockAcquired = false;
    try {
        std::unique_lock<std::timed_mutex> lock(Omega::instance().renderMutex, std::defer_lock);
        lockAcquired = lock.try_lock_for(std::chrono::milliseconds(50));
        if (!lockAcquired) {
            std::cerr << "Failed to acquire renderMutex in displayComboSlot (timeout)" << std::endl;
            return;
        }

        // Execute Python code to find the functor and store it globally
        PyGILState_STATE gstate = PyGILState_Ensure();

        std::string findFunctorCode =
            "import __main__\n"
            "__main__._current_display_functor = None\n"
            "functor = None\n"
            "# First try to find in renderer's shapeDispatcher\n"
            "if hasattr(O, 'renderer') and O.renderer:\n"
            "    renderer = O.renderer\n"
            "    if hasattr(renderer, 'shapeDispatcher') and renderer.shapeDispatcher:\n"
            "        for cb in renderer.shapeDispatcher.functors:\n"
            "            if cb.__class__.__name__ == '" + functorName + "':\n"
            "                functor = cb\n"
            "                break\n"
            "# If not found, try to create instance from the class directly\n"
            "if functor is None:\n"
            "    import sudodem\n"
            "    if hasattr(sudodem, '" + functorName + "'):\n"
            "        cls = getattr(sudodem, '" + functorName + "')\n"
            "        functor = cls()  # Create instance for introspection\n"
            "        print('Created functor instance from class: " + functorName + "')\n"
            "if functor:\n"
            "    print('Found shape functor: ' + functor.__class__.__name__)\n"
            "    __main__._current_display_functor = functor\n"
            "else:\n"
            "    print('WARNING: Could not find functor: " + functorName + "')\n";

        PyRun_SimpleString(findFunctorCode.c_str());
        PyGILState_Release(gstate);

        // Lock will be released when unique_lock goes out of scope
    } catch (const std::exception& e) {
        std::cerr << "Exception in displayComboSlot: " << e.what() << std::endl;
        return;
    }

    // Create the editor UI for this functor
    createFunctorEditor(text);
}

void Controller::wireSlot(bool checked)
{
    std::cout << "Wireframe mode: " << (checked ? "ON" : "OFF") << std::endl;
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyRun_SimpleString(("O.renderer.wire = " + std::string(checked ? "True" : "False")).c_str());
    PyGILState_Release(gstate);
}

void Controller::shapeSlot(bool checked)
{
    std::cout << "Show shapes: " << (checked ? "ON" : "OFF") << std::endl;
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyRun_SimpleString(("O.renderer.shape = " + std::string(checked ? "True" : "False")).c_str());
    PyGILState_Release(gstate);
}

void Controller::boundSlot(bool checked)
{
    std::cout << "Show bounds: " << (checked ? "ON" : "OFF") << std::endl;
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyRun_SimpleString(("O.renderer.bound = " + std::string(checked ? "True" : "False")).c_str());
    PyGILState_Release(gstate);
}

void Controller::intrGeomSlot(bool checked)
{
    std::cout << "Show interaction geometry: " << (checked ? "ON" : "OFF") << std::endl;
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyRun_SimpleString(("O.renderer.intrGeom = " + std::string(checked ? "True" : "False")).c_str());
    PyGILState_Release(gstate);
}

void Controller::intrPhysSlot(bool checked)
{
    std::cout << "Show interaction physics: " << (checked ? "ON" : "OFF") << std::endl;
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyRun_SimpleString(("O.renderer.intrPhys = " + std::string(checked ? "True" : "False")).c_str());
    PyGILState_Release(gstate);
}
