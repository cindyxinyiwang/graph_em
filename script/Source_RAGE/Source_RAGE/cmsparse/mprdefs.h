/*
 * mqrdefs.h
 *
 *  Created on: Jan 30, 2009
 *      Author: tomert
 */

#ifndef MPRDEFS_H_
#define MPRDEFS_H_

//#define USE_CONTMEMSM

#ifndef DISABLE_MH_ASSERT
#ifdef _DEBUG
#include <crtdbg.h>	// _ASSERT
	#define MH_ASSERT(expr) \
		_ASSERT(expr)
#else // _DEBUG
#ifdef WIN32
#define __asmint3__	__asm { int 3 }
#else// WIN32
#define __asmint3__	__asm__ ("\tint3");
#endif// WIN32

	#define MH_ASSERT(expr) \
        do { if (!(expr))														\
			{																	\
				fflush(stdout);					\
				fprintf(stderr,"MH assert " __FILE__ ":%d; " #expr "\n",__LINE__);	\
				__asmint3__														\
			}																	\
		} while (0)
#endif // _DEBUG
#endif // DISABLE_MH_ASSERT


// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the MPR_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// MPR_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
/*#ifdef WIN32
#ifdef MPR_EXPORTS
#define MPR_API __declspec(dllexport)
#else
#define MPR_API __declspec(dllimport)
#endif
#else// WIN32
*/
#define MPR_API
//#endif// WIN32

#ifdef __cplusplus
#include <iostream>
template<typename _CharT, typename _Traits>
class ios_manip_guard {
public:
	inline ios_manip_guard(std::basic_ios<_CharT,_Traits> & rios) :
	m_rios(rios) {
    	m_width= m_rios.width();
    	m_fill= m_rios.fill();
    	m_precision= m_rios.precision();
    	m_fmtflags= m_rios.flags();
    }
	inline ~ios_manip_guard() {
    	m_rios.width(m_width);
    	m_rios.fill(m_fill);
    	m_rios.precision(m_precision);
    	m_rios.flags(m_fmtflags);
	}
protected:
	std::basic_ios<_CharT,_Traits> & m_rios;
private:
  	std::streamsize m_width;
   	_CharT m_fill;
   	long m_precision;
   	std::ios_base::fmtflags m_fmtflags;
};
template<typename _CharT, typename _Traits>
class basic_ostream_manip_guard : public ios_manip_guard<_CharT,_Traits> {
public:
	inline basic_ostream_manip_guard(std::basic_ostream<_CharT, _Traits>& ros) :
		ios_manip_guard<_CharT,_Traits>(ros),
		m_ros(ros) {
	}
	inline operator std::basic_ostream<_CharT, _Traits>&() {
		return m_ros;
	}
protected:
	std::basic_ostream<_CharT, _Traits>& m_ros;
};

template<typename _CharT, typename _Traits = std::char_traits<_CharT> >
   class basic_ostream_manip_guard;
#define GUARD_COUT static_cast<std::ostream&> (		\
	basic_ostream_manip_guard<char>(std::cout))
#endif // __cplusplus

#endif /* MPRDEFS_H_ */
