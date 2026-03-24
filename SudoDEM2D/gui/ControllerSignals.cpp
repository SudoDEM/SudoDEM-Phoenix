#include "Controller.hpp"

void Controller::connectSignals()
{
    // File operations
    connect(m_loadButton, SIGNAL(clicked()), this, SLOT(loadSlot()));
    connect(m_saveButton, SIGNAL(clicked()), this, SLOT(saveSlot()));
    connect(m_inspectButton, SIGNAL(clicked()), this, SLOT(inspectSlot()));

    // Time step
    connect(m_dtFixedRadio, SIGNAL(clicked()), this, SLOT(dtFixedSlot()));
    connect(m_dtDynRadio, SIGNAL(clicked()), this, SLOT(dtDynSlot()));
    connect(m_dtEdit, SIGNAL(editingFinished()), this, SLOT(dtEditedSlot()));
    connect(m_dtEdit, SIGNAL(textEdited(QString)), this, SLOT(dtEditNoupdateSlot()));
    connect(m_dtEdit, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(dtEditNoupdateSlot()));

    // Playback
    connect(m_playButton, SIGNAL(clicked()), this, SLOT(playSlot()));
    connect(m_stepButton, SIGNAL(clicked()), this, SLOT(stepSlot()));
    connect(m_subStepCheckbox, SIGNAL(stateChanged(int)), this, SLOT(subStepSlot()));
    connect(m_reloadButton, SIGNAL(clicked()), this, SLOT(reloadSlot()));

    // View
    connect(m_show3dButton, SIGNAL(toggled(bool)), this, SLOT(show3dSlot(bool)));
    connect(m_referenceButton, SIGNAL(clicked()), this, SLOT(setReferenceSlot()));
    connect(m_centerButton, SIGNAL(clicked()), this, SLOT(centerSlot()));

    // Display
    connect(m_displayCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(displayComboSlot(QString)));

    // Generator
    connect(m_generatorCombo, SIGNAL(currentTextChanged(QString)),
            this, SLOT(generatorComboSlot(QString)));
    connect(m_generateButton, SIGNAL(clicked()), this, SLOT(generateSlot()));

    // Python console uses eventFilter for keyboard input
}