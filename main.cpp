#include "stdio.h"
#include "main.h"
#include "x2camera.h"
#include "../../licensedinterfaces/basicstringinterface.h"

//Ths file normally does not require changing

#define PLUGIN_NAME "Orion SSAG Autoguider"

extern "C" PlugInExport int sbPlugInName2(BasicStringInterface& str)
{
	str = PLUGIN_NAME;

	return 0;
}

extern "C" PlugInExport int sbPlugInFactory2(	const char* pszSelection, 
												const int& nInstanceIndex,
												SerXInterface					* pSerXIn, 
												TheSkyXFacadeForDriversInterface* pTheSkyXIn, 
												SleeperInterface		* pSleeperIn,
												BasicIniUtilInterface   * pIniUtilIn,
												LoggerInterface			* pLoggerIn,
												MutexInterface			* pIOMutexIn,
												TickCountInterface		* pTickCountIn,
												void** ppObjectOut)
{
	*ppObjectOut = NULL;
	OrionSSAG* gpMyImpl=NULL;

	if (NULL == gpMyImpl)
		gpMyImpl = new OrionSSAG(	pszSelection, 
									nInstanceIndex,
									pSerXIn, 
									pTheSkyXIn, 
									pSleeperIn,
									pIniUtilIn,
									pLoggerIn,
									pIOMutexIn,
									pTickCountIn);

		*ppObjectOut = gpMyImpl;

	return 0;
}

