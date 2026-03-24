#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QScrollArea>
#include <QTimer>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QDateTime>
#include <QElapsedTimer>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPlainTextEdit>
#include <QEvent>
#include <QSplitter>
#include <memory>

// Forward declarations
class GLViewer;
class SudoStyle;

class Controller : public QWidget {
    Q_OBJECT

private:
    // Main layout components
    QTabWidget* m_tabs;
    QWidget* m_viewerContainer;  // Container for the viewer widget
    QPlainTextEdit* m_pythonConsole;
    QSplitter* m_rightSplitter;    // Splitter between viewer and console
    
    // Python console state
    QStringList m_commandHistory;
    int m_historyIndex;
    QString m_currentCommand;

    // === SIMULATION TAB ===
    QWidget* m_simulationTab;
    QPushButton* m_loadButton;
    QPushButton* m_saveButton;
    QPushButton* m_inspectButton;
    QLabel* m_realTimeLabel;
    QLabel* m_virtTimeLabel;
    QLabel* m_iterLabel;
    QLabel* m_iterSpeedLabel;
    QRadioButton* m_dtFixedRadio;
    QRadioButton* m_dtDynRadio;
    QLineEdit* m_dtEdit;
    QLabel* m_fileLabel;
    QPushButton* m_playButton;  // Combined Play/Pause toggle
    QPushButton* m_stepButton;
    QCheckBox* m_subStepCheckbox;
    QPushButton* m_reloadButton;
    QPushButton* m_show3dButton;
    QPushButton* m_referenceButton;
    QPushButton* m_centerButton;

    // === DISPLAY TAB ===
    QWidget* m_displayTab;
    QComboBox* m_displayCombo;
    QScrollArea* m_displayArea;
    QWidget* m_displayAreaWidget;
    QCheckBox* m_wireCheck;
    QCheckBox* m_shapeCheck;
    QCheckBox* m_boundCheck;
    QCheckBox* m_intrGeomCheck;
    QCheckBox* m_intrPhysCheck;

    // === GENERATE TAB ===
    QWidget* m_generateTab;
    QComboBox* m_generatorCombo;
    QScrollArea* m_generatorArea;
    QWidget* m_generatorAreaWidget;
    QCheckBox* m_generatorMemoryCheck;
    QLineEdit* m_generatorFilenameEdit;
    QCheckBox* m_generatorAutoCheck;
    QPushButton* m_generateButton;

    // === STATUS BAR ===
    QWidget* m_statusBar;
    QLabel* m_bodyCountLabel;
    QLabel* m_fpsLabel;
    QLabel* m_memoryLabel;

    // Timer and viewer
    QTimer* m_updateTimer;
    QTimer* m_simulationTimer;
    QElapsedTimer m_realTimeTimer;
    bool m_isRunning;
    std::shared_ptr<GLViewer> m_viewer;

private Q_SLOTS:
    // File operations
    void loadSlot();
    void saveSlot();
    void inspectSlot();
    void reloadSlot();

    // Time step
    void dtFixedSlot();
    void dtDynSlot();
    void dtEditedSlot();
    void dtEditNoupdateSlot();

    // Playback
    void playSlot();
    void pauseSlot();
    void stepSlot();
    void subStepSlot();
    void runSimulationStep();

    // View
    void show3dSlot(bool checked);
    void setReferenceSlot();
    void centerSlot();

    // Display
    void displayComboSlot(const QString& text);
    void wireSlot(bool checked);
    void shapeSlot(bool checked);
    void boundSlot(bool checked);
    void intrGeomSlot(bool checked);
    void intrPhysSlot(bool checked);

    // Generator
    void generateSlot();
    void generatorComboSlot(const QString& text);

    // Update timer
    void updateDisplay();

private:
    void setupSimulationTab();
    void setupDisplayTab();
    void setupGenerateTab();
    void setupPythonConsole();
    void setupLayout();
    void setupStatusBar();
    void connectSignals();
    void updateTimeDisplay();
    void updateStatusBar();
    void refreshDisplayCombo();
    void createFunctorEditor(const QString& functorName);
    void executePythonCommand(const QString& command);
    QString getStyleSheet();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    // Custom style for widgets (avoids complex CSS issues)
    SudoStyle* m_customStyle;

public:
    explicit Controller(QWidget* parent = nullptr);
    ~Controller();
    void setViewer(std::shared_ptr<GLViewer> viewer);
};

#endif // CONTROLLER_HPP