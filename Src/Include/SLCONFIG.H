// SLCONFIG.H
// Copyright (c) Sobolev A. 1995, 1996, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2016, 2017, 2018, 2019
// @codepage UTF-8
//
#ifndef __SLCONFIG_H
#define __SLCONFIG_H
//
// Справочная информация по значениям _MSC_VER
// MSVC++ 5.0  _MSC_VER == 1100
// MSVC++ 6.0  _MSC_VER == 1200
// MSVC++ 7.0  _MSC_VER == 1300
// MSVC++ 7.1  _MSC_VER == 1310 (Visual Studio 2003)
// MSVC++ 8.0  _MSC_VER == 1400 (Visual Studio 2005)
// MSVC++ 9.0  _MSC_VER == 1500 (Visual Studio 2008)
// MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
// MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
// MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
// MSVC++ 14.0 _MSC_VER == 1900 (Visual Studio 2015)
// Visual Studio 2017 RTW (15.0) 	1910
// Visual Studio 2017 версия 15.3 	1911
// Visual Studio 2017 версии 15.5 	1912
// Visual Studio 2017 версии 15.6 	1913
// Visual Studio 2017 версии 15.7 	1914
// Visual Studio 2017 версии 15.8 	1915
// Visual Studio 2017 версии 15.9 	1916
//
#if (_MSC_VER < 1900)
	#ifndef _WIN32_WINNT
		#define _WIN32_WINNT 0x0501
	#endif
	#ifndef NTDDI_VERSION
		#define NTDDI_VERSION NTDDI_WINXP
	#endif
#else
	#ifndef _WIN32_WINNT
		#ifdef NGINX_SLIB
			#define _WIN32_WINNT 0x0600 // @v9.7.12
		#else
			#define _WIN32_WINNT 0x0501 // @v9.1.12 0x0600-->0x0501
		#endif
	#endif
	#ifndef NTDDI_VERSION
		#define NTDDI_VERSION NTDDI_WIN7
	#endif
	#define _CRT_SECURE_NO_WARNINGS
#endif

/* Remove comment if clarion date format is needed
#define USE_DF_CLARION */
#define DEFAULT_BIN_DATE_FORMAT   DF_BTRIEVE
#define DEFAULT_TXT_DATE_FORMAT   DATF_GERMAN

#define DEFAULT_OVERFLOW_SYMB     '#'
#define DEFAULT_PASSWORD_SYMB     '*'

// @v9.8.12 #define MAXSSIZE                 255 // Maximum size of data types, processing by STYPE mechanism
// @v9.8.12 #define COMPLEX_TYPES // Remove comment if complex types are needed (see STYPE.H)
// #define OBJSTORE_STREAM // Remove comment if ObjStore streaming is needed
// @v9.8.12 #define DECL_INTX // Comment if 'typedef [un]signed [char|short|long] [u]int[8|16|32]' not needed
//
// Если !0, то некоторые низкоуровневые функции заменяются на соответствующие функции из ASMLIB
// Замена осуществляется в slib.h посредством #define
//
// После включения библиотеки в релиз Papyrus v7.0.10 обнаружились непонятные эффекты.
// В релизе 7.0.12 библиотека отключена. Если эффекты исчезнут, то придется писать
// нагрузочные тесты для проверки всех аспектов использования данной библиотеки.
//
#define USE_ASMLIB       1
#define USE_SSE2         1 // @v9.8.4 used by pixman. @attention: Эта опция увеличивает размер exe-файла на ~2Mb
//
//
//
#define SALLOC_STD       0
#define SALLOC_NEDMALLOC 1

#define SALLOCATOR       SALLOC_STD
//
// @v9.8.1 Если !0, то ведется отладочная трассировка внутренних блокировок.
// Актуально только для сервера.
//
#ifdef _PPSERVER
	#define SLTRACELOCKSTACK 1 // @v9.8.3 С помощью трассировки аномалии идентифицировать пока не удалось. До поры-времени отключаем.
		// @v9.8.8 Опять включили.
#else
	#define SLTRACELOCKSTACK 0
#endif
//
// Descr: Если следующее определение != 0, то весь проект компилируется с включенными
//   кодами автоматического тестирования.
//
#define SLTEST_RUNNING       0

#endif /* __SLCONFIG_H */
