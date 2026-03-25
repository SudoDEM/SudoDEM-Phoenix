#pragma once
//#include

#ifndef Q_MOC_RUN
#include"GLViewer.hpp"
#endif

#include<QObject>
#include<mutex>

// Restore Qt slots macro after Python headers
#ifndef slots
#define slots Q_SLOTS
#endif
#ifndef signals
#define signals Q_SIGNALS
#endif

#ifdef _WIN32
  #ifdef GLVIEWER_OPENGL_BUILD
    #define GLVIEWER_OPENGL_API __declspec(dllexport)
  #else
    #define GLVIEWER_OPENGL_API __declspec(dllimport)
  #endif
#else
  #define GLVIEWER_OPENGL_API
#endif

/*
Singleton class managing OpenGL views,
a renderer instance and timer to refresh the display.
*/
class OpenGLManager: public QObject{
	Q_OBJECT
	DECLARE_LOGGER;
	public:
		static GLVIEWER_OPENGL_API OpenGLManager* self;
		OpenGLManager(QObject *parent=0);
		// manipulation must lock viewsMutex!
		std::vector<shared_ptr<GLViewer> > views;
		shared_ptr<OpenGLRenderer> renderer;
		// signals are protected, emitting them is therefore wrapped with such funcs
		void emitResizeView(int id, int wd, int ht){ emit resizeView(id,wd,ht); }
		void emitCreateView(){ emit createView(); }
		void emitStartTimer(){ emit startTimerSignal(); }
		void emitCloseView(int id){ emit closeView(id); }
		// create a new view and wait for it to become available; return the view number
		// if timout (in seconds) elapses without the view to come up, reports error and returns -1
		int waitForNewView(float timeout=5., bool center=true);
	signals:
		void createView();
		void resizeView(int id, int wd, int ht);
		void closeView(int id);
		// this is used to start timer from the main thread via postEvent (ugly)
		void startTimerSignal();
	public slots:
		virtual void createViewSlot();
		virtual void resizeViewSlot(int id, int wd, int ht);
		virtual void closeViewSlot(int id=-1);
		virtual void timerEvent(QTimerEvent* event);
		virtual void startTimerSlot();
		void centerAllViews();
	private:
		std::mutex viewsMutex;
};
