#ifndef TESTEXCEPTION_H
#define TESTEXCEPTION_H

#include<stdexcept>
#include "exception_common.h"
#include "common.h"

class Test_exception : public std::exception
{
	String msg_;
	String src_;
	String what_;
	public:
		Test_exception(const std::string& src, const std::string& msg);
    virtual ~Test_exception() throw();
		virtual String msg();
		const char* what();
};

#define ALIGN80 "\t\t\t\t\t\t\t\t"

#define TEST_ASSERT( dcond ) { \
	if( !(dcond) ){ std::cout << "Testing: "+std::string(#dcond)+ALIGN80+"[FAILS]" << std::endl;  } \
	else{ std::cout << "Testing: "+std::string(#dcond)+ALIGN80+"[OK]" << std::endl; } \
	}

#define TEST_EXCEPTION_THROW( function ) { \
	bool exp=false;   \
	try{ \
		({function;}); \
	}catch(Test_exception& e){ exp = true; } \
	catch(Exception& e){ exp = true; }; \
	if( !exp ){ std::cout << "Testing: "+std::string(#function)+ALIGN80+"[FAILS]" << std::endl;  } \
	else{ std::cout << "Testing: "+std::string(#function)+ALIGN80+"[OK]" << std::endl; } \
	}

#define TEST_EXCEPTION_NTHROW( function ) { \
	bool exp=false;   \
	try{ \
		(function); \
	}catch(Test_exception& e){ exp = true; } \
	catch(Exception& e){ exp = true; \
		std::cout << "Testing: "+std::string(#function)+ALIGN80+"[FAILS]" << std::endl; \
	std::cout << e << std::endl; }; \
	if( exp ){   } \
		else{ std::cout << "Testing: "+std::string(#function)+ALIGN80+"[OK]" << std::endl; } \
	}

/*
#ifndef MASSERT
#define MASSERT( t ) 		\
{   				\
	if( !(t) ) throw Test_exception(__PRETTY_FUNCTION__, #t); 	\
}
#endif // MASSERT
*/

#endif // TESTEXCEPTION_H
