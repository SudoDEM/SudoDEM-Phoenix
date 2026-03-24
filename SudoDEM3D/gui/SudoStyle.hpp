#ifndef SUDO_STYLE_HPP
#define SUDO_STYLE_HPP

#include <QProxyStyle>
#include <QStyleOption>
#include <QPainter>

/**
 * Custom proxy style for SudoDEM to avoid complex CSS stylesheet issues.
 * Implements custom styling through C++ code instead of CSS parsing.
 */
class SudoStyle : public QProxyStyle {
    Q_OBJECT

public:
    SudoStyle(QStyle* baseStyle = nullptr);

    void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                      QPainter* painter, const QWidget* widget = nullptr) const override;

    void drawControl(ControlElement element, const QStyleOption* option,
                    QPainter* painter, const QWidget* widget = nullptr) const override;

    int pixelMetric(PixelMetric metric, const QStyleOption* option = nullptr,
                   const QWidget* widget = nullptr) const override;

    QSize sizeFromContents(ContentsType type, const QStyleOption* option,
                          const QSize& size, const QWidget* widget = nullptr) const override;

    QRect subElementRect(SubElement element, const QStyleOption* option,
                        const QWidget* widget = nullptr) const override;
};

#endif // SUDO_STYLE_HPP
