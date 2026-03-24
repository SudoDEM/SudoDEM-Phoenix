#include "Controller.hpp"
#include "GLViewer.hpp"

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

void Controller::setupSimulationTab()
{
    m_simulationTab = new QWidget();
    m_simulationTab->setStyleSheet("QWidget { background-color: #f8f9fa; }");

    // Common button styles - minimal color palette
    const QString primaryButtonStyle = R"(
        QPushButton {
            background-color: #3b82f6;
            color: #ffffff;
            font-size: 13px;
            font-weight: 500;
            border-radius: 6px;
            padding: 10px 16px;
            border: none;
        }
        QPushButton:hover {
            background-color: #2563eb;
        }
        QPushButton:pressed {
            background-color: #1d4ed8;
        }
        QPushButton:disabled {
            background-color: #cbd5e1;
            color: #64748b;
        }
    )";

    const QString secondaryButtonStyle = R"(
        QPushButton {
            background-color: #64748b;
            color: #ffffff;
            font-size: 13px;
            font-weight: 500;
            border-radius: 6px;
            padding: 10px 16px;
            border: none;
        }
        QPushButton:hover {
            background-color: #475569;
        }
        QPushButton:pressed {
            background-color: #334155;
        }
    )";

    // File operations - secondary style
    m_loadButton = new QPushButton("Load");
    m_loadButton->setEnabled(true);
    m_loadButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_loadButton->setStyleSheet(secondaryButtonStyle);

    m_saveButton = new QPushButton("Save");
    m_saveButton->setEnabled(true);
    m_saveButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    m_saveButton->setStyleSheet(secondaryButtonStyle);

    m_inspectButton = new QPushButton("Inspect");
    m_inspectButton->setStyleSheet(secondaryButtonStyle);

    QHBoxLayout* fileLayout = new QHBoxLayout();
    fileLayout->setSpacing(8);
    fileLayout->addWidget(m_loadButton);
    fileLayout->addWidget(m_saveButton);
    fileLayout->addWidget(m_inspectButton);

    // Time display - clean, minimal styling
    m_realTimeLabel = new QLabel("00:00:00");
    m_realTimeLabel->setStyleSheet(R"(
        QLabel {
            color: #1e293b;
            font-size: 16px;
            font-weight: 600;
            background-color: #ffffff;
            border: 1px solid #e2e8f0;
            border-radius: 6px;
            padding: 8px 16px;
            font-family: 'SF Mono', 'Menlo', monospace;
        }
    )");

    m_virtTimeLabel = new QLabel("0.000s");
    m_virtTimeLabel->setStyleSheet(R"(
        QLabel {
            color: #475569;
            font-size: 14px;
            font-weight: 500;
            background-color: #ffffff;
            border: 1px solid #e2e8f0;
            border-radius: 6px;
            padding: 8px 16px;
            font-family: 'SF Mono', 'Menlo', monospace;
        }
    )");

    // Iteration count and speed - separate labels on same row
    m_iterLabel = new QLabel("#0");
    m_iterLabel->setStyleSheet(R"(
        QLabel {
            color: #1e293b;
            font-size: 14px;
            font-weight: 600;
            background-color: #ffffff;
            border: 1px solid #e2e8f0;
            border-radius: 6px;
            padding: 8px 12px;
            font-family: 'SF Mono', 'Menlo', monospace;
            min-width: 60px;
        }
    )");

    m_iterSpeedLabel = new QLabel("0.0/s");
    m_iterSpeedLabel->setStyleSheet(R"(
        QLabel {
            color: #059669;
            font-size: 12px;
            font-weight: 500;
            background-color: #d1fae5;
            border: 1px solid #6ee7b7;
            border-radius: 6px;
            padding: 6px 10px;
            font-family: 'SF Mono', 'Menlo', monospace;
        }
    )");

    // Time step controls
    m_dtFixedRadio = new QRadioButton("Fixed");
    m_dtFixedRadio->setChecked(true);
    m_dtFixedRadio->setStyleSheet(R"(
        QRadioButton {
            color: #334155;
            font-weight: 500;
            spacing: 6px;
        }
        QRadioButton::indicator {
            width: 16px;
            height: 16px;
            border: 2px solid #cbd5e1;
            border-radius: 8px;
            background: #ffffff;
        }
        QRadioButton::indicator:checked {
            background-color: #3b82f6;
            border-color: #3b82f6;
        }
    )");

    m_dtDynRadio = new QRadioButton("Time stepper");
    m_dtDynRadio->setEnabled(false);
    m_dtDynRadio->setStyleSheet(R"(
        QRadioButton {
            color: #94a3b8;
            font-weight: 500;
            spacing: 6px;
        }
        QRadioButton::indicator {
            width: 16px;
            height: 16px;
            border: 2px solid #e2e8f0;
            border-radius: 8px;
            background: #f1f5f9;
        }
    )");

    m_dtEdit = new QLineEdit();
    m_dtEdit->setEnabled(false);
    m_dtEdit->setPlaceholderText("auto");
    m_dtEdit->setMaximumWidth(120);
    m_dtEdit->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #e2e8f0;
            border-radius: 6px;
            padding: 6px 10px;
            background: #f8fafc;
            color: #334155;
            font-size: 12px;
            min-width: 80px;
        }
        QLineEdit:focus {
            border: 1px solid #3b82f6;
            background: #ffffff;
        }
        QLineEdit:disabled {
            background: #f1f5f9;
            color: #94a3b8;
        }
    )");

    // Time step layout: radio buttons and input in a compact horizontal row
    QHBoxLayout* dtLayout = new QHBoxLayout();
    dtLayout->setSpacing(8);
    dtLayout->addWidget(m_dtFixedRadio);
    dtLayout->addWidget(m_dtDynRadio);
    dtLayout->addWidget(m_dtEdit);
    dtLayout->setStretchFactor(m_dtEdit, 1);

    // Time info container
    QGroupBox* timeGroup = new QGroupBox("Time");
    timeGroup->setStyleSheet(R"(
        QGroupBox {
            color: #475569;
            font-weight: 600;
            font-size: 12px;
            border: 1px solid #e2e8f0;
            border-radius: 8px;
            margin-top: 8px;
            padding: 12px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 6px;
            background-color: #f8f9fa;
        }
    )");
    // Layout for iteration count and speed side by side
    QHBoxLayout* iterLayout = new QHBoxLayout();
    iterLayout->setSpacing(8);
    iterLayout->addWidget(m_iterLabel);
    iterLayout->addWidget(m_iterSpeedLabel);
    iterLayout->addStretch();

    QFormLayout* timeLayout = new QFormLayout();
    timeLayout->setContentsMargins(0, 0, 0, 0);
    timeLayout->setHorizontalSpacing(12);
    timeLayout->setVerticalSpacing(8);
    timeLayout->addRow("Real:", m_realTimeLabel);
    timeLayout->addRow("Virtual:", m_virtTimeLabel);
    timeLayout->addRow("Iterations:", iterLayout);
    timeLayout->addRow("Time step:", dtLayout);
    timeGroup->setLayout(timeLayout);

    // File label - prominent warning style when no file
    m_fileLabel = new QLabel("⚠ No file loaded");
    m_fileLabel->setStyleSheet(R"(
        QLabel {
            color: #b45309;
            font-size: 11px;
            font-weight: 500;
            background-color: #fef3c7;
            border: 1px solid #fcd34d;
            border-radius: 4px;
            padding: 6px 12px;
        }
    )");

    QHBoxLayout* fileLabelLayout = new QHBoxLayout();
    fileLabelLayout->addStretch();
    fileLabelLayout->addWidget(m_fileLabel);
    fileLabelLayout->addStretch();

    // Playback controls - primary style
    // Single toggle button for Play/Pause
    m_playButton = new QPushButton("▶ Play");
    m_playButton->setEnabled(true);
    m_playButton->setCheckable(true);
    m_playButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_playButton->setStyleSheet(primaryButtonStyle + R"(
        QPushButton:checked {
            background-color: #f59e0b;
        }
        QPushButton:checked:hover {
            background-color: #d97706;
        }
    )");

    QHBoxLayout* playPauseLayout = new QHBoxLayout();
    playPauseLayout->setSpacing(8);
    playPauseLayout->addWidget(m_playButton);

    m_stepButton = new QPushButton("Step");
    m_stepButton->setEnabled(true);
    m_stepButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_stepButton->setStyleSheet(primaryButtonStyle);

    m_subStepCheckbox = new QCheckBox("Sub-step");
    m_subStepCheckbox->setStyleSheet(R"(
        QCheckBox {
            color: #475569;
            font-weight: 500;
            font-size: 11px;
            spacing: 6px;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border: 2px solid #cbd5e1;
            border-radius: 4px;
            background: #ffffff;
        }
        QCheckBox::indicator:checked {
            background-color: #3b82f6;
            border-color: #3b82f6;
        }
    )");

    QGridLayout* stepLayout = new QGridLayout();
    stepLayout->setContentsMargins(0, 0, 0, 0);
    stepLayout->setSpacing(6);
    stepLayout->addWidget(m_stepButton, 0, 0);
    stepLayout->addWidget(m_subStepCheckbox, 1, 0, 1, 1);

    m_reloadButton = new QPushButton("Reload");
    m_reloadButton->setEnabled(true);
    m_reloadButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_reloadButton->setStyleSheet(secondaryButtonStyle);

    QHBoxLayout* stepReloadLayout = new QHBoxLayout();
    stepReloadLayout->setSpacing(8);
    stepReloadLayout->addLayout(stepLayout);
    stepReloadLayout->addWidget(m_reloadButton);

    // Playback container
    QGroupBox* playbackGroup = new QGroupBox("Playback");
    playbackGroup->setStyleSheet(R"(
        QGroupBox {
            color: #475569;
            font-weight: 600;
            font-size: 12px;
            border: 1px solid #e2e8f0;
            border-radius: 8px;
            margin-top: 8px;
            padding: 12px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 6px;
            background-color: #f8f9fa;
        }
    )");
    QVBoxLayout* playbackLayout = new QVBoxLayout();
    playbackLayout->setSpacing(8);
    playbackLayout->addLayout(playPauseLayout);
    playbackLayout->addLayout(stepReloadLayout);
    playbackGroup->setLayout(playbackLayout);

    // View controls - secondary style
    // Toggle button for Show/Hide Viewer
    m_show3dButton = new QPushButton("Hide Viewer");
    m_show3dButton->setCheckable(true);
    m_show3dButton->setChecked(true);  // Start visible (viewer shown by default)
    m_show3dButton->setStyleSheet(R"(
        QPushButton {
            background-color: #64748b;
            color: #ffffff;
            font-size: 13px;
            font-weight: 500;
            border-radius: 6px;
            padding: 10px 16px;
            border: none;
        }
        QPushButton:hover {
            background-color: #475569;
        }
        QPushButton:checked {
            background-color: #3b82f6;
        }
        QPushButton:checked:hover {
            background-color: #2563eb;
        }
        QPushButton:pressed {
            background-color: #334155;
        }
    )");

    m_referenceButton = new QPushButton("Reference");
    m_referenceButton->setStyleSheet(secondaryButtonStyle);

    m_centerButton = new QPushButton("Center");
    m_centerButton->setStyleSheet(secondaryButtonStyle);

    QHBoxLayout* viewLayout = new QHBoxLayout();
    viewLayout->setSpacing(8);
    viewLayout->addWidget(m_show3dButton);
    viewLayout->addWidget(m_referenceButton);
    viewLayout->addWidget(m_centerButton);

    // View container
    QGroupBox* viewGroup = new QGroupBox("View");
    viewGroup->setStyleSheet(R"(
        QGroupBox {
            color: #475569;
            font-weight: 600;
            font-size: 12px;
            border: 1px solid #e2e8f0;
            border-radius: 8px;
            margin-top: 8px;
            padding: 12px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 6px;
            background-color: #f8f9fa;
        }
    )");
    viewGroup->setLayout(viewLayout);

    // Main simulation tab layout
    QVBoxLayout* mainSimLayout = new QVBoxLayout();
    mainSimLayout->setContentsMargins(12, 12, 12, 12);
    mainSimLayout->setSpacing(12);
    mainSimLayout->addLayout(fileLayout);
    mainSimLayout->addWidget(timeGroup);
    mainSimLayout->addLayout(fileLabelLayout);
    mainSimLayout->addWidget(playbackGroup);
    mainSimLayout->addWidget(viewGroup);
    mainSimLayout->addStretch();

    m_simulationTab->setLayout(mainSimLayout);
    m_tabs->addTab(m_simulationTab, "Simulation");
}

// File operations
void Controller::loadSlot()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Load Simulation", "",
        "SudoDEM Files (*.xml *.xml.bz2 *.xml.gz *.sudodem *.sudodem.gz *.sudodem.bz2 *.bin *.bin.gz *.bin.bz2)");

    if (!fileName.isEmpty()) {
        // Update label with file name and normal styling
        m_fileLabel->setText("📄 " + QFileInfo(fileName).fileName());
        m_fileLabel->setStyleSheet(R"(
            QLabel {
                color: #1e40af;
                font-size: 11px;
                font-weight: 500;
                background-color: #dbeafe;
                border: 1px solid #93c5fd;
                border-radius: 4px;
                padding: 6px 12px;
            }
        )");
        m_fileLabel->setToolTip(fileName);
        std::cout << "Loading file: " << fileName.toStdString() << std::endl;
        // Load via O.load() - the sudodem.qt wrapper just calls this anyway
        PyGILState_STATE gstate = PyGILState_Ensure();
        PyRun_SimpleString(("O.load('" + fileName.toStdString() + "')").c_str());
        PyGILState_Release(gstate);
    }
}

void Controller::saveSlot()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Simulation", "",
        "SudoDEM Files (*.xml *.xml.bz2 *.xml.gz *.sudodem *.sudodem.gz *.sudodem.bz2 *.bin *.bin.gz *.bin.bz2)");

    if (!fileName.isEmpty()) {
        // Update label with file name and normal styling
        m_fileLabel->setText("📄 " + QFileInfo(fileName).fileName());
        m_fileLabel->setStyleSheet(R"(
            QLabel {
                color: #1e40af;
                font-size: 11px;
                font-weight: 500;
                background-color: #dbeafe;
                border: 1px solid #93c5fd;
                border-radius: 4px;
                padding: 6px 12px;
            }
        )");
        m_fileLabel->setToolTip(fileName);
        std::cout << "Saving file: " << fileName.toStdString() << std::endl;
        // Save via O.save() - the sudodem.qt wrapper just calls this anyway
        PyGILState_STATE gstate = PyGILState_Ensure();
        PyRun_SimpleString(("O.save('" + fileName.toStdString() + "')").c_str());
        PyGILState_Release(gstate);
    }
}

void Controller::inspectSlot()
{
    std::cout << "Inspect button clicked" << std::endl;
    // TODO: Open inspector dialog
}

void Controller::reloadSlot()
{
    std::cout << "Reload button clicked" << std::endl;
    // TODO: Reload current file
}

// Time step slots
void Controller::dtFixedSlot()
{
    std::cout << "Fixed timestep mode" << std::endl;
    m_dtEdit->setEnabled(false);
}

void Controller::dtDynSlot()
{
    std::cout << "Dynamic timestep mode" << std::endl;
    m_dtEdit->setEnabled(true);
}

void Controller::dtEditedSlot()
{
    std::cout << "Time step edited: " << m_dtEdit->text().toStdString() << std::endl;
    // TODO: Update simulation timestep
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyRun_SimpleString(("O.dt = " + m_dtEdit->text().toStdString()).c_str());
    PyGILState_Release(gstate);
}

void Controller::dtEditNoupdateSlot()
{
    // No-op - prevents premature updates
}

// Playback slots
void Controller::playSlot()
{
    bool isChecked = m_playButton->isChecked();
    
    if (isChecked) {
        // Start simulation
        std::cout << "Play button clicked" << std::endl;
        m_isRunning = true;
        m_playButton->setText("⏸ Pause");
        m_stepButton->setEnabled(false);
        m_simulationTimer->start(16); // ~60Hz
    } else {
        // Pause simulation
        std::cout << "Pause button clicked" << std::endl;
        m_isRunning = false;
        m_playButton->setText("▶ Play");
        m_stepButton->setEnabled(true);
        m_simulationTimer->stop();
    }
}

void Controller::pauseSlot()
{
    // Deprecated: handled by playSlot toggle
}

void Controller::stepSlot()
{
    std::cout << "Step button clicked" << std::endl;
    PyGILState_STATE gstate = PyGILState_Ensure();
    if (m_subStepCheckbox->isChecked()) {
        PyRun_SimpleString("O.run(1, wait=True)");
    } else {
        PyRun_SimpleString("O.run(1)");
    }
    PyGILState_Release(gstate);
}

void Controller::runSimulationStep()
{
    if (!m_isRunning) return;
    
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyRun_SimpleString("O.run(1)");
    PyGILState_Release(gstate);
}

void Controller::subStepSlot()
{
    std::cout << "Sub-step: " << (m_subStepCheckbox->isChecked() ? "enabled" : "disabled") << std::endl;
}

// View slots
void Controller::show3dSlot(bool checked)
{
    std::cout << "Show Viewer: " << (checked ? "enabled" : "disabled") << std::endl;
    if (m_viewer) {
        if (checked) {
            m_show3dButton->setText("Hide Viewer");
            m_viewer->show();
            m_viewer->enableUpdates(true);  // Re-enable OpenGL updates
        } else {
            m_show3dButton->setText("Show Viewer");
            m_viewer->hide();
            m_viewer->enableUpdates(false);  // Disable OpenGL updates to save CPU
        }
    }
}

void Controller::setReferenceSlot()
{
    std::cout << "Set reference position" << std::endl;
    // TODO: Set camera reference position
}

void Controller::centerSlot()
{
    std::cout << "Center camera button clicked" << std::endl;
    if (m_viewer) {
        m_viewer->centerScene();
    }
}