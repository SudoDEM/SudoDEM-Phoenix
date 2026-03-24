// 2009 © Vaclav Smilauer <eudoxos@arcig.cz>
#pragma once

#include <mutex>

#ifdef _WIN32
  #ifdef SUDODEM_SINGLETON_EXPORT
  	#define SUDODEM_SINGLETON_API __declspec(dllexport)
  #else
    #define SUDODEM_SINGLETON_API __declspec(dllimport)
  #endif

  #define FRIEND_SINGLETON(Class) friend class Singleton<Class>;

	#define SINGLETON_SELF(Class) \
    	template<> SUDODEM_SINGLETON_API Class* Singleton<Class>::self = nullptr;

	extern SUDODEM_SINGLETON_API std::mutex singleton_constructor_mutex;
	
	template <class T>
	class Singleton{
		protected:
			// Both static members must be exported/imported to be shared across DLL boundaries
			static SUDODEM_SINGLETON_API T* self;
		public:
			static T& instance(){
				if(!self) {
					std::lock_guard<std::mutex> lock(singleton_constructor_mutex);
					if(!self) self=new T;
				}
				return *self;
			}
	};

#else
  #define FRIEND_SINGLETON(Class) friend class Singleton<Class>;
	// use to instantiate the self static member.
	#define SINGLETON_SELF(Class) template<> Class* Singleton<Class>::self=NULL;
	namespace { std::mutex singleton_constructor_mutex; }
	template <class T>
	class Singleton{
		protected:
			static T* self; // must not be method-local static variable, since it gets created in different translation units multiple times.
		public:
			static T& instance(){
				if(!self) {
					std::lock_guard<std::mutex> lock(singleton_constructor_mutex);
					if(!self) self=new T;
				}
				return *self;
			}
	};
#endif




