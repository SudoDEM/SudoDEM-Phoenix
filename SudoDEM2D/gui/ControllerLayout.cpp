#include "Controller.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>

void Controller::setupPythonConsole()
{
    m_pythonConsole = new QPlainTextEdit();
    m_pythonConsole->setReadOnly(false);
    QFont pythonFont;
    pythonFont.setFamily("'SF Mono', 'Menlo', 'Monaco', 'Courier New', monospace");
    pythonFont.setStyleHint(QFont::Monospace);
    pythonFont.setPointSize(12);
    m_pythonConsole->setFont(pythonFont);
    
    // Apply fancy console styling
    m_pythonConsole->setStyleSheet(R"(
        QPlainTextEdit {
            background: #1e1e1e;
            color: #d4d4d4;
            border: none;
            border-radius: 6px;
            padding: 8px;
            selection-background-color: #667eea;
            selection-color: #ffffff;
        }
        QPlainTextEdit:focus {
            border: 2px solid #667eea;
        }
    )");
    
    // Set height for the console
    m_pythonConsole->setMinimumHeight(200);
    m_pythonConsole->setMaximumHeight(300);
    
    // Initialize with simple welcome message and prompt
    QString welcomeMsg = QString(
        "SudoDEM2D Python Console\n"
        "Type Python commands and press Enter\n"
        "Use ↑/↓ arrow keys for command history\n\n"
        ">>> "
    );
    m_pythonConsole->setPlainText(welcomeMsg);
    
    // Install event filter to handle keyboard input
    m_pythonConsole->installEventFilter(this);
}

void Controller::setupLayout()
{
    // Create viewer container with neutral dark background
    m_viewerContainer = new QWidget();
    m_viewerContainer->setStyleSheet(R"(
        QWidget#viewerContainer {
            background: #2d2d2d;
            border-radius: 8px;
            border: 1px solid #404040;
        }
    )");
    m_viewerContainer->setObjectName("viewerContainer");
    
    QVBoxLayout* viewerLayout = new QVBoxLayout(m_viewerContainer);
    viewerLayout->setContentsMargins(8, 8, 8, 8);
    viewerLayout->setSpacing(4);
    
    m_viewerContainer->setLayout(viewerLayout);
    
    // Create Python console container with subtle styling
    QWidget* consoleContainer = new QWidget();
    consoleContainer->setStyleSheet(R"(
        QWidget#consoleContainer {
            background: #1e1e1e;
            border-radius: 8px;
            border: 1px solid #404040;
        }
    )");
    consoleContainer->setObjectName("consoleContainer");
    
    QVBoxLayout* consoleLayout = new QVBoxLayout(consoleContainer);
    consoleLayout->setContentsMargins(8, 8, 8, 8);
    consoleLayout->setSpacing(4);
    consoleLayout->addWidget(m_pythonConsole);
    
    // Create splitter for resizable viewer/console area
    m_rightSplitter = new QSplitter(Qt::Vertical, this);
    m_rightSplitter->setHandleWidth(6);
    m_rightSplitter->setChildrenCollapsible(false);
    m_rightSplitter->setStyleSheet(R"(
        QSplitter::handle:vertical {
            background: #4a4a4a;
            height: 6px;
            border-radius: 3px;
            margin: 2px 0;
        }
        QSplitter::handle:vertical:hover {
            background: #667eea;
        }
    )");
    
    // Add viewer and console to splitter
    m_rightSplitter->addWidget(m_viewerContainer);
    m_rightSplitter->addWidget(consoleContainer);
    
    // Set initial sizes (viewer 65%, console 35%)
    QList<int> sizes;
    sizes << 500 << 280;
    m_rightSplitter->setSizes(sizes);
    
    // Create main horizontal layout for content area
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(8);
    contentLayout->addWidget(m_tabs, 1);      // Left side: tabs (~25%)
    contentLayout->addWidget(m_rightSplitter, 3);  // Right side: viewer + console (~75%)

    // Create status bar
    setupStatusBar();

    // Create outer vertical layout to hold content + status bar
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 4);
    mainLayout->setSpacing(4);
    mainLayout->addLayout(contentLayout, 1);  // Content takes most space
    mainLayout->addWidget(m_statusBar);       // Status bar at bottom
    setLayout(mainLayout);
}

void Controller::setupStatusBar()
{
    // Create status bar container
    m_statusBar = new QWidget();
    m_statusBar->setStyleSheet(R"(
        QWidget {
            background: #f1f3f4;
            border-top: 1px solid #dadce0;
            padding: 4px 8px;
        }
        QLabel {
            color: #5f6368;
            font-size: 11px;
            font-weight: 500;
            padding: 2px 8px;
            background: transparent;
        }
    )");

    QHBoxLayout* statusLayout = new QHBoxLayout(m_statusBar);
    statusLayout->setContentsMargins(8, 4, 8, 4);
    statusLayout->setSpacing(16);

    // Body count label
    m_bodyCountLabel = new QLabel("Bodies: 0");
    m_bodyCountLabel->setToolTip("Total number of bodies in the simulation");

    // FPS label
    m_fpsLabel = new QLabel("FPS: --");
    m_fpsLabel->setToolTip("Frames per second (viewer refresh rate)");

    // Memory usage label
    m_memoryLabel = new QLabel("Memory: -- MB");
    m_memoryLabel->setToolTip("Approximate memory usage");

    // Add stretch to push labels to the left
    statusLayout->addWidget(m_bodyCountLabel);
    statusLayout->addWidget(m_fpsLabel);
    statusLayout->addWidget(m_memoryLabel);
    statusLayout->addStretch();

}