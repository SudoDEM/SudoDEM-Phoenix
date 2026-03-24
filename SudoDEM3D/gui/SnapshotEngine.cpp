#ifdef SUDODEM_OPENGL

#include"SnapshotEngine.hpp"

//CH_BUG_FIX
#include <thread>
#include <chrono>

CREATE_LOGGER(SnapshotEngine);

void SnapshotEngine::action(){
	if(!OpenGLManager::self) throw logic_error("No OpenGLManager instance?!");
	if(OpenGLManager::self->views.size()==0){
		int viewNo=OpenGLManager::self->waitForNewView(deadTimeout);
		if(viewNo<0){
			if(!ignoreErrors) throw runtime_error("SnapshotEngine: Timeout waiting for new 3d view.");
			else {
				LOG_WARN("Making myself Engine::dead, as I can not live without a 3d view (timeout)."); dead=true; return;
			}
		}
	}
	const shared_ptr<GLViewer>& glv=OpenGLManager::self->views[0];
	// Convert format to lowercase manually
	string formatLower = format;
	std::transform(formatLower.begin(), formatLower.end(), formatLower.begin(), ::tolower);
	ostringstream fss; fss<<fileBase<<setw(5)<<setfill('0')<<counter++<<"."<<formatLower;
	LOG_DEBUG("GL view → "<<fss.str())
	glv->setSnapshotFormat(QString(format.c_str()));
	glv->setNextFrameSnapshotFilename(fss.str());
	// wait for the renderer to save the frame (will happen at next postDraw)
	timespec t1,t2; t1.tv_sec=0; t1.tv_nsec=10000000; /* 10 ms */
	long waiting=0;
	while(!glv->getNextFrameSnapshotFilename().empty()){
		//nanosleep(&t1,&t2); 
		std::this_thread::sleep_for(
    		std::chrono::seconds(t1.tv_sec) + std::chrono::nanoseconds(t1.tv_nsec)
		);
		waiting++;
		if(((waiting) % 1000)==0) LOG_WARN("Already waiting "<<waiting/100<<"s for snapshot to be saved. Something went wrong?");
		if(waiting/100.>deadTimeout){
			if(ignoreErrors){ LOG_WARN("Timeout waiting for snapshot to be saved, making byself Engine::dead"); dead=true; return; }
			else throw runtime_error("SnapshotEngine: Timeout waiting for snapshot to be saved.");
		}
	}
	snapshots.push_back(fss.str());
	//usleep((long)(msecSleep*1000));

	std::this_thread::sleep_for(
    	std::chrono::microseconds(static_cast<long long>(msecSleep * 1000))
	);
	//if(!plot.empty()){ pyRunString("import sudodem.plot; sudodem.plot.addImgData("+plot+"='"+fss.str()+"')"); }
}


#endif /* SUDODEM_OPENGL */
