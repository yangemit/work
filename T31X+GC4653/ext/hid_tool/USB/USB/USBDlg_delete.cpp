
// USBDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "USB.h"
#include "USBDlg.h"
#include "afxdialogex.h"
#include "hidapi.h"
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <WS2tcpip.h>
using namespace std;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#pragma  comment(lib,"hidapi")   //隐式调用库文件
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CUSBDlg 对话框



CUSBDlg::CUSBDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CUSBDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUSBDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PATH, m_editPath);
	DDX_Control(pDX, IDC_LIST_INFO, m_ListInfo);
	DDX_Control(pDX, IDC_EDIT_SN, m_Edit_SN);
	DDX_Control(pDX, IDC_EDIT_PATH_UVCATTR, m_editUVCATTR);
	DDX_Control(pDX, IDC_EDIT_PATH_UVCAPP, m_editUVCAPP);
	DDX_Control(pDX, IDC_EDIT_PATH_SENSOR_DRIVER, m_editSensor_driver);
	DDX_Control(pDX, IDC_EDIT_PATH_SENSOTR_SETTING, m_editSensor_setting);
	DDX_Control(pDX, IDC_EDIT_PATH_UVCAPP2, m_editUVCAPP2);
	DDX_Control(pDX, IDC_EDIT_CMEI, m_editPID);
	DDX_Control(pDX, IDC_EDIT_VID, m_editVID);
	DDX_Control(pDX, IDC_EDIT_BCD, m_editBCD);
	DDX_Control(pDX, IDC_EDIT_MANUFACTURE, m_editManufacture);
}

BEGIN_MESSAGE_MAP(CUSBDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CUSBDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_SCAN, &CUSBDlg::OnBnClickedButtonScan)
	ON_BN_CLICKED(IDC_BUTTON_SN, &CUSBDlg::OnBnClickedButtonSn)
	ON_BN_CLICKED(IDC_BUTTON_BURN, &CUSBDlg::OnBnClickedButtonBurn)
	ON_BN_CLICKED(IDC_BUTTON_VID, &CUSBDlg::OnBnClickedButtonVid)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_UVCATTR, &CUSBDlg::OnBnClickedButtonOpenUvcattr)
	ON_BN_CLICKED(IDC_BUTTON_BURN_UVCATTR, &CUSBDlg::OnBnClickedButtonBurnUvcattr)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_INFO, &CUSBDlg::OnNMCustomdrawListInfo)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_UVCAPP, &CUSBDlg::OnBnClickedButtonOpenUvcapp)
	ON_BN_CLICKED(IDC_BUTTON_BURN_UVCAPP, &CUSBDlg::OnBnClickedButtonBurnUvcapp)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_SENSOR_DRIVER, &CUSBDlg::OnBnClickedButtonOpenSensorDriver)
	ON_BN_CLICKED(IDC_BUTTON_BURN_SENSOR_DRIVER, &CUSBDlg::OnBnClickedButtonBurnSensorDriver)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_SENSOR_SETTINGS, &CUSBDlg::OnBnClickedButtonOpenSensorSettings)
	ON_BN_CLICKED(IDC_BUTTON_BURN_SENSOR_SETTINGS, &CUSBDlg::OnBnClickedButtonBurnSensorSettings)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CUSBDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CUSBDlg::OnBnClickedButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_UVCAPP2, &CUSBDlg::OnBnClickedButtonOpenUvcapp2)
	ON_BN_CLICKED(IDC_BUTTON_PID, &CUSBDlg::OnBnClickedButtonPid)
	ON_BN_CLICKED(IDC_BUTTON_MANUFACTURE, &CUSBDlg::OnBnClickedButtonManufacture)
	ON_BN_CLICKED(IDC_BUTTON_BCD, &CUSBDlg::OnBnClickedButtonBcd)
	ON_BN_CLICKED(IDC_BUTTON_BURN_UVCAPP2, &CUSBDlg::OnBnClickedButtonBurnUvcapp2)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CUSBDlg 消息处理程序

BOOL CUSBDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	LONG lStyle;
    lStyle = GetWindowLong(m_ListInfo.m_hWnd, GWL_STYLE);// 获取当前窗口style
    lStyle &= ~LVS_TYPEMASK; // 清除显示方式位
    lStyle |= LVS_REPORT; // 设置style
    SetWindowLong(m_ListInfo.m_hWnd, GWL_STYLE, lStyle);// 设置style
    DWORD dwStyle = m_ListInfo.GetExtendedStyle();
    dwStyle |= LVS_EX_FULLROWSELECT;// 选中某行使整行高亮（只适用与report 风格的listctrl ）
	dwStyle |= LVS_EX_GRIDLINES;// 网格线（只适用与report 风格的listctrl ）
	//dwStyle |= LVS_EX_CHECKBOXES;//item 前生成checkbox 控件
	m_ListInfo.SetExtendedStyle(dwStyle); // 设置扩展风格

	m_ListInfo.InsertColumn( 0, _T("Manufacturer"), LVCFMT_LEFT, 200 );
    m_ListInfo.InsertColumn( 1, _T("DeviceName"), LVCFMT_LEFT, 150 );
	m_ListInfo.InsertColumn( 2, _T("ProductID"), LVCFMT_LEFT, 150 );
    m_ListInfo.InsertColumn( 3, _T("VendorID"), LVCFMT_LEFT, 150 );

	LOGFONT   logfont;
	CFont   *pfont1   =   m_ListInfo.GetFont();
	pfont1->GetLogFont(   &logfont   );
	logfont.lfHeight   =logfont.lfHeight   *   1.5;   //这里可以修改字体的高比例
	logfont.lfWidth     =   logfont.lfWidth   *   1.5;   //这里可以修改字体的宽比例
	static   CFont   font1;
	font1.CreateFontIndirect(&logfont);
	m_ListInfo.SetFont(&font1);
	font1.Detach();
	SetWindowPos(NULL,0,0,665,700,SWP_NOMOVE);
	//::SetWindowLong(m_hWnd, GWL_STYLE, WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX);

	/*CRect rect;
	GetClientRect(&rect);
	old.x = rect.right - rect.left;
	old.y = rect.bottom - rect.top;
	GetSystemMetrics(SM_CXFULLSCREEN);
	GetSystemMetrics(SM_CYFULLSCREEN);
*/

	//CRect rect;
	//GetClientRect(&rect);     //取客户区大小
	//old.x=rect.right-rect.left;
	//old.y=rect.bottom-rect.top;
	//int cx = GetSystemMetrics(SM_CXFULLSCREEN);
	//int cy = GetSystemMetrics(SM_CYFULLSCREEN);
	//CRect rt;
	//SystemParametersInfo(SPI_GETWORKAREA,0,&rt,0);
	//cy = rt.bottom;
	//MoveWindow(0, 0, cx, cy);
	return TRUE;
}

void CUSBDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CUSBDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CUSBDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CUSBDlg::OnBnClickedButtonOpen()
{
	CString defaultDir = L"E:\\FileTest";
	CString fileName = L"";
	TCHAR filter[] = _T("文本文件(*.txt)||所有文件(*.*)||");
	CFileDialog openFileDlg(TRUE, defaultDir, fileName, OFN_HIDEREADONLY|OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\test.doc";
	INT_PTR result = openFileDlg.DoModal();
	if(result == IDOK) {
		filePath = openFileDlg.GetPathName();
		m_editPath.SetWindowTextW(filePath);
	}
}


void CUSBDlg::OnBnClickedButtonScan()
{
	m_ListInfo.DeleteAllItems();
	int i = 0;
	struct hid_device_info *devs, *cur_dev;
	CString str;

	devs = hid_enumerate(0x0, 0x0);
	cur_dev = devs;
	while (cur_dev) {
		TRACE("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
		TRACE("\n");
		TRACE("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
		TRACE("  Product:      %ls\n", cur_dev->product_string);
		TRACE("\n");
		int nRow = m_ListInfo.InsertItem(1,cur_dev->manufacturer_string);
		m_ListInfo.SetItemText(nRow,1,cur_dev->product_string);
		str.Format(_T("%d"),cur_dev->product_id);
		m_ListInfo.SetItemText(nRow,2,str);
		str.Format(_T("%d"),cur_dev->vendor_id);
		m_ListInfo.SetItemText(nRow,3,str);
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);
}

//CRC校验
unsigned short CUSBDlg::soft_crc16(unsigned char *buf, unsigned int len, unsigned short crc)
{
	unsigned int i, flag;
	while(len-- > 0)
	{
		crc ^= *buf++;
		i = 8;
		while(i-- > 0)
		{
			flag = crc & 0x01;
			crc >>= 1;
			if(flag)
			{
				crc ^= 0x1021;//0x8005,0x1021
			}
		}
	}
	return crc;
}
hid_device* CUSBDlg::openHIDDevice()
{
	POSITION pos=m_ListInfo.GetFirstSelectedItemPosition();
	if(pos == NULL)
	{
		AfxMessageBox(_T("请选中设备!"));
		return NULL;
	}
	hid_device *handle;
	int i = m_ListInfo.GetSelectionMark();
	CString strProdID = m_ListInfo.GetItemText(i,2);
	CString strVendorID = m_ListInfo.GetItemText(i,3);
	unsigned short iVendorID = _ttoi((LPCWSTR)strVendorID.GetBuffer());
	unsigned short iProID = _ttoi((LPCWSTR)strProdID.GetBuffer());
	int res = hid_init();
	handle = hid_open(iVendorID, iProID, NULL);
	if (handle == NULL)
	{
		AfxMessageBox(_T("HID设备打开失败!"));
		hid_exit();
		return NULL;
	}
	else
	{
		return handle;
	}
}


void CUSBDlg::updateFile(hid_device* handle,CString filePath,int cmd_file_type)
{
	if(filePath == "")
	{
		hid_close(handle);
		hid_exit();
		AfxMessageBox(_T("文件不可以为空"));
		return;
	}
	int res = hid_set_nonblocking(handle,0);
	int retryCount = 0;
	unsigned char hid_buf[BUF_LEN] = {0};
	unsigned char recv_buf[BUF_LEN] = {0};
	unsigned char *send_buffer  = (unsigned char *) malloc(BUF_LEN);
	unsigned char data_head[12] = {0};
	//第一步;发送HOU_START
HOU_START_POINT:
	hid_head = (struct hid_cmd_Head *)hid_buf;
	hid_head->report_id = HID_REPORT_ID;
	hid_head->cmd = HOU_START;
	hid_head->magic = MAGIC;
	hid_head->data_status = HID_DATA_INVALID;
	memcpy(data_head,&hid_buf[0],12);
	hid_head->crc = soft_crc16(data_head,12,0xFFFF);

	printfStructSend(hid_buf);
	res = hid_write(handle, (unsigned char*)hid_buf, BUF_LEN);
	res = hid_read_timeout(handle,(unsigned char*)recv_buf,BUF_LEN,1000);
	if (res == 0)
	{
		AfxMessageBox(_T("上位机收不到设备端的数据"));
		hid_close(handle);
		hid_exit();
		free(send_buffer);
		send_buffer = NULL;
		return;
	}
	m_response_hid_cmd_Head = (response_hid_cmd_Head *)recv_buf;
	if(m_response_hid_cmd_Head->cmd != HOU_START)
	{
		retryCount++;
		goto HOU_START_POINT;
	}
	if ((m_response_hid_cmd_Head ->response_status != CMD_EXCUTE_OK) && (retryCount < MAX_RETRY_COUNT)) {
		retryCount++;
		goto HOU_START_POINT;
	}
	else
	{
		if(retryCount == MAX_RETRY_COUNT)
		{
			AfxMessageBox(_T("发送开始准备升级命令失败!"));
			free(send_buffer);
			send_buffer = NULL;
			return;
		}
	}
	TRACE("report_id:%x,magic:%x,cmd:%x,data_status:%x,datalen:%x,response_status:%x,crc:%x\n",m_response_hid_cmd_Head->report_id,\
		m_response_hid_cmd_Head->magic,m_response_hid_cmd_Head->cmd,m_response_hid_cmd_Head->data_status,m_response_hid_cmd_Head->data_len,\
		m_response_hid_cmd_Head->response_status,m_response_hid_cmd_Head->crc);

	//第二步:告知下位发送什么类型的文件，比如：HOU_UPDATE_UVCCONFIG
	retryCount = 0;
HOU_UPDATE_UVCCONFIG_POTINT:

	memset(hid_buf,0,BUF_LEN);
	hid_head = NULL;
	hid_head = (struct hid_cmd_Head *)hid_buf;
	hid_head->report_id = HID_REPORT_ID;
	hid_head->cmd = cmd_file_type;
	hid_head->magic = MAGIC;
	hid_head->data_status = HID_DATA_INVALID;
	memcpy(data_head,hid_buf,12);
	hid_head->crc = soft_crc16(data_head,12,0xFFFF);
	printfStructSend(hid_buf);
	res = hid_write(handle, (unsigned char*)hid_buf, BUF_LEN);
	res = hid_read_timeout(handle,(unsigned char*)recv_buf,BUF_LEN,1000);
	m_response_hid_cmd_Head = (response_hid_cmd_Head *)recv_buf;
	if(m_response_hid_cmd_Head->cmd != cmd_file_type)
	{
		retryCount++;
		goto HOU_UPDATE_UVCCONFIG_POTINT;
	}
	if (m_response_hid_cmd_Head ->response_status != CMD_EXCUTE_OK && retryCount < MAX_RETRY_COUNT && m_response_hid_cmd_Head->cmd == cmd_file_type) {
		retryCount++;
		goto HOU_UPDATE_UVCCONFIG_POTINT;
	}
	else
	{
		if(retryCount == MAX_RETRY_COUNT)
		{
			AfxMessageBox(_T("告知下位机文件类型失败!"));
			hid_close(handle);
			res = hid_exit();
			free(send_buffer);
			send_buffer = NULL;
			return;
		}
	}
	TRACE("report_id:%x,magic:%x,cmd:%x,data_status:%x,datalen:%x,response_status:%x,crc:%x\n",m_response_hid_cmd_Head->report_id,\
		m_response_hid_cmd_Head->magic,m_response_hid_cmd_Head->cmd,m_response_hid_cmd_Head->data_status,m_response_hid_cmd_Head->data_len,\
		m_response_hid_cmd_Head->response_status,m_response_hid_cmd_Head->crc);
	//第三步:传输文件给下位机
	short rev = 1;	//一次实际读到的文件大小
	int len;	//传给下位的最大数据长度
	FILE *fw_fd;
	retryCount = 0;
	int size_Head = sizeof(hid_cmd_Head);
	memset(hid_buf,0,BUF_LEN);
	hid_head = NULL;
	len = BUF_LEN - size_Head;
	hid_head = (struct hid_cmd_Head *)hid_buf;
	hid_head->report_id = HID_REPORT_ID;
	hid_head->cmd = HOU_SEND_DATA;
	hid_head->magic = MAGIC;
	USES_CONVERSION;
	char* path = T2A(filePath.GetBuffer(0));
	filePath.ReleaseBuffer();

	if(fopen_s(&fw_fd,path,"rb") != 0)
	{
		AfxMessageBox(_T("打开文件失败!"));
	}
	long file_size;
	char *file_buffer = (char*)malloc(8 * 1024 * 1024);
	fseek(fw_fd, 0, SEEK_END);
	file_size = ftell(fw_fd);
	fseek(fw_fd, 0, SEEK_SET);
	long file_offset = 0;
	long read_size = file_size;
	while (rev) {
		rev = fread(file_buffer + file_offset, 1, read_size, fw_fd);
		read_size -= rev;
		file_offset += rev;

	}
	int buffer_offset = 0;
	int cnt = 0;
	int packNum = 0;

	rev = len;
	while (1)
	{
		if (buffer_offset <= file_size) {
			if ((file_size - buffer_offset) > len) {
				memcpy (hid_buf + size_Head, file_buffer + buffer_offset, len);
				rev = len;
				buffer_offset += len;
				hid_head->data_status = 1;

			} else if (((file_size - buffer_offset) <= len) && ((file_size - buffer_offset) > 0)){
				memcpy (hid_buf + size_Head, file_buffer + buffer_offset, file_size - buffer_offset);
				rev = file_size - buffer_offset;
				buffer_offset = file_size;
				hid_head->data_status = 2;
			} else if ((file_size - buffer_offset) == 0){
				break;
			}
		}

		Sleep(25);
		packNum++;
		retryCount = 0;
HOU_SEND_DATA_POINT:
		TRACE("packNUm:%d\n",packNum);
		hid_head->data_len = rev;
		TRACE("data_len:%d\n",rev);
		hid_head->pack_num = packNum;
		memcpy(data_head,&hid_buf[0],12);
		unsigned short headCRC = soft_crc16(&hid_buf[0],12,0xFFFF);
		unsigned short headrCRC2;
		headrCRC2 = soft_crc16(&hid_buf[0] + size_Head,rev, headCRC);
		TRACE("headCRC1_%x\n",headCRC);
		TRACE("headCRC2_%x\n",headrCRC2);

		hid_head->crc = headrCRC2;
		printfStructSend(hid_buf);
		memcpy(send_buffer, hid_buf, BUF_LEN);
		res = hid_write(handle, (unsigned char*)send_buffer, BUF_LEN);
		Sleep(5);
		res = hid_read_timeout(handle,(unsigned char*)recv_buf,BUF_LEN,1000);
		m_response_hid_cmd_Head = (response_hid_cmd_Head *)recv_buf;
		TRACE("respon:%x\n",m_response_hid_cmd_Head->response_status);
		TRACE("report_id:%x,magic:%x,cmd:%x,data_status:%x,datalen:%x,response_status:%x,crc:%x\n",m_response_hid_cmd_Head->report_id,\
			m_response_hid_cmd_Head->magic,m_response_hid_cmd_Head->cmd,m_response_hid_cmd_Head->data_status,m_response_hid_cmd_Head->data_len,\
			m_response_hid_cmd_Head->response_status,m_response_hid_cmd_Head->crc);
		//校验下位机返回的结构体信息
		//unsigned short respseCRC = soft_crc16(recv_buf,12,0xFFFF);
		if(m_response_hid_cmd_Head->cmd != HOU_SEND_DATA)
		{
			TRACE("retryCount:%d\n",retryCount);
			retryCount++;
			Sleep(20);
			goto HOU_SEND_DATA_POINT;
		}
		if ( m_response_hid_cmd_Head->response_status != CMD_EXCUTE_OK && retryCount < MAX_RETRY_COUNT && m_response_hid_cmd_Head->cmd == HOU_SEND_DATA)
		{
			TRACE("retryCount:%d\n",retryCount);
			retryCount++;
			Sleep(20);
			goto HOU_SEND_DATA_POINT;
		}
		else
		{
			if(retryCount == MAX_RETRY_COUNT)
			{
				AfxMessageBox(_T("发送升级包失败!"));
				hid_close(handle);
				res = hid_exit();
				/*free(send_buffer);
				send_buffer = NULL;
				free(file_buffer);
				file_buffer = NULL;*/
				return;
			}
		}
	}
	retryCount = 0;
	Sleep(1000);
	//第四步:获取升级结果
HOU_GET_RESULT_POINT:
	memset(hid_buf,0,BUF_LEN);
	hid_head = NULL;
	hid_head = (struct hid_cmd_Head *)hid_buf;
	hid_head->report_id = HID_REPORT_ID;
	hid_head->cmd = HOU_GET_RESULT;
	hid_head->magic = MAGIC;
	hid_head->data_status = HID_DATA_INVALID;
	memcpy(data_head,&hid_buf[0],12);
	hid_head->crc = soft_crc16(data_head,12,0xFFFF);
	printfStructSend(hid_buf);
	res = hid_write(handle, (unsigned char*)hid_buf, BUF_LEN);
HOU_GET_RESULT_POINT_READ:
	//memset(recv_buf,0,1024);
	res = hid_read_timeout(handle,(unsigned char*)recv_buf,BUF_LEN,1000);
	m_response_hid_cmd_Head = (response_hid_cmd_Head *)recv_buf;
	TRACE("report_id:%x,magic:%x,cmd:%x,data_status:%x,datalen:%x,response_status:%x,crc:%x\n",m_response_hid_cmd_Head->report_id,\
		m_response_hid_cmd_Head->magic,m_response_hid_cmd_Head->cmd,m_response_hid_cmd_Head->data_status,m_response_hid_cmd_Head->data_len,\
		m_response_hid_cmd_Head->response_status,m_response_hid_cmd_Head->crc);

	if (m_response_hid_cmd_Head->cmd != HOU_GET_RESULT)
	{
		retryCount++;
		goto HOU_GET_RESULT_POINT;
	}
	if (m_response_hid_cmd_Head ->response_status != CMD_EXCUTE_OK && retryCount < MAX_RETRY_COUNT && m_response_hid_cmd_Head->cmd == HOU_GET_RESULT) {
		retryCount++;
		goto HOU_GET_RESULT_POINT;
	}
	else
	{
		if(retryCount == MAX_RETRY_COUNT)
		{
			AfxMessageBox(_T("发送获取升级结果失败!"));
			hid_close(handle);
			res = hid_exit();
			/*free(send_buffer);
			send_buffer = NULL;
			free(file_buffer);
			file_buffer = NULL;*/
			return;
		}
	}
	if(m_response_hid_cmd_Head->response_status == CMD_EXCUTE_OK)
	{
		AfxMessageBox(_T("烧录成功!"));
	}
	/*free(file_buffer);
	file_buffer = NULL;
	free(send_buffer);
	send_buffer = NULL;*/
	hid_close(handle);
	res = hid_exit();
}

void CUSBDlg::OnBnClickedButtonBurn()
{
	m_editPath.GetWindowTextW(filePath);
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	updateFile(handle,filePath,HOU_UPDATE_UVCCONFIG);
}


void CUSBDlg::OnBnClickedButtonVid()
{
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	CString str;
	m_editVID.GetWindowText(str);
	if (str == "")
	{
		AfxMessageBox(_T("VID不可以为空"));
		hid_close(handle);
		hid_exit();
		return;
	}
	int res = WriteInfoToDevice(handle,str,HID_BASIC_INFO_VID);
	if(res == 0)
	{
		AfxMessageBox(_T("写入VID成功!"));
	}
	else
	{
		AfxMessageBox(_T("写入VID失败!"));
	}
}

void CUSBDlg::printfStructSend(unsigned char* buf)
{
	TRACE("host_send:	report_id:%x,magic:%x%x,cmd:%x%x,data_status:%x,data_len:%x%x,pack_num:%x%x%x%x,crc:%x%x\n",buf[0],buf[2],buf[1],buf[4],buf[3],\
		buf[5],buf[7],buf[6],buf[11],buf[10],buf[9],buf[8],buf[13],buf[12]);
}

void CUSBDlg::OnNMCustomdrawListInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	*pResult = 0;
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );
	*pResult = CDRF_DODEFAULT;
	if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage )
	{
		COLORREF clrNewTextColor, clrNewBkColor;
		int    nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );
		BOOL bSelect = FALSE;
		POSITION pos =m_ListInfo.GetFirstSelectedItemPosition();
		while(pos)
		{
			int index = m_ListInfo.GetNextSelectedItem(pos);
			if(index==nItem)
			{
				bSelect = TRUE;
				break;
			}
		}
		if(bSelect)
		{
			clrNewTextColor = RGB(0,255,0);     //Set the text to red
			clrNewBkColor = RGB(0,0,255);     //Set the bkgrnd color to blue
		}
		else
		{
			clrNewTextColor = RGB(0,0,0);     //Leave the text black
			clrNewBkColor = RGB(255,255,255);    //leave the bkgrnd color white
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;
		*pResult = CDRF_DODEFAULT;

	}
}

void CUSBDlg::OnBnClickedButtonOpenUvcapp()
{
	CString defaultDir = L"E:\\FileTest";
	CString fileName = L"";
	TCHAR filter[] = _T("文本文件(*.txt)||所有文件(*.*)||");
	CFileDialog openFileDlg(TRUE, defaultDir, fileName, OFN_HIDEREADONLY|OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\test.doc";
	INT_PTR result = openFileDlg.DoModal();
	if(result == IDOK) {
		filePath_UvcApp = openFileDlg.GetPathName();
		m_editUVCAPP.SetWindowTextW(filePath_UvcApp);
	}
}


void CUSBDlg::OnBnClickedButtonBurnUvcapp()
{
	m_editUVCAPP.GetWindowTextW(filePath_UvcApp);
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	updateFile(handle,filePath_UvcApp,HOU_UPDATE_UVCAPP);
}


void CUSBDlg::OnBnClickedButtonOpenSensorDriver()
{
	CString defaultDir = L"E:\\FileTest";
	CString fileName = L"";
	TCHAR filter[] = _T("文本文件(*.txt)||所有文件(*.*)||");
	CFileDialog openFileDlg(TRUE, defaultDir, fileName, OFN_HIDEREADONLY|OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\test.doc";
	INT_PTR result = openFileDlg.DoModal();
	if(result == IDOK) {
		filePath_Sensor_Driver = openFileDlg.GetPathName();
		m_editSensor_driver.SetWindowTextW(filePath_Sensor_Driver);
	}
}


void CUSBDlg::OnBnClickedButtonBurnSensorDriver()
{
	m_editSensor_driver.GetWindowTextW(filePath_Sensor_Driver);
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	updateFile(handle,filePath_Sensor_Driver,HOU_UPDATE_SENSOR_DIRVER);
}


void CUSBDlg::OnBnClickedButtonOpenSensorSettings()
{
	CString defaultDir = L"E:\\FileTest";
	CString fileName = L"";
	TCHAR filter[] = _T("文本文件(*.txt)||所有文件(*.*)||");
	CFileDialog openFileDlg(TRUE, defaultDir, fileName, OFN_HIDEREADONLY|OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\test.doc";
	INT_PTR result = openFileDlg.DoModal();
	if(result == IDOK) {
		filePath_Sensor_Settings = openFileDlg.GetPathName();
		m_editSensor_setting.SetWindowTextW(filePath_Sensor_Settings);
	}
}


void CUSBDlg::OnBnClickedButtonBurnSensorSettings()
{
	m_editSensor_setting.GetWindowTextW(filePath_Sensor_Settings);
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	updateFile(handle,filePath_Sensor_Settings,HOU_UPDATE_SENSOR_SETTING);
}


void CUSBDlg::OnBnClickedButtonOpenUvcattr()
{
	CString defaultDir = L"E:\\FileTest";
	CString fileName = L"";
	TCHAR filter[] = _T("文本文件(*.txt)||所有文件(*.*)||");
	CFileDialog openFileDlg(TRUE, defaultDir, fileName, OFN_HIDEREADONLY|OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\test.doc";
	INT_PTR result = openFileDlg.DoModal();
	if(result == IDOK) {
		filePath_UvcAttr = openFileDlg.GetPathName();
		m_editUVCATTR.SetWindowTextW(filePath_UvcAttr);
	}
}

void CUSBDlg::OnBnClickedButtonBurnUvcattr()
{
	m_editUVCATTR.GetWindowTextW(filePath_UvcAttr);
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	updateFile(handle,filePath_UvcAttr,HOU_UPDATE_UVCATTR);
}

void CUSBDlg::OnBnClickedButtonSend()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	CString str1;
	GetDlgItem(IDC_SENDTEXT)->GetWindowText(str);  //获取用户发送信息的字符串
	if (str == "")
	{
		GetDlgItem(IDC_TEXT)->GetWindowText(str1);
		str1 += "\r\n";
		str1 += _T("消息不能为空");
		GetDlgItem(IDC_TEXT)->SetWindowText(str1);
	}
	else
	{
		USES_CONVERSION;
		std::string s(W2A(str));
		const char* cstr = s.c_str();
		::send(socket, cstr, str.GetLength(), 0);  //发送消息到指定服务器
		GetDlgItem(IDC_TEXT)->GetWindowText(str1);
		str1 += "\r\n";
		str1 += str;
		GetDlgItem(IDC_TEXT)->SetWindowText(str1);
	}
}


void CUSBDlg::OnBnClickedButtonConnect()
{
	// TODO: 在此添加控件通知处理程序代码
	//定义字符串变量
	CString str;
	CString str1;
	int port;  //定义端口号
	GetDlgItem(IDC_ADDR)->GetWindowText(str);  //获取服务器地址
	GetDlgItem(IDC_PORT)->GetWindowText(str1);  //获取端口号
	if (str == "" || str1 == "")
	{
		MessageBox(_T("服务器地址或端口不能为空"));
	}
	else
	{
		USES_CONVERSION;
		std::string s(W2A(str1));
		const char* cstr = s.c_str();

		port = atoi(cstr); //将字符串转换为数字
		addr.sin_family = AF_INET;

		std::string s2(W2A(str));
		const char* cstr2 = s2.c_str();
		addr.sin_addr.S_un.S_addr = inet_addr(cstr2);  //转换服务器IP地址
		addr.sin_port = ntohs(port);
		GetDlgItem(IDC_TEXT)->SetWindowText(_T("正在连接服务器\r\n"));

		if (::connect(socket, (sockaddr*)&addr, sizeof(addr)))
		{
			GetDlgItem(IDC_TEXT)->GetWindowText(str);
			str += "服务器连接成功\r\n";
			GetDlgItem(IDC_TEXT)->SetWindowText(str);
			GetDlgItem(IDC_SENDTEXT)->EnableWindow(true);
			//设置控件的状态
			GetDlgItem(IDC_BUTTON_SEND)->EnableWindow(true);
			GetDlgItem(IDC_ADDR)->EnableWindow(false);
			GetDlgItem(IDC_PORT)->EnableWindow(false);
		}
		else
		{
			GetDlgItem(IDC_TEXT)->GetWindowText(str);
			str += "连接服务器失败!请重试\r\n";
			GetDlgItem(IDC_TEXT)->SetWindowText(str);
		}
	}
}


LRESULT CUSBDlg::OnSocket(WPARAM wParam, LPARAM lParam)
{
	char cs[100] = {0};  //定义缓冲区
	if (lParam == FD_READ)  //如果是套子接读取时间
	{
		CString num = _T("");
		recv(socket, cs, 100, NULL);  //接收数据
		GetDlgItem(IDC_TEXT)->GetWindowText(num);
		num += _T("\r\n服务器说:");
		num += (LPTSTR)cs;
		GetDlgItem(IDC_TEXT)->SetWindowText(num);
	}
	return true;

}

void CUSBDlg::OnBnClickedButtonSn()
{
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	CString str;
	m_Edit_SN.GetWindowText(str);
	if (str == "")
	{
		AfxMessageBox(_T("SN不可以为空"));
		hid_close(handle);
		hid_exit();
		return;
	}
	int res = WriteInfoToDevice(handle,str,HID_BASIC_INFO_SN);
	if(res == 0)
	{
		AfxMessageBox(_T("写入SN成功!"));
	}
	else
	{
		AfxMessageBox(_T("写入SN失败!"));
	}
}

int CUSBDlg::WriteInfoToDevice(hid_device* handle,CString info,int cmd)
{
	//头结构体14个字节，最后2个字节是CRC的值
	int snLength = info.GetLength();
	int res = -1;
	int nBytes = WideCharToMultiByte(CP_ACP,0,info,snLength,NULL,0,NULL,NULL);
	char* c_SN = new char[ nBytes + 1];
	memset(c_SN,0,snLength + 1);
	WideCharToMultiByte(CP_OEMCP, 0, info, snLength, c_SN, nBytes, NULL, NULL);
	c_SN[nBytes] = 0;
	unsigned char hid_buf[BUF_LEN] = {0};
	unsigned char recv_buf[BUF_LEN] = {0};
	unsigned char data_head[12] = {0};
	hid_head = (struct hid_cmd_Head *)hid_buf;
	hid_head->report_id = HID_REPORT_ID;
	hid_head->cmd = cmd;
	hid_head->magic = MAGIC;
	hid_head->data_status = HID_DATA_END;
	hid_head->data_len = snLength;
	memcpy(data_head,&hid_buf[0],12);
	unsigned short head_crc = soft_crc16(data_head,12,0xFFFF);
	unsigned short crc = soft_crc16((unsigned char*)c_SN,nBytes,head_crc);
	hid_head->crc = crc;
	memcpy(&hid_buf[14],c_SN,nBytes);
	printfStructSend(hid_buf);
	res = hid_write(handle, (unsigned char*)hid_buf, BUF_LEN);
	res = hid_read_timeout(handle,(unsigned char*)recv_buf,BUF_LEN,1000);
	if (res == 0)
	{
		hid_close(handle);
		hid_exit();
		return -1;
	}
	m_response_hid_cmd_Head = (response_hid_cmd_Head *)recv_buf;
	hid_close(handle);
	hid_exit();
	if (m_response_hid_cmd_Head->response_status == CMD_EXCUTE_OK)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

void CUSBDlg::OnBnClickedButtonOpenUvcapp2()
{
	CString defaultDir = L"E:\\FileTest";
	CString fileName = L"";
	TCHAR filter[] = _T("文本文件(*.txt)||所有文件(*.*)||");
	CFileDialog openFileDlg(TRUE, defaultDir, fileName, OFN_HIDEREADONLY|OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\test.doc";
	INT_PTR result = openFileDlg.DoModal();
	if(result == IDOK) {
		filePath_UvcApp2 = openFileDlg.GetPathName();
		m_editUVCAPP2.SetWindowTextW(filePath_UvcApp2);
	}
}


void CUSBDlg::OnBnClickedButtonPid()
{
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	CString str;
	m_editPID.GetWindowText(str);
	if (str == "")
	{
		AfxMessageBox(_T("PID不可以为空"));
		hid_close(handle);
		hid_exit();
		return;
	}
	int res = WriteInfoToDevice(handle,str,HID_BASIC_INFO_PID);
	if(res == 0)
	{
		AfxMessageBox(_T("写入PID成功!"));
	}
	else
	{
		AfxMessageBox(_T("写入PID失败!"));
	}
}


void CUSBDlg::OnBnClickedButtonManufacture()
{
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	CString str;
	m_editManufacture.GetWindowText(str);
	if (str == "")
	{
		AfxMessageBox(_T("Manufacture不可以为空"));
		hid_close(handle);
		hid_exit();
		return;
	}
	int res = WriteInfoToDevice(handle,str,HID_BASIC_INFO_MANUFACTURE);
	if(res == 0)
	{
		AfxMessageBox(_T("写入Manufacture成功!"));
	}
	else
	{
		AfxMessageBox(_T("写入Manufacture失败!"));
	}
}


void CUSBDlg::OnBnClickedButtonBcd()
{
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	CString str;
	m_editBCD.GetWindowText(str);
	if (str == "")
	{
		AfxMessageBox(_T("BCD不可以为空"));
		hid_close(handle);
		hid_exit();
		return;
	}
	int res = WriteInfoToDevice(handle,str,HID_BASIC_INFO_BCD);
	if(res == 0)
	{
		AfxMessageBox(_T("写入BCD成功!"));
	}
	else
	{
		AfxMessageBox(_T("写入BCD失败!"));
	}
}


void CUSBDlg::OnBnClickedButtonBurnUvcapp2()
{
	m_editUVCAPP2.GetWindowTextW(filePath_UvcApp2);
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	updateFile(handle,filePath_UvcApp2,HOU_UPDATE_UVCAPP_ANTICOPY);
}


void CUSBDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	//ReSize();
	return;
}

void CUSBDlg::ReSize(void)
{
	float fsp[2];
	POINT Newp; //获取现在对话框的大小
	CRect recta;
	GetClientRect(&recta);     //取客户区大小
	Newp.x=recta.right-recta.left;
	Newp.y=recta.bottom-recta.top;
	fsp[0]=(float)Newp.x/old.x;
	fsp[1]=(float)Newp.y/old.y;
	CRect Rect;
	int woc;
	CPoint OldTLPoint,TLPoint; //左上角
	CPoint OldBRPoint,BRPoint; //右下角
	HWND  hwndChild=::GetWindow(m_hWnd,GW_CHILD);  //列出所有控件
	while(hwndChild)
	{
		woc=::GetDlgCtrlID(hwndChild);//取得ID
		GetDlgItem(woc)->GetWindowRect(Rect);
		ScreenToClient(Rect);
		OldTLPoint = Rect.TopLeft();
		TLPoint.x = long(OldTLPoint.x*fsp[0]);
		TLPoint.y = long(OldTLPoint.y*fsp[1]);
		OldBRPoint = Rect.BottomRight();
		BRPoint.x = long(OldBRPoint.x *fsp[0]);
		BRPoint.y = long(OldBRPoint.y *fsp[1]);
		Rect.SetRect(TLPoint,BRPoint);
		GetDlgItem(woc)->MoveWindow(Rect,TRUE);
		hwndChild=::GetWindow(hwndChild, GW_HWNDNEXT);
	}
	old=Newp;
}
