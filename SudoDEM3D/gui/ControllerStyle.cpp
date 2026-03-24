#include "Controller.hpp"

QString Controller::getStyleSheet()
{
    return R"(
        QWidget {
            font-family: BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;
            font-size: 13px;
            color: #2c3e50;
            background-color: #f8f9fa;
        }

        QTabWidget::pane {
            border: 1px solid #e9ecef;
            border-radius: 8px;
            background: #ffffff;
            margin-top: -1px;
        }

        QTabBar::tab {
            background: #f1f3f4;
            border: 1px solid #e9ecef;
            border-bottom: none;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            padding: 10px 20px;
            margin-right: 2px;
            font-weight: 500;
            color: #5f6368;
            min-width: 80px;
        }

        QTabBar::tab:selected {
            background: #ffffff;
            color: #1a73e8;
            font-weight: 600;
            border-bottom: 2px solid #1a73e8;
        }

        QTabBar::tab:hover:!selected {
            background: #e8eaed;
            color: #3c4043;
        }

        QPushButton {
            background: #1a73e8;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            color: #ffffff;
            font-weight: 500;
        }

        QPushButton:hover {
            background: #1557b0;
        }

        QPushButton:pressed {
            background: #174ea6;
        }

        QPushButton:disabled {
            background: #dadce0;
            color: #9aa0a6;
        }

        QLabel {
            color: #2c3e50;
            font-weight: 500;
        }

        QLineEdit {
            border: 2px solid #e9ecef;
            border-radius: 8px;
            padding: 8px 12px;
            background: #ffffff;
            color: #2c3e50;
        }

        QLineEdit:focus {
            border: 2px solid #1a73e8;
            background: #ffffff;
        }

        QComboBox {
            border: 2px solid #e9ecef;
            border-radius: 8px;
            padding: 8px 12px;
            background: #ffffff;
            color: #2c3e50;
        }

        QComboBox:hover {
            border: 2px solid #1a73e8;
        }

        QComboBox::drop-down {
            border: none;
        }

        QComboBox::down-arrow {
            width: 12px;
            height: 12px;
        }

        QCheckBox {
            spacing: 8px;
            color: #2c3e50;
        }

        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid #e9ecef;
            border-radius: 4px;
            background: #ffffff;
        }

        QCheckBox::indicator:checked {
            background: #1a73e8;
            border-color: #1a73e8;
        }

        QRadioButton {
            spacing: 8px;
            color: #2c3e50;
        }

        QRadioButton::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid #e9ecef;
            border-radius: 9px;
            background: #ffffff;
        }

        QRadioButton::indicator:checked {
            background: #1a73e8;
            border-color: #1a73e8;
        }

        QScrollArea {
            border: 1px solid #e9ecef;
            border-radius: 8px;
            background: #ffffff;
        }

        QPlainTextEdit {
            border: 1px solid #e9ecef;
            border-radius: 8px;
            background: #1e1e1e;
            color: #d4d4d4;
            padding: 8px;
        }

        QGroupBox {
            border: 2px solid #e9ecef;
            border-radius: 8px;
            margin-top: 12px;
            padding: 12px;
            font-weight: 600;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 8px;
            color: #1a73e8;
        }

        QSpinBox {
            border: 2px solid #e9ecef;
            border-radius: 8px;
            padding: 6px 10px;
            background: #ffffff;
            color: #2c3e50;
        }

        QSpinBox:focus {
            border: 2px solid #667eea;
        }

        QSpinBox::up-button, QSpinBox::down-button {
            border: none;
            width: 20px;
        }

        QSpinBox::up-button:hover, QSpinBox::down-button:hover {
            background: #e9ecef;
        }
    )";
}