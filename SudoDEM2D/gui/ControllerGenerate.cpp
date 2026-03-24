#include "Controller.hpp"

#include <iostream>

void Controller::setupGenerateTab()
{
    m_generateTab = new QWidget();

    m_generatorCombo = new QComboBox();
    
    m_generatorArea = new QScrollArea();
    m_generatorArea->setWidgetResizable(true);
    m_generatorArea->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_generatorAreaWidget = new QWidget();
    m_generatorAreaWidget->setMinimumSize(500, 500);
    m_generatorArea->setWidget(m_generatorAreaWidget);

    m_generatorMemoryCheck = new QCheckBox("memory slot");
    m_generatorMemoryCheck->setEnabled(false);
    
    m_generatorFilenameEdit = new QLineEdit("/tmp/scene.sudodem.gz");
    
    m_generatorAutoCheck = new QCheckBox("open automatically");
    m_generatorAutoCheck->setChecked(true);
    
    m_generateButton = new QPushButton("Generate");

    QGridLayout* generatorBottomLayout = new QGridLayout();
    generatorBottomLayout->addWidget(m_generatorMemoryCheck, 0, 0);
    generatorBottomLayout->addWidget(m_generatorFilenameEdit, 0, 1, 1, 2);
    generatorBottomLayout->addWidget(m_generatorAutoCheck, 1, 0, 1, 2);
    generatorBottomLayout->addWidget(m_generateButton, 1, 2);

    QGridLayout* generateLayout = new QGridLayout();
    generateLayout->setSpacing(0);
    generateLayout->addWidget(m_generatorCombo, 0, 0);
    generateLayout->addWidget(m_generatorArea, 1, 0);
    generateLayout->addLayout(generatorBottomLayout, 2, 0);

    m_generateTab->setLayout(generateLayout);
    m_tabs->addTab(m_generateTab, "Generate");
}

// Generator slots
void Controller::generateSlot()
{
    std::cout << "Generate button clicked" << std::endl;
    // TODO: Execute generator
}

void Controller::generatorComboSlot(const QString& text)
{
    std::cout << "Generator combo: " << text.toStdString() << std::endl;
    // TODO: Update generator area
}