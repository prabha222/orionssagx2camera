#pragma once

#include "../../licensedinterfaces/cameradriverinterface.h"
#include "../../licensedinterfaces/modalsettingsdialoginterface.h"
#include "../../licensedinterfaces/x2guiinterface.h"


class SerXInterface;		
class TheSkyXFacadeForDriversInterface;
class SleeperInterface;
class BasicIniUtilInterface;
class LoggerInterface;
class MutexInterface;
class TickCountInterface;


/*!
\brief The OrionSSAG example.

\ingroup Example

Use this example to write an OrionSSAG driver.
*/
class OrionSSAG: public CameraDriverInterface, public ModalSettingsDialogInterface, public X2GUIEventInterface
{
public: 
	/*!Standard X2 constructor*/
	OrionSSAG(const char* pszSelectionString, 
					const int& nISIndex,
					SerXInterface*						pSerX,
					TheSkyXFacadeForDriversInterface* pTheSkyXForMounts,
					SleeperInterface*					pSleeper,
					BasicIniUtilInterface*				pIniUtil,
					LoggerInterface*					pLogger,
					MutexInterface*					pIOMutex,
					TickCountInterface*				pTickCount);
	virtual ~OrionSSAG();  

	/*!\name DriverRootInterface Implementation
	See DriverRootInterface.*/
	//@{ 
	virtual int									queryAbstraction(const char* pszName, void** ppVal)			;
	//@} 

	/*!\name DriverInfoInterface Implementation
	See DriverInfoInterface.*/
	//@{ 
	virtual void								driverInfoDetailedInfo(BasicStringInterface& str) const		;
	virtual double								driverInfoVersion(void) const								;
	//@} 

	/*!\name HardwareInfoInterface Implementation
	See HardwareInfoInterface.*/
	//@{ 
	virtual void deviceInfoNameShort(BasicStringInterface& str) const										;
	virtual void deviceInfoNameLong(BasicStringInterface& str) const										;
	virtual void deviceInfoDetailedDescription(BasicStringInterface& str) const								;
	virtual void deviceInfoFirmwareVersion(BasicStringInterface& str)										;
	virtual void deviceInfoModel(BasicStringInterface& str)													;
	//@} 

public://Properties

	/*!\name CameraDriverInterface Implementation
	See CameraDriverInterface.*/
	//@{ 

	virtual enumCameraIndex	cameraId();
	virtual	void		setCameraId(enumCameraIndex Cam);
	virtual bool		isLinked()					{return m_bLinked;}
	virtual void		setLinked(const bool bYes)	{m_bLinked = bYes;}
	
	virtual int			GetVersion(void)			{return CAMAPIVERSION;}
	virtual CameraDriverInterface::ReadOutMode readoutMode(void);
	virtual int			pathTo_rm_FitsOnDisk(char* lpszPath, const int& nPathSize);

public://Methods

	virtual int CCSettings(const enumCameraIndex& Camera, const enumWhichCCD& CCD);

	virtual int CCEstablishLink(enumLPTPort portLPT, const enumWhichCCD& CCD, enumCameraIndex DesiredCamera, enumCameraIndex& CameraFound, const int nDesiredCFW, int& nFoundCFW);
	virtual int CCDisconnect(const bool bShutDownTemp);

	virtual int CCGetChipSize(const enumCameraIndex& Camera, const enumWhichCCD& CCD, const int& nXBin, const int& nYBin, const bool& bOffChipBinning, int& nW, int& nH, int& nReadOut);
	virtual int CCGetNumBins(const enumCameraIndex& Camera, const enumWhichCCD& CCD, int& nNumBins);
	virtual	int CCGetBinSizeFromIndex(const enumCameraIndex& Camera, const enumWhichCCD& CCD, const int& nIndex, long& nBincx, long& nBincy);

	virtual int CCSetBinnedSubFrame(const enumCameraIndex& Camera, const enumWhichCCD& CCD, const int& nLeft, const int& nTop, const int& nRight, const int& nBottom);

	virtual void CCMakeExposureState(int* pnState, enumCameraIndex Cam, int nXBin, int nYBin, int abg, bool bRapidReadout);//SBIG specific

	virtual int CCStartExposure(const enumCameraIndex& Cam, const enumWhichCCD CCD, const double& dTime, enumPictureType Type, const int& nABGState, const bool& bLeaveShutterAlone);
	virtual int CCIsExposureComplete(const enumCameraIndex& Cam, const enumWhichCCD CCD, bool* pbComplete, unsigned int* pStatus);
	virtual int CCEndExposure(const enumCameraIndex& Cam, const enumWhichCCD CCD, const bool& bWasAborted, const bool& bLeaveShutterAlone);

	virtual int CCReadoutLine(const enumCameraIndex& Cam, const enumWhichCCD& CCD, const int& pixelStart, const int& pixelLength, const int& nReadoutMode, unsigned char* pMem);
	virtual int CCDumpLines(const enumCameraIndex& Cam, const enumWhichCCD& CCD, const int& nReadoutMode, const unsigned int& lines);

	virtual int CCReadoutImage(const enumCameraIndex& Cam, const enumWhichCCD& CCD, const int& nWidth, const int& nHeight, const int& nMemWidth, unsigned char* pMem);

	virtual int CCRegulateTemp(const bool& bOn, const double& dTemp);
	virtual int CCQueryTemperature(double& dCurTemp, double& dCurPower, char* lpszPower, const int nMaxLen, bool& bCurEnabled, double& dCurSetPoint);
	virtual int	CCGetRecommendedSetpoint(double& dRecSP);
	virtual int	CCSetFan(const bool& bOn);

	virtual int CCActivateRelays(const int& nXPlus, const int& nXMinus, const int& nYPlus, const int& nYMinus, const bool& bSynchronous, const bool& bAbort, const bool& bEndThread);

	virtual int CCPulseOut(unsigned int nPulse, bool bAdjust, const enumCameraIndex& Cam);

	virtual int CCSetShutter(bool bOpen);
	virtual int CCUpdateClock(void);

	virtual int CCSetImageProps(const enumCameraIndex& Camera, const enumWhichCCD& CCD, const int& nReadOut, void* pImage);	
	virtual int CCGetFullDynamicRange(const enumCameraIndex& Camera, const enumWhichCCD& CCD, unsigned long& dwDynRg);
	
	virtual void CCBeforeDownload(const enumCameraIndex& Cam, const enumWhichCCD& CCD);
	virtual void CCAfterDownload(const enumCameraIndex& Cam, const enumWhichCCD& CCD);
	//@} 

	//
	/*!\name ModalSettingsDialogInterface Implementation
	See ModalSettingsDialogInterface.*/
	//@{ 
	virtual int								initModalSettingsDialog(void){return 0;}
	virtual int								execModalSettingsDialog(void);
	//@} 
	
	//
	/*!\name X2GUIEventInterface Implementation
	See X2GUIEventInterface.*/
	//@{ 
	virtual void uiEvent(X2GUIExchangeInterface* uiex, const char* pszEvent);
	//@} 

	//Implemenation below here
private:
	SerXInterface 									*	GetSerX() {return m_pSerX; }		
	TheSkyXFacadeForDriversInterface				*	GetTheSkyXFacadeForDrivers() {return m_pTheSkyXForMounts;}
	SleeperInterface								*	GetSleeper() {return m_pSleeper; }
	BasicIniUtilInterface							*	GetBasicIniUtil() {return m_pIniUtil; }
	LoggerInterface									*	GetLogger() {return m_pLogger; }
	MutexInterface									*	GetMutex() const  {return m_pIOMutex;}
	TickCountInterface								*	GetTickCountInterface() {return m_pTickCount;}

	SerXInterface									*	m_pSerX;		
	TheSkyXFacadeForDriversInterface				*	m_pTheSkyXForMounts;
	SleeperInterface								*	m_pSleeper;
	BasicIniUtilInterface							*	m_pIniUtil;
	LoggerInterface									*	m_pLogger;
	MutexInterface									*	m_pIOMutex;
	TickCountInterface								*	m_pTickCount;

	int m_nPrivateISIndex;

	int width();
	int  height();
	void setWidth(const int& nValue);
	void setHeight(const int& nValue);

	int m_dwFin;
	int m_CachedBinX;
	int m_CachedBinY;
	int m_CachedCam;
	double m_dCurTemp;
	double m_dCurPower;

	int doAddPixelDialogExample(int& xCoord, int& yCoord, bool& bPressedOK);
};



