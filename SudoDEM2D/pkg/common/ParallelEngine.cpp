#include <sudodem/pkg/common/ParallelEngine.hpp>

#ifdef SUDODEM_OPENMP
  #include<omp.h>
#endif

void ParallelEngine::action(){
	// openMP warns if the iteration variable is unsigned...
	const int size=(int)slaves.size();
	#ifdef SUDODEM_OPENMP
		//nested parallel regions are disabled by default on some platforms, we enable them since some of the subengine may be also parallel
		omp_set_nested(1);
		#pragma omp parallel for num_threads(ompThreads)

	#endif
	for(int i=0; i<size; i++){
		// run every slave group sequentially
		for(const shared_ptr<Engine>& e : slaves[i]) {
			//cerr<<"["<<omp_get_thread_num()<<":"<<e->getClassName()<<"]";
			e->scene=scene;
			if(!e->dead && e->isActivated()) e->action();
		}
	}
}

void ParallelEngine::slaves_set(const pybind11::list& slaves2){
	int len=pybind11::len(slaves2);
	slaves.clear();
	for(int i=0; i<len; i++){
		try { 
			std::vector<shared_ptr<Engine> > serialGroup = slaves2[i].cast<std::vector<shared_ptr<Engine> > >();
			slaves.push_back(serialGroup); 
			continue; 
		} catch (...) {}
		try { 
			shared_ptr<Engine> serialAlone = slaves2[i].cast<shared_ptr<Engine> >();
			vector<shared_ptr<Engine> > aloneWrap; 
			aloneWrap.push_back(serialAlone); 
			slaves.push_back(aloneWrap); 
			continue; 
		} catch (...) {}
		PyErr_SetString(PyExc_TypeError,"List elements must be either\n (a) sequences of engines to be executed one after another\n(b) alone engines.");
		throw pybind11::error_already_set();
	}
}

pybind11::list ParallelEngine::slaves_get(){
	pybind11::list ret;
	for (auto& grp : slaves){
		if(grp.size()==1) ret.append(pybind11::cast(grp[0]));
		else ret.append(pybind11::cast(grp));
	}
	return ret;
}


