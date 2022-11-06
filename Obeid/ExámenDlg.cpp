// ExámenDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Exámen.h"
#include "ExámenDlg.h"
#include <HD/hd.h>// Library of haptic device
#include <HDU/hduError.h>
#include <math.h> // Math operations
#include "mmsystem.h" // Multimedia timer
//#include "analysis.h" // Matrix operations,

#define pi 3.1415926535
#define MAX_GRAF_ROWS 40000
HHD hHDm;
bool initialized = false, schedulerStarted = false; // user flags
double taum[3] = {0.0,0.0,0.0};
HDSchedulerHandle servoLoopHandle;
const double T = 0.001; // Sample time
const int n = 3; // Number of joints

int indx = 0;
const int grafSkip = 0;

double qm[3] = {0.0};
const double angle_final_effector = 15.0*pi/180.0;
const double a2 = 0.145, a3 = sqrt(.135*.135 + .04*.04 - 2.0*0.135*.04*cos(pi-angle_final_effector));
const double gamma_final_effector = asin(0.04*sin(pi-angle_final_effector)/a3);
double grafi[MAX_GRAF_ROWS][25] = {0.0};

//Timers
MMRESULT HomeTimerID;
MMRESULT SmcTimerID;
//bool iCTele=true, PID= true;
//Banderas
bool iCHome = true, Home=true, homeCompletedFlag=true;
bool iCSmc = true, Smc=true, SmcCompletedFlag=true;
//Bandera para matar toos los timers
bool CompletedFlag = true;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

CExámenDlg::CExámenDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExámenDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CExámenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ENCODER4, m_time);
	DDX_Control(pDX, IDC_ENCODER1, m_Encoder1);
	DDX_Control(pDX, IDC_ENCODER5, m_statusTextBox);
	DDX_Control(pDX, IDC_ENCODER2, m_Encoder2);
	DDX_Control(pDX, IDC_ENCODER3, m_Encoder3);
}

BEGIN_MESSAGE_MAP(CExámenDlg, CDialog)
	//ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_INITIALIZE, &CExámenDlg::OnBnClickedInitialize)
	ON_BN_CLICKED(IDC_CALIB, &CExámenDlg::OnBnClickedCalib)
	ON_BN_CLICKED(IDC_REED, &CExámenDlg::OnBnClickedReed)
	ON_BN_CLICKED(IDC_PID, &CExámenDlg::OnBnClickedHome)
	ON_BN_CLICKED(IDC_SMC, &CExámenDlg::OnBnClickedSmc)
END_MESSAGE_MAP()


// CExámenDlg message handlers

BOOL CExámenDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

/*void CExámenDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}*/

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CExámenDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.

void CExámenDlg::OnClose() // When close button... 
{
	timeEndPeriod(1);
//Matamos todos los timers
	if(!Smc || !Home)
	{
		timeKillEvent(SmcTimerID);
		CompletedFlag = Smc = Home = true;
	}	
	if (initialized&&hdIsEnabled(HD_FORCE_OUTPUT))
		hdDisable(HD_FORCE_OUTPUT);
	hdUnschedule(servoLoopHandle);
	if(schedulerStarted)
		hdStopScheduler();
	if(initialized)
	{
		hdDisableDevice(hHDm);
	}

	FILE *outFile;
	if(fopen_s(&outFile, "DATOS.m","w")!=0){
		MessageBox(_T("No se pudo crear el archivo para graficar"));
	}
	else{
		for(int i=0; i<indx; i++){
			fprintf(outFile, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",grafi[i][0],grafi[i][1], grafi[i][2], grafi[i][3], grafi[i][4], grafi[i][5], grafi[i][6], grafi[i][7], grafi[i][8], grafi[i][9], grafi[i][10], grafi[i][11], grafi[i][12], grafi[i][13], grafi[i][14], grafi[i][15], grafi[i][16], grafi[i][17], grafi[i][18], grafi[i][19], grafi[i][20], grafi[i][21],grafi[i][21], grafi[i][23], grafi[i][24]);}
		fclose(outFile);
	}
	exit(0);
}

HCURSOR CExámenDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

HDCallbackCode HDCALLBACK CalibrationStatusCallback(void * pUserData)
{
	HDenum *pStatus = (HDenum *) pUserData;

    hdBeginFrame(hHDm);
	hdUpdateCalibration(HD_CALIBRATION_INKWELL);
    *pStatus = hdCheckCalibration();
    hdEndFrame(hHDm);

    return HD_CALLBACK_DONE;
}

typedef struct 
{ hduVector3Dd position;

} DeviceStateStruct;
DeviceStateStruct state;

HDCallbackCode HDCALLBACK ServoLoopCallback(void *pUserData)
{
	DeviceStateStruct *pState = static_cast<DeviceStateStruct *>(pUserData);
	HDdouble torque[3];
	hdBeginFrame(hHDm);
	//hdGetDoublev(HD_CURRENT_POSITION,pos->position);
	hdGetDoublev(HD_CURRENT_JOINT_ANGLES,pState->position);
	
	torque[0] = -1000.0*taum[0];
	torque[1] = 1000.0*taum[1];
	torque[2] = 1000.0*taum[2];

	qm[0] = -state.position[0];
	qm[1] = state.position[1];
	qm[2] = state.position[2]-0.5*pi-qm[1]-gamma_final_effector;

	hdSetDoublev(HD_CURRENT_JOINT_TORQUE, torque);

    hdEndFrame(hHDm);

return HD_CALLBACK_CONTINUE;
}


void CExámenDlg::OnBnClickedInitialize()
{
	// TODO: Add your control notification handler code here
	HDErrorInfo error;
	HDstring MasterRobot = "Default Device";
	//HDstring SlaveRobot = "p2";
	hHDm = hdInitDevice(MasterRobot);
	if (HD_DEVICE_ERROR(error = hdGetError())) 
	{
        MessageBox(_T("Master Device not Found!"));
		return;
	}

	
	servoLoopHandle = hdScheduleAsynchronous(ServoLoopCallback, &state, HD_MAX_SCHEDULER_PRIORITY);

	hdMakeCurrentDevice(hHDm);
	if (!hdIsEnabled(HD_FORCE_OUTPUT))
		hdEnable(HD_FORCE_OUTPUT);

	if (HD_DEVICE_ERROR(error = hdGetError()))
		MessageBox(_T("Force output enable error!"));

	if(!schedulerStarted)
		{
			hdStartScheduler();
			schedulerStarted = true;
			Sleep(1500);
		}
	

    if (HD_DEVICE_ERROR(error = hdGetError()))
    {
		MessageBox(_T("Servo loop initialization error"));

        hdDisableDevice(hHDm);
         exit(-1);        
    }
	else
	{
		initialized = true;
		m_statusTextBox.SetWindowTextW(_T("*** Phantom Robot initialized ***"));
	}
	
	timeBeginPeriod(1);
}

void CExámenDlg::OnBnClickedCalib()
{
	// TODO: Add your control notification handler code here
	if(initialized)
	{
		int supportedCalibrationStyles;
	    int calibrationStyle;
		HDErrorInfo error;

		hdGetIntegerv(HD_CALIBRATION_STYLE, &supportedCalibrationStyles);

		if (supportedCalibrationStyles & HD_CALIBRATION_INKWELL)
		    calibrationStyle = HD_CALIBRATION_INKWELL;
		else
		{
			MessageBox(_T(" Sorry, no ink-well calibration available "));
			return;
		}
		
		if (HD_DEVICE_ERROR(error = hdGetError()))
			m_statusTextBox.SetWindowTextW(_T("*** Failed to start the scheduler ***"));           

			HDenum status;
			hdScheduleSynchronous(CalibrationStatusCallback, &status, HD_DEFAULT_SCHEDULER_PRIORITY);

		if (status == HD_CALIBRATION_NEEDS_MANUAL_INPUT)
			MessageBox(_T(" Please put the device into the ink-well "));
		else
		{
			m_statusTextBox.SetWindowTextW(_T("*** Calibration done ***")); 
		}
	
		return;
	}
	else
	{
		MessageBox(_T(" Please initialize first the Phantom device "));
	}
}


void CExámenDlg::OnBnClickedReed()
{	
	CString text[3];
	text[0].Format(L"%.3f",qm[0]*180.0/pi);
	text[1].Format(L"%.3f",qm[1]*180.0/pi);
	text[2].Format(L"%.3f",qm[2]*180.0/pi);
	m_Encoder1.SetWindowTextW(text[0]);
	m_Encoder2.SetWindowTextW(text[1]);
	m_Encoder3.SetWindowTextW(text[2]);
 
	return;
}


void CExámenDlg::OnBnClickedHome()
{

	if(!iCHome)
	{
		timeKillEvent(HomeTimerID);
		homeCompletedFlag = iCHome = true;
	}
	if(!iCSmc)
	{
		timeKillEvent(SmcTimerID);
		//SmcCompletedFlag = iCSmc = true;
	}

	HomeTimerID = timeSetEvent( T*1000, 0, HomeTimerProc, 0, TIME_PERIODIC);	// Home timer initialization


//HomeTimerID = timeSetEvent( static_cast<UINT>(1000*T), 0, HomeTimerProc, 0, TIME_PERIODIC);	// Home timer initialization
	
}

void CALLBACK CExámenDlg::HomeTimerProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{   static int grafFlag = 0; //Bandera para gráficar
	static double tini = 0.0; //Tiempo inicial
	static double tf = 2.0;	//Tiempo que dura la trayectoria
	static double bm0[n] = {0.0}, bm3[n] = {0.0}, bm4[n] = {0.0}, bm5[n] = {0.0};	//Coeficientes del polinomio
	const double qmdf[n] = {0.0*pi/180.0, 90.0*pi/180.0, -90.0*pi/180.0};	//Punto final (0 , 90 , -90 )
	double t = 0.0;	//Variable tiempo
	double qmd[n] = {0.0};	//Polinomio para la trayectoria  qm deseada
	static double em_1[n] = {0.0};	//Error anterior
	double em[n] = {0.0}, emp[n] = {0.0}, emi[n] = {0.0};	//Error, Error derivativo, Error integral
	const double kpm[n] = {1.2,1.2,1.2};	//Ganancias Proporcional
	const double kim[n] = {0.2,0.2,0.2}; 	//Ganancias Integral   
	const double kdm[n] = {0.1,0.1,0.1}; //Ganancias Derivativa
	double qpmd[3] = {0.0};

	CString TIEMPO;

	CExámenDlg *pMainWnd = (CExámenDlg *) AfxGetApp()->m_pMainWnd;	
	
	if(iCHome)
	{  
	tini = timeGetTime();

	//Cálculo de los coeficioentes del polinomio, solo se hace una vez 
	for(int i=0; i<n; i++)
		{
		bm0[i] = qm[i];
		bm3[i] = 10*(qmdf[i]-qm[i])/(tf*tf*tf);
		bm4[i] = -15*(qmdf[i]-qm[i])/(tf*tf*tf*tf);
		bm5[i] = 6*(qmdf[i]-qm[i])/(tf*tf*tf*tf*tf);
		}
		
		pMainWnd->m_statusTextBox.SetWindowTextW(_T("*** Reaching home position ***")); 
		
	}
	t = (timeGetTime() - tini)/1000.0;
	if(t<=tf)
	{
	TIEMPO.Format(_T("%f"),t);
	pMainWnd->m_time.SetWindowTextW(TIEMPO);
	}
	else{
	t=tf;
	TIEMPO.Format(_T("%f"),t);
	pMainWnd->m_time.SetWindowTextW(TIEMPO);
	}

	
	
	

	if(tini<=0.001)
	{
		for(int i=0; i<n; i++)
		{
		bm0[i]=qm[i];
		}
	}

	
	for(int i=0; i<n; i++)
	{
		if(t<=tf)
		{
            qmd[i] = bm5[i]*(t*t*t*t*t)+bm4[i]*(t*t*t*t)+bm3[i]*(t*t*t)+bm0[i];
			qpmd[i] = 5*bm5[i]*(t*t*t*t)+4*bm4[i]*(t*t*t)+3*bm3[i]*(t*t);
		}
		else
		{
			qmd[i] = qmdf[i];
		}
	}
	
	//Definimos el error
	for(int i=0; i<n; i++)
	{
		em[i] = qm[i] - qmd[i];
		if(iCHome)
		{
			em_1[i] =em[i];
		}
	}

	//Definimos la integral y almacenamos el valor actual en variable auxiliar em_1[];
		for(int i=0; i<n; i++)
	{
		emi[i] += em[i]*T;
		em_1[i]  = em[i];
	}
		for(int i=0; i<n; i++)
	{
	//Definimos la derivada del error
		emp[i] = (em[i]-em_1[i])/T;
	}
	//Ley de control PID
	for(int i=0; i<n; i++) 
	{
		taum[i] = -kpm[i]*em[i]-kim[i]*emi[i]-kdm[i]*emp[i];
		//taum[i]=0;
	}	
	
	if(t>tf&&CompletedFlag)
	{
		pMainWnd->m_statusTextBox.SetWindowTextW(_T("*** Home position ***")); 
		CompletedFlag = false;
	}

	if(iCHome)
		iCHome = false;

	return;
}



double CExámenDlg::Signo(double f)
{
  static double signo=0.0;
	if(f>0.0)
		signo=1.0;
	else if(f<0.0)
		signo=-1.0;
	else
		signo=0.0;
return signo;
}

double CExámenDlg::RED1(double posq1)
{
	const double lambda= 5.5; 
	const double alfa = 0.70;
	static double u= 0.0;
	static double u1= 0.0;
	static double dx= 0.0;
	static double x= 0.0;
	static double du= 0.0;
	static double du1= 0.0;
	static double aux1= 0.0;
	static double aux2= 0.0;
	//static double REDV1= 0.0

	aux1= abs(x-posq1);
	aux2= 0.5;
	
	u1=u1+du1*T;
	x=x+dx*T;
	u=u1-lambda*(pow(aux1,aux2))*(Signo(x - posq1));
	dx=u;
	du1=-alfa*Signo(x-posq1);


return u;
}


double CExámenDlg::RED2(double posq2)
{
	const double lambda= 1.5; 
	const double alfa = 0.80;
	//static double u= 1.4;
	static double u= 0.0;
	static double u1= 0.0;
	static double dx= 0.0;
	static double x= 1.57;
	static double du= 0.0;
	static double du1= 0.0;
	//static double REDV1= 0.0

	dx=u;
	u=u1-lambda*(pow(abs(x - posq2),0.5))*(Signo(x - posq2)) ;
	du1=-alfa*Signo(x-posq2);

	u1+=du1*T;
	x+=dx*T;

return u;
}

double CExámenDlg::RED3(double posq3)
{
	const double lambda= 1.5; 
	const double alfa = 0.80;
	//static double u= -1.66;
	static double u= 0.0;
	static double u1= 0.0;
	static double dx= 0.0;
	static double x= -1.57;
	static double du= 0.0;
	static double du1= 0.0;
	//static double REDV1= 0.0

	dx=u;
	u=u1-lambda*(pow(abs(x - posq3),0.5))*(Signo(x - posq3)) ;
	du1=-alfa*Signo(x-posq3);

	u1+=du1*T;
	x+=dx*T;

return u;
}




double CExámenDlg::Levantq1(double pos1)
{
	const double lambda_cero = 2.0; 
	const double lambda_uno = 1.5;
	const double lambda_dos = 1.1;
	const double L = 3.0;
	static double dZ_0= 0.0;
	static double dZ_1= 0.0;
	static double dZ_2= 0.0;

	static double Z0= 0.0;
	static double Z1= 0.0;
	static double Z2= 0.0;

	static double V0 = 0.0;
	static double V1 = 0.0;
V0 = -(lambda_dos)*(pow(L,0.33))*(pow(abs(Z0 - pos1),0.66))*(Signo(Z0 - pos1)) + Z1;
V1 = -(lambda_uno)*(pow(L,0.5))*(pow(abs(Z1 - pos1),0.5))*(Signo(Z1 - V0)) + Z2;
dZ_2 = -lambda_cero*L*(Signo(Z2 - V1)); 

dZ_0 = V0;
dZ_1 = V1;
Z0+=dZ_0*T;
Z1+=dZ_1*T;
Z2+=dZ_2*T;
return Z1;
}

double CExámenDlg::Levantq2(double pos2)
{
	const double lambda_cero = 2.0; 
	const double lambda_uno = 1.5;
	const double lambda_dos = 1.1;
	const double L = 3.0;
	static double dZ_0= 0.0;
	static double dZ_1= 0.0;
	static double dZ_2= 0.0;

	static double Z0= 1.65;
	static double Z1= 0.0;
	static double Z2= 0.0;

	static double V0 = 0.0;
	static double V1 = 0.0;
V0 = -(lambda_dos)*(pow(L,1/3))*(pow((abs(Z0 - pos2)),(2/3)))*(Signo(Z0 - pos2)) + Z1;
V1 = -(lambda_uno)*(pow(L,1/2))*(pow((abs(Z1 - pos2)),(1/2)))*(Signo(Z1 - V0)) + Z2;
dZ_2 = -(lambda_cero)*L*(Signo(Z2 - V1)); 

dZ_0 = V0;
dZ_1 = V1;
Z0+=dZ_0*T;
Z1+=dZ_1*T;
Z2+=dZ_2*T;

return Z1;
}
double CExámenDlg::Levantq3(double pos3)
{
	const double lambda_cero = 2.0; 
	const double lambda_uno = 1.5;
	const double lambda_dos = 1.1;
	const double L =3.0;
	static double dZ_0= 0.0;
	static double dZ_1= 0.0;
	static double dZ_2= 0.0;

	static double Z0= -1.74;
	static double Z1= 0.0;
	static double Z2= 0.0;

	 static double V0 = 0.0;
	 static double V1 = 0.0;
V0 = -(lambda_dos)*(pow(L,1/3))*(pow((abs(Z0 - pos3)),(2/3)))*(Signo(Z0 - pos3)) + Z1;
V1 = -(lambda_uno)*(pow(L,1/2))*(pow((abs(Z1 - pos3)),(1/2)))*(Signo(Z1 - V0)) + Z2;
dZ_2 = -(lambda_cero)*L*(Signo(Z2 - V1)); 

dZ_0 = V0;
dZ_1 = V1;
Z0+=dZ_0*T;
Z1+=dZ_1*T;
Z2+=dZ_2*T;

return Z1;
}



void CExámenDlg::OnBnClickedSmc()
{
	if(!iCHome)
	{
		timeKillEvent(HomeTimerID);
	}

	if(!iCSmc)
	{
		timeKillEvent(SmcTimerID);
	    SmcCompletedFlag = iCSmc = true;
	}

	SmcTimerID = timeSetEvent( T*1000, 0, SmcTimerProc, 0, TIME_PERIODIC);
}// TODO: Add your control notification handler code here


void CALLBACK CExámenDlg::SmcTimerProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{	
	static int grafFlag = 0; //Bandera para gráficar
	static double tini2 = 0.0; //Tiempo inicial
	const double qmdf[n] = {0.0*pi/180.0, 90.0*pi/180.0, -90.0*pi/180.0};	//Punto final (0 , 90 , -90 )
	double t = 0.0;
	CString TIEMPO2;

	const double qmd[n]={0.0, 0.0, 0.0};
	static double qm_1[n]={0.0*pi/180.0, 90.0*pi/180.0, -90.0*pi/180.0};
	double dqm[n]={0.0,0.0,0.0};

	const double lambda_cero[n] = {2.0, 2.0, 2.0}; 
	const double lambda_uno[n] = {1.5, 1.5, 1.5};
	const double lambda_dos[n] = {1.1, 1.1, 1.1};

//sf PAPER BUENO
	/*const double K1[n]={12.0, 20.0, 18.0 };
	const double K2[n]={0.05, .05, .005};
	const double K3[n]={0.05, .05, .005};
	const double K4[n]={0.16, 0.28, 0.22};*/
//cf	
	/*const double K1[n]={12.0, 20.0, 18.0 };
	const double K2[n]={0.05, .05, .005};
	const double K3[n]={0.05, .05, .005};
	const double K4[n]={0.16, 0.28, 0.25};
	const double Kv[n]={0.08, 0.08, 0.08};*/

	//CI DIFERENTES
	const double K1[n]={11.0, 16.0, 12.0 };
	const double K2[n]={0.05, 0.08, 0.06};
	const double K3[n]={0.05, 0.05, 0.05};
	const double K4[n]={0.16, 0.28, 0.26};
	const double Kv[n]={0.08, 0.08, 0.08};	
	
	/* // 
	const double K1[n]={0.05, .05, .05};
	const double K2[n]={0.05, .05, .05};
	const double K3[n]={0.05, .05, .05};
	const double K4[n]={0.1, .1, .1};*/
	//const double  alpha=0.81;
	const double  alpha=9/11.0;
	//const double L = 3;

	double qd1=0.0;
	double qd2=0.0;
	double qd3=0.0;

	double dqd1=0.0;
	double dqd2=0.0;
	double dqd3=0.0;

	double dotePos[n]={0.0,0.0,0.0};
	double ePos[n]={0.0,0.0,0.0};
	double sq[n]={0.0,0.0,0.0};
	double dotqr[n]={0.0,0.0,0.0};
	double s[n]={0.0,0.0,0.0};
	double qd[n]={0.0,0.0,0.0};
    double dqd[n]={0.0,0.0,0.0};
	double dotsigma[n]={0.0,0.0,0.0};
	static double sigma[n]={0.0,0.0,0.0};
	/*double aux1[n]={0.0,0.0,0.0};
	double aux2[n]={0.0,0.0,0.0};
	double aux3[n]={0.0,0.0,0.0};
	float num[n]={0.0,0.0,0.0};
	float den[n]={0.0,0.0,0.0};*/
	double vel[n]={0.0,0.0,0.0};

	//Ganancias PID PARA REALIZAR LA COMPARACIÓN
	const double kp[n] = {1,.5,.8};	//Ganancias Proporcional
	const double ki[n] = {0.2,0.2,0.2}; 	//Ganancias Integral   
	const double kd[n] = {0.1,0.2,0.2};
	double ePosi[n] = {0.0,0.0,0.0};

	//GANANCIAS PARRA-VEGA
	double DOTQR[n]={0.0,0.0,0.0};
	double SD[n]={0.0,0.0,0.0};
	const double GAMMA[n]={0.4,0.4,0.4};
	static double SIGMA[n]={0.0,0.0,0.0};
	double DOTSIGMA[n]={0.0,0.0,0.0};
	double SQ[n]={0.0,0.0,0.0};
	const double ALPHA[n]={14.0,14.0,14.0};
	double S[n]={0.0,0.0,0.0};
	double SR[n]={0.0,0.0,0.0};
	const double KD[n]={0.35,0.35,0.35};

	CExámenDlg *pMainWnd = (CExámenDlg *) AfxGetApp()->m_pMainWnd;	


	if(iCSmc)
	{  
	tini2 = timeGetTime();
	pMainWnd->m_statusTextBox.SetWindowTextW(_T("*** Control in progress ***"));

	for(int i=0; i<n; i++){
		qm_1[i]=qm[i];
	}

	}

	t = (timeGetTime() - tini2)/1000.0;

	TIEMPO2.Format(_T("%f"),t);
	pMainWnd->m_time.SetWindowTextW(TIEMPO2);

	/*if(T==0)
	{
	for(int i=0; i<n; i++)
		{
	//Definimos la derivada del error
		vel[i] = 0.0;
		}
	}
	else
	{
/*for(int i=0; i<n; i++)
		{
	//Definimos la derivada del error
		dqm[i] = (qm[i]-qm_1[i])/T;
		qm_1[i] = qm[i];
		//dqmf[i] =
		}
	}*/

	//Levant
	vel[1]=RED2(qm[1]);
	vel[2]=RED3(qm[2]);
	vel[0]=RED1(qm[0]);

	//vel[2]=0;

	

// Generación de la trayectoria
    
   /* qd1= .4*sin(.1*t);
    qd2= 1.37+.2*cos(.1*t);//%45 Grados +..
    qd3= -1.423+.15*cos(.1*t);//%90 Grados +..
    */
	/*qd[0]=-.4+.4*cos(t);
	qd[1]=1.23+.2*cos(t);
	qd[2]=-1.6-.1*cos(t);*/
//CI DIF >0
	/*
	qd[0]=-.42+.4*cos(3*t);
	qd[1]=1.25+.2*cos(t);
	qd[2]=-1.63-.1*cos(t);
	*/

	//CI DIF >>0
	
	qd[0]=-.4+.4*cos(3*t);
	qd[1]=1.37+.2*cos(t);
	qd[2]=-1.37-.2*cos(2*t);

	//dqd[0]=-.4*sin(3*t);
	dqd[0]=-.12*sin(3*t);
	dqd[1]=-.2*sin(t);
	dqd[2]=.4*sin(2*t);


//Control Javier

	for(int i=0; i<n; i++)
		{
	    ePos[i] = qm[i] - qd[i]; //% Error de posición        
		}      
       
	for(int i=0; i<n; i++)  
	{     
		dotePos[i]=vel[i]-dqd[i];
	}
    for(int i=0; i<n; i++)      
	{
		sq[i]=dotePos[i]+K1[i]*Signo(ePos[i])*(pow((abs(ePos[i])),alpha));
	}
    /*
%      if abs(sq(i))==0
%         w(i)=0;
%      else
%         w(i)=sq(i)/(abs(sq(i)));
%      end
%     */

	for(int i=0; i<n; i++)
	{  
		/*num[i]= abs(ePos[i]);
	aux2[i]=abs(ePos[i]);
	aux1[i]=-K1[i]*pow(aux2[i],alpha);*/
//aux3[i]=aux2[i]*aux1[i];
	dotqr[i] =  dqd[i]-K1[i]*Signo(ePos[i])*pow((abs(ePos[i])),alpha)-K2[i]*sigma[i];
//%     dotsigma(i)=K3(i)*sq(i)+w(i);
    s[i]=dqm[i]-dotqr[i];

	dotsigma[i]=K3[i]*sq[i]+Signo(sq[i]);
	}

	for(int i=0; i<n; i++) 
	{
		sigma[i]+=dotsigma[i]*T;
	}
//Ley de control NL
/*for(int i=0; i<n; i++) 
	{
	taum[i] = -K4[i]*tanh(s[i]);
	//cf
	//taum[i] = -K4[i]*tanh(s[i])-Kv[i]*vel[i];
	//taum[i]=0;
	}
*/
	//CONTROLADORES
//PARRA VEGA 2002
for(int i=0; i<n; i++)
	{SIGMA[i]+=DOTSIGMA[i]*T;
		DOTQR[i] = dqd[i]-ALPHA[i]*ePos[i]+SD[i]-GAMMA[i]*SIGMA[i];
		DOTSIGMA[i]=Signo(SQ[i]);
		S[i]=dotePos[i]+ALPHA[i]*ePos[i];
		SD[i]=0;
		SQ[i]=S[i]-SD[i];
		SR[i]=SQ[i]+GAMMA[i]*SIGMA[i];
	}

	//Ley de control PID
for(int i=0; i<n; i++)
	{
		ePosi[i] += ePos[i]*T;	
	}
	for(int i=0; i<n; i++) 
	{
		//taum[i] = -K4[i]*tanh(s[i])-Kv[i]*vel[i];
		taum[i] = -kp[i]*ePos[i]-ki[i]*ePosi[i]-kd[i]*dotePos[i];
		//taum[i] = -K4[i]*tanh(s[i]);
		//taum[i] = -KD[i]*SR[i];
		//taum[i]=0;
	}

	//if(PID)
	//	PID = false;

	if(iCSmc)
	indx = 0;	
	if(grafFlag==0)
	{	
	grafi[indx][0] = t;
	grafi[indx][1] = qm[0]*180.0/pi;
	grafi[indx][2] = qm[1]*180.0/pi;
	grafi[indx][3] = qm[2]*180.0/pi;
	grafi[indx][4] = dotePos[0]*180.0/pi;
	grafi[indx][5] = dotePos[1]*180.0/pi;
	grafi[indx][6] = dotePos[2]*180.0/pi;
	grafi[indx][7] = ePos[0]*180.0/pi;
	grafi[indx][8] = ePos[1]*180.0/pi;
	grafi[indx][9] = ePos[2]*180.0/pi;
	grafi[indx][10] = qd[0]*180.0/pi;
	grafi[indx][11] = qd[1]*180.0/pi;
	grafi[indx][12] = qd[2]*180.0/pi;
	grafi[indx][13] = abs(taum[0]);	
	grafi[indx][14] = abs(taum[1]);
	grafi[indx][15] = abs(taum[2]);
	grafi[indx][16] = dqd[0]*180.0/pi;
	grafi[indx][17] = dqd[1]*180.0/pi;
	grafi[indx][18] = dqd[2]*180.0/pi;
	grafi[indx][19] = vel[0]*180.0/pi;
	grafi[indx][20] = vel[1]*180.0/pi;
	grafi[indx][21] = vel[2]*180.0/pi;
	grafi[indx][22] = dotqr[0];
	grafi[indx][23] = dotqr[1];
	grafi[indx][24] = dotqr[2];

	/*grafi[indx][7] = z1a;
	grafi[indx][8] = z12a;
	grafi[indx][9] = z14a;
	grafi[indx][10] = z1;
	grafi[indx][11] = z12;
	grafi[indx][12] = z14;*/

	indx++;
		grafFlag = grafSkip+1;
	}

	grafFlag--;

	if(iCSmc){
	 iCSmc = false;
	}
	return;

}

//***** Diferenciador de Levant *****    
    // *** Derivada de la posición

	/*
	static double dZ0[n]={0.0, 0.0, 0.0};
	static double dZ1[n]={0.0, 0.0, 0.0};
	static double dZ2[n]={0.0, 0.0, 0.0};

	

	const double lambda_cero[n]={2.0, 2.0, 2.0}; 
	const double lambda_uno[n]={1.5, 1.5, 1.5};
	const double lambda_dos[n]={1.1, 1.1, 1.1};

	const double K1[n]={0.01, 0.01, 0.01};
	const double K2[n]={.01, .01, .01};
	const double K3[n]={.001, .001, .001};
	const double K4[n]={.01, .01, .01};
	const double alpha=9/11;
	const double L = 3;
	
	static double V0[n]={0.0, 0.0, 0.0};
	static double V1[n]={0.0, 0.0, 0.0};
	static double V2[n]={0.0, 0.0, 0.0};

    static double Z0[n]={0.0*pi/180.0, 90.0*pi/180.0, -90.0*pi/180.0};
    static double Z1[n]={0.0, 0.0, 0.0};
    static double Z2[n]={0.0, 0.0, 0.0};

	static double V0_1[n]={0.0, 0.0, 0.0};
	static double V1_1[n]={0.0, 0.0, 0.0};
	static double V2_1[n]={0.0, 0.0, 0.0};

    static double Z0_1[n]={0.0, 0.0, 0.0};
    static double Z1_1[n]={0.0, 0.0, 0.0};
    static double Z2_1[n]={0.0, 0.0, 0.0};
	
	
	for(int i=0; i<n; i++) 
	{	VAUX[i]=Z0[i] - qm[i];
		V0[i] = -(lambda_dos[i])*(pow(L,(1/3))*pow((abs(Z0[i] - qm[i])),(2/3)))*(Signo(VAUX[i])) + Z1[i];
	}
	
	for(int i=0; i<n; i++) 
	{
		dZ0[i] = V0[i];
	}

	for(int i=0; i<n; i++) 
	{
		V1[i] = -(lambda_uno[i])*(pow(L,(1/2))*pow((abs(Z1[i] - qm[i])),(1/2)))*(Signo(Z1[i] - V0[i])) + Z2[i];	
	}

	for(int i=0; i<n; i++) 
	{
		dZ1[i] = V1[i];
	} 

// Derivada de la aceleración
	for(int i=0; i<n; i++) 
	{
		dZ2[i] = -(lambda_cero[i])*L*(Signo(Z2[i] - V1[i]));
	}

//Actualización de variables

	for(int i=0; i<n; i++) 
	{
		 Z0_1[i] = Z0[i];
		 Z1_1[i] = Z1[i];
		 Z2_1[i] = Z2[i];
	}
//Integramos
for(int i=0; i<n; i++) 
	{
		//Z0[i]=Z0_1[i] + dZ0[i]*T;
	Z0[i]+= dZ0[i]*T;
	}
for(int i=0; i<n; i++) 
	{
		//Z1[i]=Z1_1[i] + dZ1[i]*T;
		Z1[i]+=dZ1[i]*T;
	}
for(int i=0; i<n; i++) 
	{
		//Z2[i]= Z2_1[i] + dZ2[i]*T;
		Z2[i]+= dZ2[i]*T;
	}
	*/