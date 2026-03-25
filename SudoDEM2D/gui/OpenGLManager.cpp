#include"OpenGLManager.hpp"

CREATE_LOGGER(OpenGLManager);

OpenGLManager* OpenGLManager::self=NULL;

OpenGLManager::OpenGLManager(QObject* parent): QObject(parent){
	if(self) throw runtime_error("OpenGLManager instance already exists, uses OpenGLManager::self to retrieve it.");
	self=this;
	renderer=shared_ptr<OpenGLRenderer>(new OpenGLRenderer);
	// Note: renderer->init() is NOT called here because there's no OpenGL context yet.
	// The initialization will happen lazily in OpenGLRenderer::render() when first called,
	// at which point a valid GL context will be current.
	connect(this,SIGNAL(createView()),this,SLOT(createViewSlot()));
	connect(this,SIGNAL(resizeView(int,int,int)),this,SLOT(resizeViewSlot(int,int,int)));
	connect(this,SIGNAL(closeView(int)),this,SLOT(closeViewSlot(int)));
	connect(this,SIGNAL(startTimerSignal()),this,SLOT(startTimerSlot()),Qt::QueuedConnection);
}

void OpenGLManager::timerEvent(QTimerEvent* event){
	//cerr<<".";
	std::lock_guard<std::mutex> lock(viewsMutex);
	// when sharing the 0th view widget, it should be enough to update the primary view only
	//if(views.size()>0) views[0]->updateGL();
	#if 1
		for(const auto& view : views){ 
			if(view && view->isUpdatesEnabled()) view->update(); 
		}
	#endif
}

void OpenGLManager::createViewSlot(){
	std::lock_guard<std::mutex> lock(viewsMutex);
	if(views.size()==0){
		views.push_back(shared_ptr<GLViewer>(new GLViewer(0,renderer,/*shareWidget*/(QGLWidget*)0)));
	} else {
		throw runtime_error("Secondary views not supported");
		//views.push_back(shared_ptr<GLViewer>(new GLViewer(views.size(),renderer,views[0].get())));
	}	
}

void OpenGLManager::resizeViewSlot(int id, int wd, int ht){
	views[id]->resize(wd,ht);
}

void OpenGLManager::closeViewSlot(int id){
	std::lock_guard<std::mutex> lock(viewsMutex);
	for(size_t i=views.size()-1; (!views[i]); i--){ views.resize(i); } // delete empty views from the end
	if(id<0){ // close the last one existing
		assert(*views.rbegin()); // this should have been sanitized by the loop above
		views.resize(views.size()-1); // releases the pointer as well
	}
	if(id==0){
		LOG_DEBUG("Closing primary view.");
		if(views.size()==1) views.clear();
		else{ LOG_INFO("Cannot close primary view, secondary views still exist."); }
	}
}
void OpenGLManager::centerAllViews(){
	std::lock_guard<std::mutex> lock(viewsMutex);
	for(const auto& g : views){ if(!g) continue; g->centerScene(); }
}
void OpenGLManager::startTimerSlot(){
	startTimer(50);
}

int OpenGLManager::waitForNewView(float timeout,bool center){
	// std::cerr << "DEBUG OpenGLManager::waitForNewView() starting" << std::endl;
	size_t origViewCount=views.size();
	// std::cerr << "DEBUG OpenGLManager::waitForNewView() emitting createView" << std::endl;
	emitCreateView();
	float t=0;
	// std::cerr << "DEBUG OpenGLManager::waitForNewView() waiting for view creation" << std::endl;
	while(views.size()!=origViewCount+1)
	{
		#ifdef _WIN32
			std::this_thread::sleep_for(std::chrono::microseconds(50000));
		#else
			usleep(50000); 
		#endif
		
		t+=.05;
		// wait at most 5 secs
		if(t>=timeout) {
			LOG_ERROR("Timeout waiting for the new view to open, giving up."); return -1;
		}
	}
	// std::cerr << "DEBUG OpenGLManager::waitForNewView() view created, center=" << center << std::endl;
	if(center) {
		// std::cerr << "DEBUG OpenGLManager::waitForNewView() calling centerScene" << std::endl;
		(*views.rbegin())->centerScene();
		// std::cerr << "DEBUG OpenGLManager::waitForNewView() centerScene completed" << std::endl;
	}
	// std::cerr << "DEBUG OpenGLManager::waitForNewView() returning viewId" << std::endl;
	return (*views.rbegin())->viewId; 
}
