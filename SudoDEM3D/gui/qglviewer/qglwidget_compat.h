#ifndef QGLWIDGET_COMPAT_H
#define QGLWIDGET_COMPAT_H

#include <QOpenGLWidget>
#include <QtGlobal>
#include <QTime>
#include <QPainter>
#include <QFontMetrics>
#include <QWheelEvent>
#include <algorithm>

// Qt6 compatibility macros
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	// Dummy classes for Qt6 compatibility
	class QGLContext {
	public:
		QGLContext() {}
	};
	
	class QGLFormat {
	public:
		QGLFormat() {}
	};
	
	// QGLWidget -> QOpenGLWidget with wrapper constructors
	class QGLWidget : public QOpenGLWidget {
	public:
		// Default constructor
		explicit QGLWidget(QWidget* parent = nullptr, const QOpenGLWidget* shareWidget = nullptr, Qt::WindowFlags f = Qt::WindowFlags())
			: QOpenGLWidget(parent, f) {
			(void)shareWidget; // TODO: implement context sharing
			// Qt6/QOpenGLWidget: Ensure proper initialization
			setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
		}

		// Constructor with QGLContext (ignored in Qt6)
		explicit QGLWidget(QGLContext*, QWidget* parent = nullptr, const QOpenGLWidget* shareWidget = nullptr, Qt::WindowFlags f = Qt::WindowFlags())
			: QOpenGLWidget(parent, f) {
			(void)shareWidget;
			setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
		}

		// Constructor with QGLFormat (ignored in Qt6)
		explicit QGLWidget(const QGLFormat&, QWidget* parent = nullptr, const QOpenGLWidget* shareWidget = nullptr, Qt::WindowFlags f = Qt::WindowFlags())
			: QOpenGLWidget(parent, f) {
			(void)shareWidget;
			setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
		}

		// Dummy methods for Qt6 compatibility
		void makeCurrent() {
			QOpenGLWidget::makeCurrent();
		}

		void doneCurrent() {
			QOpenGLWidget::doneCurrent();
		}
	};
	
#else
	// Qt5 - use original QGLWidget
	#include <QGLWidget>
	#include <QGLContext>
	#include <QGLFormat>
#endif

// Compatibility macros for deprecated Qt APIs
#define QString_null QString()
#define Qt_MidButton Qt::MiddleButton

// QTime compatibility macros
#define QTIME_START(obj) do { \
	(obj) = QTime::currentTime(); \
} while(0)

#define QTIME_RESTART(obj) ( \
	(obj) = QTime::currentTime(), \
	0 \
)

// QTime::elapsed compatibility
static inline int QTIME_elapsed(const QTime& t) {
	return QTime::currentTime().msecsTo(t);
}

// renderText compatibility for Qt6 (uses QPainter)
static inline void renderText(QGLWidget* w, int x, int y, const QString& str, const QFont& fnt = QFont()) {
	QPainter painter(w);
	painter.setFont(fnt);
	painter.drawText(x, y, str);
}

// OpenGL color functions compatibility
static inline void qglClearColor(const QColor& c) {
	glClearColor(c.redF(), c.greenF(), c.blueF(), c.alphaF());
}

static inline void qglColor(const QColor& c) {
	glColor4f(c.redF(), c.greenF(), c.blueF(), c.alphaF());
}

// qSort compatibility (use std::sort)
#define qSort std::sort

// QWheelEvent::angleDelta compatibility
static inline QPoint QWHEEL_delta(const QWheelEvent* e) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	return e->angleDelta();
#else
	return QPoint(e->delta(), 0);
#endif
}

// QString::SkipEmptyParts compatibility
#ifndef QT_SKIP_EMPTY_PARTS
#define QT_SKIP_EMPTY_PARTS Qt::SkipEmptyParts
#endif

// grabFrameBuffer compatibility
static inline QImage QGLWIDGET_grabFrameBuffer(QGLWidget* w) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	return w->grabFramebuffer();
#else
	return w->grabFrameBuffer();
#endif
}

// QString::sprintf compatibility
#define QString_sprintf(buffer, ...) QString::asprintf(buffer, __VA_ARGS__)

#endif // QGLWIDGET_COMPAT_H