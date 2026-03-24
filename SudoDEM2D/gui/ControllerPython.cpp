#include "Controller.hpp"

#include <QTextDocument>
#include <iostream>

// Include Python.h after Qt headers to avoid conflicts
#ifdef slots
#undef slots
#endif
#include <Python.h>
#ifdef slots
#define slots Q_SLOTS
#endif

void Controller::executePythonCommand(const QString& command)
{
    PyGILState_STATE gstate = PyGILState_Ensure();
    
    // Create a Python string object with the command
    PyObject* cmdObj = PyUnicode_FromString(command.toStdString().c_str());
    
    // Set it as a variable in __main__ module
    PyObject* mainModule = PyImport_ImportModule("__main__");
    PyObject* mainDict = nullptr;
    if (mainModule) {
        mainDict = PyModule_GetDict(mainModule);
        PyObject_SetAttrString(mainModule, "_sudodem_cmd", cmdObj);
        Py_DECREF(mainModule);
    }
    Py_DECREF(cmdObj);
    
    if (!mainDict) {
        PyGILState_Release(gstate);
        return;
    }
    
    // Redirect stdout/stderr
    PyObject* sysModule = PyImport_ImportModule("sys");
    PyObject* ioModule = PyImport_ImportModule("io");
    if (!sysModule || !ioModule) {
        Py_XDECREF(sysModule);
        Py_XDECREF(ioModule);
        PyGILState_Release(gstate);
        return;
    }
    
    PyObject* StringIOClass = PyObject_GetAttrString(ioModule, "StringIO");
    PyObject* stdoutBuffer = PyObject_CallObject(StringIOClass, nullptr);
    PyObject* stderrBuffer = PyObject_CallObject(StringIOClass, nullptr);
    Py_DECREF(StringIOClass);
    
    PyObject* oldStdout = PyObject_GetAttrString(sysModule, "stdout");
    PyObject* oldStderr = PyObject_GetAttrString(sysModule, "stderr");
    
    PyObject_SetAttrString(sysModule, "stdout", stdoutBuffer);
    PyObject_SetAttrString(sysModule, "stderr", stderrBuffer);
    
    // Execute the command
    std::string execCode = 
        "try:\n"
        "    result = eval(_sudodem_cmd, globals())\n"
        "    if result is not None:\n"
        "        print(repr(result))\n"
        "except:\n"
        "    try:\n"
        "        exec(_sudodem_cmd, globals())\n"
        "    except Exception as e:\n"
        "        print(f\"Error: {e}\")\n";
    
    PyRun_String(execCode.c_str(), Py_file_input, mainDict, mainDict);
    
    // Restore stdout/stderr
    PyObject_SetAttrString(sysModule, "stdout", oldStdout);
    PyObject_SetAttrString(sysModule, "stderr", oldStderr);
    
    // Get captured output
    PyObject* stdoutGetvalue = PyObject_CallMethod(stdoutBuffer, "getvalue", nullptr);
    PyObject* stderrGetvalue = PyObject_CallMethod(stderrBuffer, "getvalue", nullptr);
    
    // Combine and display output
    if (stdoutGetvalue) {
        const char* out = PyUnicode_AsUTF8(stdoutGetvalue);
        if (out) {
            m_pythonConsole->moveCursor(QTextCursor::End);
            m_pythonConsole->insertPlainText(QString::fromUtf8(out));
        }
    }
    if (stderrGetvalue) {
        const char* err = PyUnicode_AsUTF8(stderrGetvalue);
        if (err) {
            m_pythonConsole->moveCursor(QTextCursor::End);
            m_pythonConsole->insertPlainText(QString::fromUtf8(err));
        }
    }
    
    // Cleanup
    Py_XDECREF(stdoutGetvalue);
    Py_XDECREF(stderrGetvalue);
    Py_XDECREF(oldStdout);
    Py_XDECREF(oldStderr);
    Py_XDECREF(stdoutBuffer);
    Py_XDECREF(stderrBuffer);
    Py_XDECREF(sysModule);
    Py_XDECREF(ioModule);
    
    PyGILState_Release(gstate);
}

// Event filter for Python console
bool Controller::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_pythonConsole && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            // Get the current line
            QTextCursor cursor = m_pythonConsole->textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.select(QTextCursor::LineUnderCursor);
            QString line = cursor.selectedText();
            
            // Remove prompt prefix if present
            QString command = line;
            if (command.startsWith(">>> ") || command.startsWith("... ")) {
                command = command.mid(4);
            }
            
            m_pythonConsole->moveCursor(QTextCursor::End);
            m_pythonConsole->insertPlainText("\n");

            if (!command.trimmed().isEmpty()) {
                // Execute the command
                std::cout << "Python executing: " << command.toStdString() << std::endl;
                executePythonCommand(command);

                // Add to history
                if (m_commandHistory.isEmpty() || m_commandHistory.last() != command) {
                    m_commandHistory.append(command);
                }
                m_historyIndex = m_commandHistory.size();
            }

            m_pythonConsole->insertPlainText(">>> ");
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Up) {
            // Navigate command history up
            if (!m_commandHistory.isEmpty() && m_historyIndex > 0) {
                m_historyIndex--;
                QTextCursor cursor = m_pythonConsole->textCursor();
                cursor.movePosition(QTextCursor::End);
                cursor.select(QTextCursor::LineUnderCursor);
                QString line = cursor.selectedText();
                
                if (line.startsWith(">>> ") || line.startsWith("... ")) {
                    cursor.removeSelectedText();
                    cursor.insertText(">>> " + m_commandHistory[m_historyIndex]);
                }
            }
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Down) {
            // Navigate command history down
            if (!m_commandHistory.isEmpty() && m_historyIndex < m_commandHistory.size() - 1) {
                m_historyIndex++;
                QTextCursor cursor = m_pythonConsole->textCursor();
                cursor.movePosition(QTextCursor::End);
                cursor.select(QTextCursor::LineUnderCursor);
                QString line = cursor.selectedText();
                
                if (line.startsWith(">>> ") || line.startsWith("... ")) {
                    cursor.removeSelectedText();
                    cursor.insertText(">>> " + m_commandHistory[m_historyIndex]);
                }
            } else if (m_historyIndex == m_commandHistory.size() - 1) {
                // Clear to empty prompt
                m_historyIndex = m_commandHistory.size();
                QTextCursor cursor = m_pythonConsole->textCursor();
                cursor.movePosition(QTextCursor::End);
                cursor.select(QTextCursor::LineUnderCursor);
                QString line = cursor.selectedText();
                
                if (line.startsWith(">>> ") || line.startsWith("... ")) {
                    cursor.removeSelectedText();
                    cursor.insertText(">>> ");
                }
            }
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}