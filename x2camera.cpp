// x2camera.cpp  
//
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "x2camera.h"
#include "../../licensedinterfaces/sberrorx.h"
#include "../../licensedinterfaces/serxinterface.h"
#include "../../licensedinterfaces/theskyxfacadefordriversinterface.h"
#include "../../licensedinterfaces/sleeperinterface.h"
#include "../../licensedinterfaces/loggerinterface.h"
#include "../../licensedinterfaces/basiciniutilinterface.h"
#include "../../licensedinterfaces/mutexinterface.h"
#include "../../licensedinterfaces/tickcountinterface.h"
#include "../../licensedinterfaces/basicstringinterface.h"
#include "../../licensedinterfaces/x2guiinterface.h"

#define DEVICE_AND_DRIVER_INFO_STRING "Orion SSAG Autoguider"

//For properties that need to be persistent
#define KEY_X2CAM_ROOT			"OrionSSAG"
#define KEY_WIDTH				"Width"
#define KEY_HEIGHT				"Height"


OrionSSAG::OrionSSAG( const char* pszSelection, 
					const int& nISIndex,
					SerXInterface*						pSerX,
					TheSkyXFacadeForDriversInterface*	pTheSkyXForMounts,
					SleeperInterface*					pSleeper,
					BasicIniUtilInterface*				pIniUtil,
					LoggerInterface*					pLogger,
					MutexInterface*						pIOMutex,
					TickCountInterface*					pTickCount)
{   
	m_nPrivateISIndex				= nISIndex;
	m_pSerX							= pSerX;
	m_pTheSkyXForMounts				= pTheSkyXForMounts;
	m_pSleeper						= pSleeper;
	m_pIniUtil						= pIniUtil;
	m_pLogger						= pLogger;	
	m_pIOMutex						= pIOMutex;
	m_pTickCount					= pTickCount;

	setCameraId(CI_PLUGIN);

	m_dwFin = 0;
	m_dCurTemp = -5;
	m_dCurPower = 82.5;
	m_CachedBinX = 1;
	m_CachedBinY = 1; 
	m_CachedCam = 0;

}

OrionSSAG::~OrionSSAG()
{
	//Delete objects used through composition
	if (GetSerX())
		delete GetSerX();
	if (GetTheSkyXFacadeForDrivers())
		delete GetTheSkyXFacadeForDrivers();
	if (GetSleeper())
		delete GetSleeper();
	if (GetBasicIniUtil())
		delete GetBasicIniUtil();
	if (GetLogger())
		delete GetLogger();
	if (GetMutex())
		delete GetMutex();
	if (GetTickCountInterface())
		delete GetTickCountInterface();

}

//DriverRootInterface
int	OrionSSAG::queryAbstraction(const char* pszName, void** ppVal)			
{
	X2MutexLocker ml(GetMutex());

	if (!strcmp(pszName, ModalSettingsDialogInterface_Name))
		*ppVal = dynamic_cast<ModalSettingsDialogInterface*>(this);
	else
		if (!strcmp(pszName, X2GUIEventInterface_Name))
			*ppVal = dynamic_cast<X2GUIEventInterface*>(this);

	return SB_OK;
}

//DriverInfoInterface
void OrionSSAG::driverInfoDetailedInfo(BasicStringInterface& str) const		
{
	X2MutexLocker ml(GetMutex());

	str = DEVICE_AND_DRIVER_INFO_STRING;
}
double OrionSSAG::driverInfoVersion(void) const								
{
	X2MutexLocker ml(GetMutex());

	return 1.0;
}

//HardwareInfoInterface
void OrionSSAG::deviceInfoNameShort(BasicStringInterface& str) const										
{
	X2MutexLocker ml(GetMutex());

	str = DEVICE_AND_DRIVER_INFO_STRING;
}
void OrionSSAG::deviceInfoNameLong(BasicStringInterface& str) const										
{
	X2MutexLocker ml(GetMutex());

	str = DEVICE_AND_DRIVER_INFO_STRING;
}
void OrionSSAG::deviceInfoDetailedDescription(BasicStringInterface& str) const								
{
	X2MutexLocker ml(GetMutex());

	str = DEVICE_AND_DRIVER_INFO_STRING;
}
void OrionSSAG::deviceInfoFirmwareVersion(BasicStringInterface& str)										
{
	X2MutexLocker ml(GetMutex());

	str = DEVICE_AND_DRIVER_INFO_STRING;
}
void OrionSSAG::deviceInfoModel(BasicStringInterface& str)													
{
	X2MutexLocker ml(GetMutex());

	str = DEVICE_AND_DRIVER_INFO_STRING;
}

int OrionSSAG::CCEstablishLink(const enumLPTPort portLPT, const enumWhichCCD& CCD, enumCameraIndex DesiredCamera, enumCameraIndex& CameraFound, const int nDesiredCFW, int& nFoundCFW)
{   
	m_bLinked = true;
	return SB_OK;
}


int OrionSSAG::CCQueryTemperature(double& dCurTemp, double& dCurPower, char* lpszPower, const int nMaxLen, bool& bCurEnabled, double& dCurSetPoint)
{   
	X2MutexLocker ml(GetMutex());

	if (!m_bLinked)
		return ERR_NOLINK;

	dCurTemp = m_dCurTemp;
	dCurPower = m_dCurPower;

	return SB_OK;
}

int OrionSSAG::CCRegulateTemp(const bool& bOn, const double& dTemp)
{ 
	X2MutexLocker ml(GetMutex());

	if (!m_bLinked)
		return ERR_NOLINK;

	return SB_OK;
}

int OrionSSAG::CCGetRecommendedSetpoint(double& RecTemp)
{
	X2MutexLocker ml(GetMutex());

	RecTemp = 100;//Set to 100 if you cannot recommend a setpoint

	return SB_OK;
}  


int OrionSSAG::CCStartExposure(const enumCameraIndex& Cam, const enumWhichCCD CCD, const double& dTime, enumPictureType Type, const int& nABGState, const bool& bLeaveShutterAlone)
{   
	X2MutexLocker ml(GetMutex());

	if (!m_bLinked)
		return ERR_NOLINK;

	bool bLight = true;

	switch (Type)
	{
		case PT_FLAT:
		case PT_LIGHT:			bLight = true;	break;
		case PT_DARK:	
		case PT_AUTODARK:	
		case PT_BIAS:			bLight = false;	break;
		default:				return ERR_CMDFAILED;
	}

	if (m_pTickCount)
		m_dwFin = (unsigned long)(dTime*1000)+m_pTickCount->elapsed();
	else
		m_dwFin = 0;

	return SB_OK;
}   



int OrionSSAG::CCIsExposureComplete(const enumCameraIndex& Cam, const enumWhichCCD CCD, bool* pbComplete, unsigned int* pStatus)
{   
	X2MutexLocker ml(GetMutex());

	if (!m_bLinked)
		return ERR_NOLINK;

	if (m_pTickCount)
	{
		if (m_pTickCount->elapsed()>m_dwFin)
			*pbComplete = true;
	}
	else
		*pbComplete = true;

	return SB_OK;
}

int OrionSSAG::CCEndExposure(const enumCameraIndex& Cam, const enumWhichCCD CCD, const bool& bWasAborted, const bool& bLeaveShutterAlone)           
{   
	X2MutexLocker ml(GetMutex());

	if (!m_bLinked)
		return ERR_NOLINK;

	int nErr = SB_OK;

	if (bWasAborted)
	{

	}

exit:

	return nErr;
}

int OrionSSAG::CCGetChipSize(const enumCameraIndex& Camera, const enumWhichCCD& CCD, const int& nXBin, const int& nYBin, const bool& bOffChipBinning, int& nW, int& nH, int& nReadOut)
{
	X2MutexLocker ml(GetMutex());

	m_CachedCam = CCD;
	m_CachedBinX = nXBin;
	m_CachedBinY = nYBin;

	nW = width()/nXBin;
	nH = height()/nYBin;

	return SB_OK;
}

int OrionSSAG::CCGetNumBins(const enumCameraIndex& Camera, const enumWhichCCD& CCD, int& nNumBins)
{
	X2MutexLocker ml(GetMutex());

	if (!m_bLinked)
		nNumBins = 4;
	else
	{
		nNumBins = 5;
	}

	return SB_OK;
}

int OrionSSAG::CCGetBinSizeFromIndex(const enumCameraIndex& Camera, const enumWhichCCD& CCD, const int& nIndex, long& nBincx, long& nBincy)
{
	X2MutexLocker ml(GetMutex());

	switch (nIndex)
	{
		case 0:		nBincx=nBincy=1;break;
		case 1:		nBincx=nBincy=2;break;
		case 2:		nBincx=nBincy=3;break;
		case 3:		nBincx=nBincy=4;break;
		case 4:		nBincx=nBincy=5;break;
		default:	nBincx=nBincy=1;break;
	}

	return SB_OK;
}

int OrionSSAG::CCUpdateClock(void)
{   
	X2MutexLocker ml(GetMutex());

	return SB_OK;
}

int OrionSSAG::CCSetShutter(bool bOpen)           
{   
	X2MutexLocker ml(GetMutex());

	return SB_OK;;
}

int OrionSSAG::CCActivateRelays(const int& nXPlus, const int& nXMinus, const int& nYPlus, const int& nYMinus, const bool& bSynchronous, const bool& bAbort, const bool& bEndThread)
{   
	X2MutexLocker ml(GetMutex());
	return SB_OK;
}

int OrionSSAG::CCPulseOut(unsigned int nPulse, bool bAdjust, const enumCameraIndex& Cam)
{   
	X2MutexLocker ml(GetMutex());
	return SB_OK;
}

void OrionSSAG::CCBeforeDownload(const enumCameraIndex& Cam, const enumWhichCCD& CCD)
{
	X2MutexLocker ml(GetMutex());
}

void OrionSSAG::CCAfterDownload(const enumCameraIndex& Cam, const enumWhichCCD& CCD)
{
	X2MutexLocker ml(GetMutex());
	return;
}

int OrionSSAG::CCReadoutLine(const enumCameraIndex& Cam, const enumWhichCCD& CCD, const int& pixelStart, const int& pixelLength, const int& nReadoutMode, unsigned char* pMem)
{   
	X2MutexLocker ml(GetMutex());
	return SB_OK;
}           

int OrionSSAG::CCDumpLines(const enumCameraIndex& Cam, const enumWhichCCD& CCD, const int& nReadoutMode, const unsigned int& lines)
{                                     
	X2MutexLocker ml(GetMutex());
	return SB_OK;
}           


int OrionSSAG::CCReadoutImage(const enumCameraIndex& Cam, const enumWhichCCD& CCD, const int& nWidth, const int& nHeight, const int& nMemWidth, unsigned char* pMem)
{
	X2MutexLocker ml(GetMutex());

	if (!m_bLinked)
		return ERR_NOLINK;

	return SB_OK;
}

int OrionSSAG::CCDisconnect(const bool bShutDownTemp)
{
	X2MutexLocker ml(GetMutex());

	if (m_bLinked)
	{
		setLinked(false);
	}

	return SB_OK;
}

int OrionSSAG::CCSetImageProps(const enumCameraIndex& Camera, const enumWhichCCD& CCD, const int& nReadOut, void* pImage)
{
	X2MutexLocker ml(GetMutex());

	return SB_OK;
}

int OrionSSAG::CCGetFullDynamicRange(const enumCameraIndex& Camera, const enumWhichCCD& CCD, unsigned long& dwDynRg)
{
	X2MutexLocker ml(GetMutex());

	return SB_OK;
}

void OrionSSAG::CCMakeExposureState(int* pnState, enumCameraIndex Cam, int nXBin, int nYBin, int abg, bool bRapidReadout)
{
	X2MutexLocker ml(GetMutex());

	return;
}

int OrionSSAG::CCSetBinnedSubFrame(const enumCameraIndex& Camera, const enumWhichCCD& CCD, const int& nLeft, const int& nTop, const int& nRight, const int& nBottom)
{
	X2MutexLocker ml(GetMutex());

	return SB_OK;
}


int OrionSSAG::CCSettings(const enumCameraIndex& Camera, const enumWhichCCD& CCD)
{
	X2MutexLocker ml(GetMutex());

	return ERR_NOT_IMPL;
}

int OrionSSAG::CCSetFan(const bool& bOn)
{
	X2MutexLocker ml(GetMutex());

	return SB_OK;
}

int	OrionSSAG::pathTo_rm_FitsOnDisk(char* lpszPath, const int& nPathSize)
{
	X2MutexLocker ml(GetMutex());

	if (!m_bLinked)
		return ERR_NOLINK;

	//Just give a file path to a FITS and TheSkyX will load it
		
	return SB_OK;
}

CameraDriverInterface::ReadOutMode OrionSSAG::readoutMode(void)		
{
	X2MutexLocker ml(GetMutex());

	return CameraDriverInterface::rm_FitsOnDisk;
}


enumCameraIndex	OrionSSAG::cameraId()
{
	X2MutexLocker ml(GetMutex());

	return m_Camera;
}

void OrionSSAG::setCameraId(enumCameraIndex Cam)	
{
	m_Camera = Cam;
}

int OrionSSAG::width()
{
	int nDef = 512;

	if (GetBasicIniUtil())
		return GetBasicIniUtil()->readInt(KEY_X2CAM_ROOT, KEY_WIDTH, nDef);
	else
		return nDef;

}
int OrionSSAG::height()
{
	int nDef = 512;

	if (GetBasicIniUtil())
		return GetBasicIniUtil()->readInt(KEY_X2CAM_ROOT, KEY_HEIGHT, nDef);
	else
		return nDef;
}

void OrionSSAG::setWidth(const int& nValue )
{
	if (GetBasicIniUtil())
		GetBasicIniUtil()->writeInt(KEY_X2CAM_ROOT, KEY_WIDTH, nValue);
}

void OrionSSAG::setHeight(const int& nValue )
{

	if (GetBasicIniUtil())
		GetBasicIniUtil()->writeInt(KEY_X2CAM_ROOT, KEY_HEIGHT, nValue);

}

int OrionSSAG::execModalSettingsDialog()
{
	int nErr = SB_OK;
	X2ModalUIUtil uiutil(this, GetTheSkyXFacadeForDrivers());
	X2GUIInterface*					ui = uiutil.X2UI();
	X2GUIExchangeInterface*			dx = NULL;//Comes after ui is loaded
	bool bPressedOK = false;

	if (NULL == ui)
		return ERR_POINTER;

	if (nErr = ui->loadUserInterface("ssagcamera.ui", deviceType(), m_nPrivateISIndex))
		return nErr;

	if (NULL == (dx = uiutil.X2DX()))
		return ERR_POINTER;

	//Intialize the user interface

	//Check USB interface
	dx->setChecked("radioButton",true);

	//Populate the USB Interface combo box and set the current index (selection)
	dx->comboBoxAppendString("comboBox","Camera A");
	dx->comboBoxAppendString("comboBox","Camera B");
	dx->comboBoxAppendString("comboBox","Camera C");
	dx->setCurrentIndex("comboBox",2);

	//dx->invokeMethod("comboBox","clear");

	//Disable ethernet items
	dx->setEnabled("radioButton_2",false);
	dx->setEnabled("comboBox_2",false);
	dx->setEnabled("pushButton",false);

	//Status Indicators
	dx->setChecked("checkBox",true);
	dx->setChecked("checkBox_2",true);

	//Populate the Fan Mode 
	dx->comboBoxAppendString("comboBox_3","Quite");
	dx->comboBoxAppendString("comboBox_3","Ultra Quite");
	dx->comboBoxAppendString("comboBox_3","Really Quite");
	dx->setCurrentIndex("comboBox_3",1);

	//Imaging Options
	//Do Camera Gain
	dx->comboBoxAppendString("comboBox_4","High");
	dx->comboBoxAppendString("comboBox_4","Medium");
	dx->comboBoxAppendString("comboBox_4","Low");
	dx->setCurrentIndex		("comboBox_4",2);
	
	//Do Shutter Priority
	dx->comboBoxAppendString("comboBox_5","Mechanical");
	dx->comboBoxAppendString("comboBox_5","Digital");
	dx->comboBoxAppendString("comboBox_5","Normal");
	dx->setCurrentIndex		("comboBox_5",1);
	
	//Do Anti-blooming
	dx->comboBoxAppendString("comboBox_6","Normal");
	dx->comboBoxAppendString("comboBox_6","Mid-level");
	dx->comboBoxAppendString("comboBox_6","low");
	dx->setCurrentIndex		("comboBox_6",2);
	
	//Do Pre-exposure Flush
	dx->comboBoxAppendString("comboBox_7","Normal");
	dx->comboBoxAppendString("comboBox_7","Strong");
	dx->comboBoxAppendString("comboBox_7","Weak");
	dx->setCurrentIndex		("comboBox_7",0);
	
	//Check show D/L progress
	dx->setChecked("checkBox_3",true);

	//Do Optimization
	dx->setChecked("radioButton_4",true);
	dx->setEnabled("radioButton_4",false);
	dx->setEnabled("radioButton_3",false);

	//Cooling Control
	dx->setPropertyInt("doubleSpinBox","decimals", 3);
	dx->setPropertyDouble("doubleSpinBox","maximum", 15);
	dx->setPropertyDouble("doubleSpinBox","minimum", -15);
	dx->setPropertyDouble("doubleSpinBox","singleStep", 0.5);
	dx->setPropertyDouble("doubleSpinBox","value", 5.23);

	//Disable the filter button
	dx->setEnabled("pushButton_2",false);

	//Show a photo using the X2_PhotoFileName property 
	//The photo must reside in the same folder as the .ui file
	//Once set, the photo is static and cannot change
	//Requires TheSky build 8504 or later.
	dx->setPropertyString("label_8","X2_PhotoFileName", "testphoto.png");

	//Display the user interface
	if (nErr = ui->exec(bPressedOK))
		return nErr;

	//Retreive values from the user interface
	if (bPressedOK)
	{
		int nCamera, nFanMode, nGain, nShutterPriority, nAntiBlooming, nFlush;
		int bUSB, bLEDOn, bSoundOn, bDLStatus, bOptimizationImageQuality;
		double dSetpoint;

		//USB interface
		bUSB = dx->isChecked("radioButton");

		//Camera
		nCamera = dx->currentIndex("comboBox");

		//Status Indicators
		bLEDOn = dx->isChecked("checkBox");
		bSoundOn = dx->isChecked("checkBox_2");

		//Populate the Fan Mode 
		nFanMode = dx->currentIndex("comboBox_3");

		//Imaging Options
		nGain = dx->currentIndex("comboBox_4");
		
		//Shutter Priority
		nShutterPriority = dx->currentIndex("comboBox_5");
		
		//Anti-blooming
		nAntiBlooming = dx->currentIndex("comboBox_6");
		
		//Pre-exposure Flush
		nFlush = dx->currentIndex("comboBox_7");
		
		//D/L progress
		bDLStatus = dx->isChecked("checkBox_3");

		//Optimization
		bOptimizationImageQuality = dx->isChecked("radioButton_4");
		
		//Cooling Control
		dx->propertyDouble("doubleSpinBox","value", dSetpoint);

	}

	return nErr;
}

int OrionSSAG::doAddPixelDialogExample(int& xCoord, int& yCoord, bool& bPressedOK)
{
	int nErr = SB_OK;
	X2ModalUIUtil uiutil(this, GetTheSkyXFacadeForDrivers());
	X2GUIInterface*					ui = uiutil.X2UI();
	X2GUIExchangeInterface*			dx = NULL;
	bPressedOK = false;

	if (NULL == ui)
		return ERR_POINTER;

	if (nErr = ui->loadUserInterface("ssagaddpixel.ui", deviceType(), m_nPrivateISIndex))
		return nErr;

	if (NULL == (dx = uiutil.X2DX()))
		return ERR_POINTER;

	dx->setPropertyInt("spinBox","value", xCoord);
	dx->setPropertyInt("spinBox_2","value", yCoord);

	if (nErr = ui->exec(bPressedOK))
		return nErr;

	dx->propertyInt("spinBox","value", xCoord);
	dx->propertyInt("spinBox_2","value", yCoord);

	return nErr;
}
void OrionSSAG::uiEvent(X2GUIExchangeInterface* uiex, const char* pszEvent)
{
	char szEvt[DRIVER_MAX_STRING];

	sprintf(szEvt, pszEvent);

	//An example of showing another modal dialog
	if (!strcmp(szEvt, "on_pushButton_3_clicked"))
	{
		int nErr=SB_OK;
		bool bPressedOK = false;
		int xCoord=10;
		int yCoord=20;

		doAddPixelDialogExample(xCoord, yCoord, bPressedOK);

		if (bPressedOK)
		{
			int nRow=0;
			char szX[DRIVER_MAX_STRING];
			char szY[DRIVER_MAX_STRING];

			sprintf(szX,"%d",xCoord);
			sprintf(szY,"%d",yCoord);

			//Get the number of rows
			uiex->propertyInt("tableWidget","rowCount",nRow);

			//Increase rows by one
			uiex->setPropertyInt("tableWidget","rowCount",nRow+1);

			//Set the new pixel
			uiex->tableWidgetSetItem("tableWidget", nRow,0,szX);
			uiex->tableWidgetSetItem("tableWidget", nRow,1,szY);
		}
	}
	else
		if (!strcmp(szEvt, "on_pushButton_4_clicked"))
		{
			int nCurRow=-1;

			uiex->tableWidgetCurrentRow("tableWidget",nCurRow);

			if (-1!=nCurRow)
				uiex->tableWidgetRemoveRow("tableWidget",nCurRow);

		}
	else
		if (!strcmp(szEvt, "on_timer"))
		{
			static int nCount;
			char szCount[DRIVER_MAX_STRING];

			++nCount;
			sprintf(szCount,"Elapsed seconds = %d",nCount);

			uiex->setText("label_7",szCount);
		}

}
