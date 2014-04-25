#ifndef __THREAD_H__
#define __THREAD_H__

#include <string>

typedef void (*CALLBACK_FUNC)(std::string, int, ...);

inline void NoCallback(std::string, int, ...) { }

typedef struct ThreadData
{
	CALLBACK_FUNC callbackFunc;
	void* pParams;

	ThreadData(
		CALLBACK_FUNC _callbackFunc = NoCallback,
		void* _pParams = NULL) :
		callbackFunc(_callbackFunc), pParams(_pParams) {}
}
ThreadData;

#endif
