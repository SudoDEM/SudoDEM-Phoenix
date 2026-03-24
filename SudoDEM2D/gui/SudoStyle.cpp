#include "SudoStyle.hpp"

#include <QPainter>
#include <QStyleOption>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QScrollArea>

SudoStyle::SudoStyle(QStyle* baseStyle)
    : QProxyStyle(baseStyle)
{
}

void SudoStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                              QPainter* painter, const QWidget* widget) const
{
    // Custom drawing for primitive elements
    switch (element) {
        case PE_PanelButtonCommand: {
            // Custom button drawing with rounded corners
            QRect rect = option->rect;
            bool hovered = option->state & State_MouseOver;
            bool pressed = option->state & State_Sunken;
            bool enabled = option->state & State_Enabled;

            painter->setRenderHint(QPainter::Antialiasing);

            // Button background color
            QColor bgColor;
            if (!enabled) {
                bgColor = QColor("#dadce0");
            } else if (pressed) {
                bgColor = QColor("#174ea6");
            } else if (hovered) {
                bgColor = QColor("#1557b0");
            } else {
                bgColor = QColor("#1a73e8");
            }

            painter->setBrush(bgColor);
            painter->setPen(Qt::NoPen);
            painter->drawRoundedRect(rect, 6, 6);
            return;
        }

        case PE_IndicatorCheckBox: {
            // Custom checkbox drawing
            QRect rect = option->rect.adjusted(2, 2, -2, -2);
            bool checked = option->state & State_On;
            bool hovered = option->state & State_MouseOver;

            painter->setRenderHint(QPainter::Antialiasing);

            // Border
            QColor borderColor = hovered ? QColor("#5a67d8") : QColor("#667eea");
            painter->setPen(QPen(borderColor, 2));

            // Background
            if (checked) {
                painter->setBrush(QColor("#667eea"));
            } else {
                painter->setBrush(Qt::white);
            }

            painter->drawRoundedRect(rect, 4, 4);

            // Checkmark
            if (checked) {
                painter->setPen(QPen(Qt::white, 2));
                QPointF points[3] = {
                    QPointF(rect.left() + rect.width() * 0.25, rect.center().y()),
                    QPointF(rect.left() + rect.width() * 0.45, rect.bottom() - rect.height() * 0.3),
                    QPointF(rect.right() - rect.width() * 0.25, rect.top() + rect.height() * 0.25)
                };
                painter->drawPolyline(points, 3);
            }
            return;
        }

        case PE_IndicatorRadioButton: {
            // Custom radio button drawing
            QRect rect = option->rect.adjusted(2, 2, -2, -2);
            bool checked = option->state & State_On;
            bool hovered = option->state & State_MouseOver;

            painter->setRenderHint(QPainter::Antialiasing);

            // Border
            QColor borderColor = hovered ? QColor("#5a67d8") : QColor("#667eea");
            painter->setPen(QPen(borderColor, 2));
            painter->setBrush(Qt::white);
            painter->drawEllipse(rect);

            // Inner circle
            if (checked) {
                painter->setBrush(QColor("#667eea"));
                painter->setPen(Qt::NoPen);
                QRect innerRect = rect.adjusted(rect.width() / 4, rect.height() / 4,
                                                -rect.width() / 4, -rect.height() / 4);
                painter->drawEllipse(innerRect);
            }
            return;
        }

        default:
            break;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void SudoStyle::drawControl(ControlElement element, const QStyleOption* option,
                           QPainter* painter, const QWidget* widget) const
{
    switch (element) {
        case CE_ComboBoxLabel: {
            // Draw combo box text
            if (const QStyleOptionComboBox* cb = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
                QRect textRect = subControlRect(CC_ComboBox, cb, SC_ComboBoxEditField, widget);
                painter->setPen(cb->palette.text().color());
                painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, cb->currentText);
            }
            return;
        }

        default:
            break;
    }

    QProxyStyle::drawControl(element, option, painter, widget);
}

int SudoStyle::pixelMetric(PixelMetric metric, const QStyleOption* option,
                          const QWidget* widget) const
{
    switch (metric) {
        case PM_ButtonMargin:
            return 8;
        case PM_ButtonDefaultIndicator:
            return 0;
        case PM_IndicatorWidth:
        case PM_IndicatorHeight:
            return 18;
        case PM_ExclusiveIndicatorWidth:
        case PM_ExclusiveIndicatorHeight:
            return 18;
        case PM_ComboBoxFrameWidth:
            return 8;
        default:
            return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

QSize SudoStyle::sizeFromContents(ContentsType type, const QStyleOption* option,
                                 const QSize& size, const QWidget* widget) const
{
    QSize newSize = QProxyStyle::sizeFromContents(type, option, size, widget);

    switch (type) {
        case CT_PushButton:
            newSize.setHeight(qMax(newSize.height(), 32));
            newSize.rwidth() += 20;
            break;
        case CT_ComboBox:
            newSize.setHeight(qMax(newSize.height(), 32));
            newSize.rwidth() += 20;
            break;
        case CT_LineEdit:
            newSize.setHeight(qMax(newSize.height(), 32));
            break;
        default:
            break;
    }

    return newSize;
}

QRect SudoStyle::subElementRect(SubElement element, const QStyleOption* option,
                               const QWidget* widget) const
{
    QRect rect = QProxyStyle::subElementRect(element, option, widget);

    switch (element) {
        case SE_CheckBoxIndicator:
            rect.setSize(QSize(18, 18));
            break;
        case SE_RadioButtonIndicator:
            rect.setSize(QSize(18, 18));
            break;
        default:
            break;
    }

    return rect;
}
