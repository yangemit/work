
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
#define MAX_PATH	1024
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
	DDX_Control(pDX, IDC_LIST_INFO, m_ListInfo);
	DDX_Control(pDX, IDC_EDIT_SN, m_Edit_SN);
	DDX_Control(pDX, IDC_EDIT_PID, m_editPID);
	DDX_Control(pDX, IDC_EDIT_VID, m_editVID);
	DDX_Control(pDX, IDC_EDIT_BCD, m_editBCD);
	DDX_Control(pDX, IDC_EDIT_MANUFACTURE, m_editManufacture);
	DDX_Control(pDX, IDC_EDIT_MODEL, m_editWrite_Model);
	DDX_Control(pDX, IDC_EDIT_WRITE_CMEI, m_editWrite_Cmei);
	DDX_Control(pDX, IDC_EDIT_READ_SN, m_editRead_SN);
	DDX_Control(pDX, IDC_EDIT_READ_PID, m_editRead_PID);
	DDX_Control(pDX, IDC_EDIT_READ_VID, m_editRead_VID);
	DDX_Control(pDX, IDC_EDIT_READ_CMEI, m_editRead_CMEI);
	DDX_Control(pDX, IDC_EDIT_READ_MANUFACTURE, m_editRead_MANUFACTURER);
	DDX_Control(pDX, IDC_EDIT_READ_BCD, m_editRead_BCD);
	DDX_Control(pDX, IDC_EDIT_READ_MODEL, m_editRead_Model);
	DDX_Control(pDX, IDC_EDIT_PATH_PACKET, m_edit_path_packet);
	DDX_Control(pDX, IDC_TEXT_SN, m_editAlogirmSN);
	DDX_Control(pDX, IDC_PROGRESS, m_ProgressCtrl);
	DDX_Control(pDX, IDC_STATIC_UPDATE_INFO, m_Static_Update_Info);
	DDX_Control(pDX, IDC_EDIT_READ_VERSION, m_editReadVersion);
	DDX_Control(pDX, IDC_EDIT_WRITE_APPKEY, m_editWrite_AppKey);
	DDX_Control(pDX, IDC_EDIT_READ_APPKEY, m_editReadAppKey);
}

BEGIN_MESSAGE_MAP(CUSBDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SCAN, &CUSBDlg::OnBnClickedButtonScan)
	ON_BN_CLICKED(IDC_BUTTON_SN, &CUSBDlg::OnBnClickedButtonSn)
	ON_BN_CLICKED(IDC_BUTTON_VID, &CUSBDlg::OnBnClickedButtonVid)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_INFO, &CUSBDlg::OnNMCustomdrawListInfo)
	ON_BN_CLICKED(IDC_BUTTON_PID, &CUSBDlg::OnBnClickedButtonPid)
	ON_BN_CLICKED(IDC_BUTTON_MANUFACTURE, &CUSBDlg::OnBnClickedButtonManufacture)
	ON_BN_CLICKED(IDC_BUTTON_BCD, &CUSBDlg::OnBnClickedButtonBcd)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_WRITE_CMEI, &CUSBDlg::OnBnClickedButtonWriteCmei)
	ON_BN_CLICKED(IDC_BUTTON_MODEL, &CUSBDlg::OnBnClickedButtonModel)
	ON_BN_CLICKED(IDC_BUTTON_READ_SN, &CUSBDlg::OnBnClickedButtonReadSn)
	ON_BN_CLICKED(IDC_BUTTON_READ_MANUFACTURE, &CUSBDlg::OnBnClickedButtonReadManufacture)
	ON_BN_CLICKED(IDC_BUTTON_READ_PID, &CUSBDlg::OnBnClickedButtonReadPid)
	ON_BN_CLICKED(IDC_BUTTON_READ_BCD, &CUSBDlg::OnBnClickedButtonReadBcd)
	ON_BN_CLICKED(IDC_BUTTON_READ_VID, &CUSBDlg::OnBnClickedButtonReadVid)
	ON_BN_CLICKED(IDC_BUTTON_READ_MODEL, &CUSBDlg::OnBnClickedButtonReadModel)
	ON_BN_CLICKED(IDC_BUTTON_READ_CMEI, &CUSBDlg::OnBnClickedButtonReadCmei)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_PACKET, &CUSBDlg::OnBnClickedButtonOpenPacket)
	ON_BN_CLICKED(IDC_BUTTON_OPENPACKET, &CUSBDlg::OnBnClickedButtonOpenpacket)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE_PACKET, &CUSBDlg::OnBnClickedButtonUpdatePacket)
	ON_BN_CLICKED(IDC_BUTTON_GET_SERVER_INFO, &CUSBDlg::OnBnClickedButtonGetServerInfo)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_READ_VERSION, &CUSBDlg::OnBnClickedButtonReadVersion)
	ON_BN_CLICKED(IDC_BUTTON_READ_APPKEY, &CUSBDlg::OnBnClickedButtonReadAppkey)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_APPKEY, &CUSBDlg::OnBnClickedButtonWriteAppkey)
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
	dwStyle |= LVS_EX_FULLROWSELECT;// 选中某行使整行高亮（只适用与report 风格的listctrl
	dwStyle |= LVS_EX_GRIDLINES;// 网格线（只适用与report 风格的listctrl)
	//dwStyle |= LVS_EX_CHECKBOXES;//item 前生成checkbox 控件
	m_ListInfo.SetExtendedStyle(dwStyle); // 设置扩展风格
	m_ListInfo.InsertColumn( 0, _T("Manufacturer"), LVCFMT_LEFT, 200 );
	m_ListInfo.InsertColumn( 1, _T("DeviceName"), LVCFMT_LEFT, 150 );
	m_ListInfo.InsertColumn( 2, _T("ProductID"), LVCFMT_LEFT, 150 );
	m_ListInfo.InsertColumn( 3, _T("VendorID"), LVCFMT_LEFT, 150 );

	LOGFONT   logfont;
	CFont   *pfont1   =   m_ListInfo.GetFont();
	pfont1->GetLogFont(   &logfont   );
//	logfont.lfHeight   =logfont.lfHeight   *   1.5;   //这里可以修改字体的高比例
//	logfont.lfWidth     =   logfont.lfWidth   *   1.5;   //这里可以修改字体的宽比例
	static   CFont   font1;
	font1.CreateFontIndirect(&logfont);
	m_ListInfo.SetFont(&font1);
	font1.Detach();
	//SetWindowPos(NULL,0,0,665,830,SWP_NOMOVE);//Hik
	//SetWindowPos(NULL,0,0,665,395,SWP_NOMOVE);//Ares
	m_ProgressCtrl.SetRange(0,100);
	m_ProgressCtrl.SetPos(0);

#ifdef CONFIG_PRODUCT_ARES
	(CStatic*)GetDlgItem(IDC_STATIC_TIP2)->ShowWindow(SW_HIDE);
	(CButton*)GetDlgItem(IDC_BUTTON_READ_SN)->ShowWindow(SW_HIDE);
	(CButton*)GetDlgItem(IDC_BUTTON_READ_PID)->ShowWindow(SW_HIDE);
	(CButton*)GetDlgItem(IDC_BUTTON_READ_VID)->ShowWindow(SW_HIDE);
	(CButton*)GetDlgItem(IDC_BUTTON_READ_CMEI)->ShowWindow(SW_HIDE);
	(CButton*)GetDlgItem(IDC_BUTTON_READ_MANUFACTURE)->ShowWindow(SW_HIDE);
	(CButton*)GetDlgItem(IDC_BUTTON_READ_BCD)->ShowWindow(SW_HIDE);
	(CButton*)GetDlgItem(IDC_BUTTON_READ_MODEL)->ShowWindow(SW_HIDE);

	(CButton*)GetDlgItem(IDC_BUTTON_SN)->ShowWindow(SW_HIDE);
	(CButton*)GetDlgItem(IDC_BUTTON_PID)->ShowWindow(SW_HIDE);
	(CButton*)GetDlgItem(IDC_BUTTON_VID)->ShowWindow(SW_HIDE);
	(CButton*)GetDlgItem(IDC_BUTTON_WRITE_CMEI)->ShowWindow(SW_HIDE);
	(CButton*)GetDlgItem(IDC_BUTTON_MANUFACTURE)->ShowWindow(SW_HIDE);
	(CButton*)GetDlgItem(IDC_BUTTON_BCD)->ShowWindow(SW_HIDE);
	(CButton*)GetDlgItem(IDC_BUTTON_MODEL)->ShowWindow(SW_HIDE);


	(CEdit*)GetDlgItem(IDC_EDIT_READ_SN)->ShowWindow(SW_HIDE);
	(CEdit*)GetDlgItem(IDC_EDIT_READ_PID)->ShowWindow(SW_HIDE);
	(CEdit*)GetDlgItem(IDC_EDIT_READ_VID)->ShowWindow(SW_HIDE);
	(CEdit*)GetDlgItem(IDC_EDIT_READ_CMEI)->ShowWindow(SW_HIDE);
	(CEdit*)GetDlgItem(IDC_EDIT_READ_MANUFACTURE)->ShowWindow(SW_HIDE);
	(CEdit*)GetDlgItem(IDC_EDIT_READ_BCD)->ShowWindow(SW_HIDE);
	(CEdit*)GetDlgItem(IDC_EDIT_READ_MODEL)->ShowWindow(SW_HIDE);

	(CEdit*)GetDlgItem(IDC_EDIT_SN)->ShowWindow(SW_HIDE);
	(CEdit*)GetDlgItem(IDC_EDIT_PID)->ShowWindow(SW_HIDE);
	(CEdit*)GetDlgItem(IDC_EDIT_VID)->ShowWindow(SW_HIDE);
	(CEdit*)GetDlgItem(IDC_EDIT_WRITE_CMEI)->ShowWindow(SW_HIDE);
	(CEdit*)GetDlgItem(IDC_EDIT_MANUFACTURE)->ShowWindow(SW_HIDE);
	(CEdit*)GetDlgItem(IDC_EDIT_BCD)->ShowWindow(SW_HIDE);
	(CEdit*)GetDlgItem(IDC_EDIT_MODEL)->ShowWindow(SW_HIDE);

#endif


#ifndef CONFIG_PRODUCT_ARES
	AdjustWindowsSize();
	ReSize();
#else
	SetWindowPos(NULL,0,0,665,485,SWP_NOMOVE);
#endif
	OnBnClickedButtonScan();
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


bool CUSBDlg::updateFileSuccess(hid_device* handle,CString filePath,int cmd_file_type)
{
	int res = hid_set_nonblocking(handle,0);
	int retryCount = 0;
	unsigned char hid_buf[BUF_LEN] = {0};
	unsigned char recv_buf[BUF_LEN] = {0};
	unsigned char data_head[12] = {0};
	hid_cmd_Head *hid_head = NULL;
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
		return false;
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
			return false;
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
			return false;
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

		Sleep(10);
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
		res = hid_write(handle, (unsigned char*)hid_buf, BUF_LEN);
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
				free(file_buffer);
				file_buffer = NULL;
				return false;
			}
		}
	}
	retryCount = 0;
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
	res = hid_read_timeout(handle,(unsigned char*)recv_buf,BUF_LEN,3000);
	TRACE("readlength:%d\n",res);
	memset(&m_response_hid_cmd_Head,0,sizeof(response_hid_cmd_Head));
	m_response_hid_cmd_Head = (response_hid_cmd_Head *)recv_buf;
	TRACE("report_id:%x,magic:%x,cmd:%x,data_status:%x,datalen:%x,response_status:%x,crc:%x\n",m_response_hid_cmd_Head->report_id,\
		m_response_hid_cmd_Head->magic,m_response_hid_cmd_Head->cmd,m_response_hid_cmd_Head->data_status,m_response_hid_cmd_Head->data_len,\
		m_response_hid_cmd_Head->response_status,m_response_hid_cmd_Head->crc);

	/*if (m_response_hid_cmd_Head->cmd != HOU_GET_RESULT)
	{
	retryCount++;
	goto HOU_GET_RESULT_POINT;
	}*/
	if ((m_response_hid_cmd_Head ->response_status != CMD_EXCUTE_OK && retryCount < MAX_RETRY_COUNT && m_response_hid_cmd_Head->cmd == HOU_GET_RESULT) || m_response_hid_cmd_Head->cmd != HOU_GET_RESULT) {
		retryCount++;
		goto HOU_GET_RESULT_POINT;
	}
	else
	{
		if(retryCount == MAX_RETRY_COUNT)
		{
			AfxMessageBox(_T("发送获取升级结果失败!"));
			free(file_buffer);
			file_buffer = NULL;
			return false;
		}
	}
	if(m_response_hid_cmd_Head->response_status == CMD_EXCUTE_OK)
	{
		//AfxMessageBox(_T("烧录成功!"));
	}
	free(file_buffer);
	file_buffer = NULL;
	return true;
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
	hid_cmd_Head *hid_head = NULL;
	int res = hid_set_nonblocking(handle,0);
	int retryCount = 0;
	unsigned char hid_buf[BUF_LEN] = {0};
	unsigned char recv_buf[BUF_LEN] = {0};
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

		Sleep(10);
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
		res = hid_write(handle, (unsigned char*)hid_buf, BUF_LEN);
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
				free(file_buffer);
				file_buffer = NULL;
				return;
			}
		}
	}
	retryCount = 0;
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
	res = hid_read_timeout(handle,(unsigned char*)recv_buf,BUF_LEN,3000);
	TRACE("readlength:%d\n",res);
	memset(&m_response_hid_cmd_Head,0,sizeof(response_hid_cmd_Head));
	m_response_hid_cmd_Head = (response_hid_cmd_Head *)recv_buf;
	TRACE("report_id:%x,magic:%x,cmd:%x,data_status:%x,datalen:%x,response_status:%x,crc:%x\n",m_response_hid_cmd_Head->report_id,\
		m_response_hid_cmd_Head->magic,m_response_hid_cmd_Head->cmd,m_response_hid_cmd_Head->data_status,m_response_hid_cmd_Head->data_len,\
		m_response_hid_cmd_Head->response_status,m_response_hid_cmd_Head->crc);

	/*if (m_response_hid_cmd_Head->cmd != HOU_GET_RESULT)
	{
	retryCount++;
	goto HOU_GET_RESULT_POINT;
	}*/
	if ((m_response_hid_cmd_Head ->response_status != CMD_EXCUTE_OK && retryCount < MAX_RETRY_COUNT && m_response_hid_cmd_Head->cmd == HOU_GET_RESULT) || m_response_hid_cmd_Head->cmd != HOU_GET_RESULT) {
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
			free(file_buffer);
			file_buffer = NULL;
			return;
		}
	}
	if(m_response_hid_cmd_Head->response_status == CMD_EXCUTE_OK)
	{
		AfxMessageBox(_T("烧录成功!"));
	}
	free(file_buffer);
	file_buffer = NULL;
	hid_close(handle);
	res = hid_exit();
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
	hid_cmd_Head *hid_head = NULL;
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

void CUSBDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
#ifndef CONFIG_PRODUCT_ARES
	ReSize();
#endif
	return;
}
void CUSBDlg::AdjustWindowsSize(void)
{
#if 0
       int cx = GetSystemMetrics(SM_CXFULLSCREEN);
	int cy = GetSystemMetrics(SM_CYFULLSCREEN);
	CRect rt;
	SystemParametersInfo(SPI_GETWORKAREA,0,&rt,0);  //获得桌面工作区大小，即不包括任务栏的大小
	cy = rt.bottom;
#endif
	CRect rect;
	GetClientRect(&rect);     //取客户区大小
	old.x=rect.right-rect.left;
	old.y=rect.bottom-rect.top;

	MoveWindow(0, 0, old.x, old.y);
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
              if (GetWindowLong(hwndChild, GWL_STYLE) & WS_VISIBLE) {
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
		}
		  hwndChild=::GetWindow(hwndChild, GW_HWNDNEXT);
	}
	old=Newp;
}

void CUSBDlg::OnBnClickedButtonWriteCmei()
{
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	CString str;
	m_editWrite_Cmei.GetWindowText(str);
	if (str == "")
	{
		AfxMessageBox(_T("CMEI不可以为空"));
		hid_close(handle);
		hid_exit();
		return;
	}
	int res = WriteInfoToDevice(handle,str,HID_BASIC_INFO_CMEI);
	if(res == 0)
	{
		AfxMessageBox(_T("写入CMEI成功!"));
	}
	else
	{
		AfxMessageBox(_T("写入CMEI失败!"));
	}
}


void CUSBDlg::OnBnClickedButtonModel()
{
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	CString str;
	m_editWrite_Model.GetWindowText(str);
	if (str == "")
	{
		AfxMessageBox(_T("Model不可以为空"));
		hid_close(handle);
		hid_exit();
		return;
	}
	int res = WriteInfoToDevice(handle,str,HID_BASIC_INFO_MODEL);
	if(res == 0)
	{
		AfxMessageBox(_T("写入Model成功!"));
	}
	else
	{
		AfxMessageBox(_T("写入Model失败!"));
	}
}

CString CUSBDlg::ReadInfoFromDevice(hid_device* handle,int cmd)
{
	//头结构体14个字节，最后2个字节是CRC的值
	CString strReturnInfo = _T("");
	unsigned char hid_buf[BUF_LEN] = {0};
	unsigned char recv_buf[BUF_LEN] = {0};
	unsigned char data_head[12] = {0};
	hid_cmd_Head *hid_head = NULL;
	int res = 0;
	hid_head = (struct hid_cmd_Head *)hid_buf;
	hid_head->report_id = HID_REPORT_ID;
	hid_head->cmd = cmd;
	hid_head->magic = MAGIC;
	hid_head->data_status = HID_DATA_END;
	hid_head->data_len = 0;
	memcpy(data_head,&hid_buf[0],12);
	unsigned short head_crc = soft_crc16(data_head,12,0xFFFF);
	hid_head->crc = head_crc;
	printfStructSend(hid_buf);
	res = hid_write(handle, (unsigned char*)hid_buf, BUF_LEN);
	res = hid_read_timeout(handle,(unsigned char*)recv_buf,BUF_LEN,3000);
	if (res == 0)
	{
		hid_close(handle);
		hid_exit();
		return strReturnInfo;
	}
	m_response_hid_cmd_HeadWithData = (response_hid_cmd_HeadWithData *)recv_buf;
	hid_close(handle);
	hid_exit();
	if (m_response_hid_cmd_HeadWithData->response_status == CMD_EXCUTE_OK && m_response_hid_cmd_HeadWithData->cmd == cmd)
	{
		//strReturnInfo.Format(_T("%s"),m_response_hid_cmd_HeadWithData->data);
		strReturnInfo = AnsiToUnicode(m_response_hid_cmd_HeadWithData->data,m_response_hid_cmd_HeadWithData->data_len);
		return strReturnInfo;
	}
	else
	{
		return strReturnInfo;
	}
}


CString CUSBDlg::AnsiToUnicode(char * szAnsi, int len)
{
	CString str;
	// ansi to unicode
	//预转换，得到所需空间的大小
	int wcsLen;
	if (len>0)
		wcsLen = len;
	else
		wcsLen = ::MultiByteToWideChar(CP_ACP, NULL, szAnsi, strlen(szAnsi), NULL, 0);
	//分配空间要给'\0'留个空间，MultiByteToWideChar不会给'\0'空间
	wchar_t* wszString = new wchar_t[wcsLen + 1];
	//转换
	::MultiByteToWideChar(CP_ACP, NULL, szAnsi, strlen(szAnsi), wszString, wcsLen);
	//最后加上'\0'
	wszString[wcsLen] = '\0';            // UNICODE字串
	str = wszString;
	delete wszString;
	return str;
}

void CUSBDlg::OnBnClickedButtonReadSn()
{
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	CString strRes = ReadInfoFromDevice(handle,HID_BASIC_INFO_READ_SN);
	if (strRes == _T(""))
	{
		AfxMessageBox(_T("读SN失败!"));
	}
	else
	{
		m_editRead_SN.SetWindowTextW(strRes);
	}
}


void CUSBDlg::OnBnClickedButtonReadManufacture()
{
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	CString strRes = ReadInfoFromDevice(handle,HID_BASIC_INFO_READ_MANUFACTURE);
	if (strRes == _T(""))
	{
		AfxMessageBox(_T("读Manufacture失败!"));
	}
	else
	{
		m_editRead_MANUFACTURER.SetWindowTextW(strRes);
	}
}


void CUSBDlg::OnBnClickedButtonReadPid()
{
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	CString strRes = ReadInfoFromDevice(handle,HID_BASIC_INFO_READ_PID);
	if (strRes == _T(""))
	{
		AfxMessageBox(_T("读PID失败!"));
	}
	else
	{
		m_editRead_PID.SetWindowTextW(strRes);
	}
}


void CUSBDlg::OnBnClickedButtonReadBcd()
{
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	CString strRes = ReadInfoFromDevice(handle,HID_BASIC_INFO_READ_BCD);
	if (strRes == _T(""))
	{
		AfxMessageBox(_T("读BCD失败!"));
	}
	else
	{
		m_editRead_BCD.SetWindowTextW(strRes);
	}
}


void CUSBDlg::OnBnClickedButtonReadVid()
{
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	CString strRes = ReadInfoFromDevice(handle,HID_BASIC_INFO_READ_VID);
	if (strRes == _T(""))
	{
		AfxMessageBox(_T("读VID失败!"));
	}
	else
	{
		m_editRead_VID.SetWindowTextW(strRes);
	}
}


void CUSBDlg::OnBnClickedButtonReadModel()
{
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	CString strRes = ReadInfoFromDevice(handle,HID_BASIC_INFO_READ_MODEL);
	if (strRes == _T(""))
	{
		AfxMessageBox(_T("读Mode失败!"));
	}
	else
	{
		m_editRead_Model.SetWindowTextW(strRes);
	}
}


void CUSBDlg::OnBnClickedButtonReadCmei()
{
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	CString strRes = ReadInfoFromDevice(handle,HID_BASIC_INFO_READ_CMEI);
	if (strRes == _T(""))
	{
		AfxMessageBox(_T("读CMEI失败!"));
	}
	else
	{
		m_editRead_CMEI.SetWindowTextW(strRes);
	}
}


void CUSBDlg::OnBnClickedButtonOpenPacket()
{
	// TODO: 在此添加控件通知处理程序代码
	COpenPacketDialog openPacketDialog;
	openPacketDialog.DoModal();
}


void CUSBDlg::OnBnClickedButtonOpenpacket()
{
	CString defaultDir = L"E:\\FileTest";
	CString fileName = L"";
	TCHAR filter[] = _T("文本文件(*.txt)||所有文件(*.*)||");
	CFileDialog openFileDlg(TRUE, defaultDir, fileName, OFN_HIDEREADONLY|OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\test.doc";
	INT_PTR result = openFileDlg.DoModal();
	if(result == IDOK) {
		filePath_Updatepacket = openFileDlg.GetPathName();
		m_edit_path_packet.SetWindowTextW(filePath_Updatepacket);
	}
}


void CUSBDlg::OnBnClickedButtonUpdatePacket()
{
	m_ProgressCtrl.SetPos(0);
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	if(filePath_Updatepacket == _T(""))
	{
		AfxMessageBox(_T("文件名不可以为空!"));
		return;
	}
	hid_cmd_Head *hid_head = NULL;
	char *packet_buffer = (char*)malloc(10*1024*1024);
	m_Static_Update_Info.SetWindowTextW(_T("升级中..."));
	int res = -1;
	int file_count = GetPrivateProfileInt(_T("Update_Info"),_T("file_num"),0,filePath_Updatepacket);
	for(int i=1;i<= file_count+1;i++)
	{
		if(i == file_count + 1)
		{
			break;
			//发送文件结束命令
			//ReadInfoFromDevice(handle,HOU_UPDATE_END);
			CString strReturnInfo = _T("");
			unsigned char hid_buf[BUF_LEN] = {0};
			unsigned char recv_buf[BUF_LEN] = {0};
			unsigned char data_head[12] = {0};
			int res = 0;
			hid_head = (struct hid_cmd_Head *)hid_buf;
			hid_head->report_id = HID_REPORT_ID;
			hid_head->cmd = HOU_UPDATE_END;
			hid_head->magic = MAGIC;
			hid_head->data_status = HID_DATA_END;
			hid_head->data_len = 0;
			memcpy(data_head,&hid_buf[0],12);
			unsigned short head_crc = soft_crc16(data_head,12,0xFFFF);
			hid_head->crc = head_crc;
			printfStructSend(hid_buf);
			res = hid_write(handle, (unsigned char*)hid_buf, BUF_LEN);
			res = hid_read_timeout(handle,(unsigned char*)recv_buf,BUF_LEN,3000);

			m_response_hid_cmd_HeadWithData = (response_hid_cmd_HeadWithData *)recv_buf;
			/*if (m_response_hid_cmd_HeadWithData->response_status == CMD_EXCUTE_OK && m_response_hid_cmd_HeadWithData->cmd == HOU_UPDATE_END)
			{

			}
			else
			{

			}*/
		}
		else
		{
			m_ProgressCtrl.SetPos(0);
			CString cs_fileAddNum;
			cs_fileAddNum.Format(_T("file%d"),i);
			CString cs_fileName;//当前文件
			CString cs_DevieFilePath;
			GetPrivateProfileString(cs_fileAddNum,_T("filename"),_T(""),cs_fileName.GetBuffer(MAX_PATH),MAX_PATH,filePath_Updatepacket);
			GetPrivateProfileString(cs_fileAddNum,_T("DevicefilePath"),_T(""),cs_DevieFilePath.GetBuffer(MAX_PATH),MAX_PATH,filePath_Updatepacket);

			//char *packet_buffer = (char*)malloc(10*1024*1024);
			memset(packet_buffer,0, 10 * 1024*1024);
			short rev = 1;
			FILE *fw_fd;
			USES_CONVERSION;
			char* devicePath = T2A(cs_DevieFilePath.GetBuffer(0));
			cs_DevieFilePath.ReleaseBuffer();

			CString  strProgramPath;
			GetModuleFileName(NULL,strProgramPath.GetBuffer(MAX_PATH),MAX_PATH);
			strProgramPath.ReleaseBuffer(MAX_PATH);
			int nPathPos  = strProgramPath.ReverseFind('\\');
			strProgramPath = strProgramPath.Left(nPathPos + 1);

			CString str;
			str.Format(_T("%s%s"),strProgramPath,cs_fileName);
			char* devicefilePath = T2A(str.GetBuffer(0));
			str.ReleaseBuffer();

			if(fopen_s(&fw_fd,devicefilePath,"rb") != 0)
			{
				AfxMessageBox(_T("打开文件失败!"));
				free(packet_buffer);
				break;
			}
			long file_size;
			unsigned long length;
			long offset = 0;
			fseek(fw_fd, 0, SEEK_END);
			file_size = ftell(fw_fd);
			fseek(fw_fd, 0, SEEK_SET);
			long read_size = file_size;
			while (rev) {
				rev = fread(packet_buffer + offset, 1, read_size, fw_fd);
				read_size -= rev;
				offset += rev;
			}
			fclose(fw_fd);
			res = updateFileFromPacket(handle,packet_buffer,HOU_UPDATE_ALL_INIT,file_size,devicePath);
			//free(packet_buffer);
			if(res != 0)
			{
				CString strMessage;
				strMessage.Format(_T("升级%s失败"),cs_fileName);
				AfxMessageBox(strMessage);
				m_ProgressCtrl.SetPos(0);
				free(packet_buffer);
				break;
			}
			m_ProgressCtrl.SetPos(100);
			if(cs_DevieFilePath.Find(_T(".lzma")) != -1)
			{
				Sleep(8000);
			}

			/*if(file_size > MAX_3M_LENGTH)
			{
				Sleep(6000);
			}*/
		}
	}
	free(packet_buffer);
	hid_close(handle);
	hid_exit();
	if(res == 0)
	{
		m_ProgressCtrl.SetPos(100);
		m_Static_Update_Info.SetWindowTextW(_T("升级完成!"));
		AfxMessageBox(_T("升级成功!"));
		m_ProgressCtrl.SetPos(0);
		m_Static_Update_Info.SetWindowTextW(_T(""));
	}
	else
	{
		m_Static_Update_Info.SetWindowTextW(_T(""));
	}
}


int CUSBDlg::updateFileFromPacket(hid_device* handle,char* data,int cmd_file_type,unsigned long datalen,char* deviceFilePath)
{
	TRACE("data:%s\n",data);
	hid_cmd_Head *hid_head = NULL;
	int res = hid_set_nonblocking(handle,0);
	int retryCount = 0;
	unsigned char hid_buf[BUF_LEN] = {0};
	unsigned char recv_buf[BUF_LEN] = {0};
	unsigned char data_head[12] = {0};
	struct response_hid_cmd_Head hid_cmd_respone_Head;

	// debug
	TRACE("m_hid_update_model = %p, %d\n", m_hid_update_model, __LINE__);


	//第一步;发送HOU_START
HOU_START_POINT:
	memset(&hid_cmd_respone_Head, 0, sizeof(response_hid_cmd_Head));
	hid_head = (struct hid_cmd_Head *)hid_buf;
	hid_head->report_id = HID_REPORT_ID;
	hid_head->cmd = HOU_START;
	hid_head->magic = MAGIC;
	hid_head->data_status = HID_DATA_INVALID;
	memcpy(data_head,&hid_buf[0],12);
	hid_head->crc = soft_crc16(data_head,12,0xFFFF);

	printfStructSend(hid_buf);
	res = hid_write(handle, (unsigned char*)hid_buf, BUF_LEN);
	//res = hid_read_timeout(handle,(unsigned char*)recv_buf,BUF_LEN,1000);
	res = hid_read_timeout(handle,(unsigned char*)&hid_cmd_respone_Head,sizeof(hid_cmd_respone_Head),1000);
	if (res == 0)
	{
		AfxMessageBox(_T("上位机收不到设备端的数据"));
		m_ProgressCtrl.SetPos(0);
		return -1;
	}
	//m_response_hid_cmd_Head = (response_hid_cmd_Head *)recv_buf;
	m_response_hid_cmd_Head = &hid_cmd_respone_Head;
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
			m_ProgressCtrl.SetPos(0);
			return -1;
		}
	}
	TRACE("report_id:%x,magic:%x,cmd:%x,data_status:%x,datalen:%x,response_status:%x,crc:%x\n",m_response_hid_cmd_Head->report_id,\
		m_response_hid_cmd_Head->magic,m_response_hid_cmd_Head->cmd,m_response_hid_cmd_Head->data_status,m_response_hid_cmd_Head->data_len,\
		m_response_hid_cmd_Head->response_status,m_response_hid_cmd_Head->crc);

	//第二步:告知下位发送什么类型的文件，比如：HOU_UPDATE_UVCCONFIG
	retryCount = 0;
HOU_UPDATE_UVCCONFIG_POTINT:
	memset(hid_buf,0,BUF_LEN);
	memset(&hid_cmd_respone_Head, 0, sizeof(response_hid_cmd_Head));
	hid_head = NULL;
	hid_head = (struct hid_cmd_Head *)hid_buf;
	hid_head->report_id = HID_REPORT_ID;
	hid_head->pack_num = 1;
	hid_head->cmd = cmd_file_type;
	hid_head->magic = MAGIC;
	hid_head->data_len = strlen(deviceFilePath);
	hid_head->data_status = HID_DATA_END;
	memcpy(data_head,&hid_buf[0],12);
	unsigned short head_crc = soft_crc16(data_head,12,0xFFFF);
	memcpy(data_head,hid_buf,12);
	hid_head->crc = soft_crc16((unsigned char*)deviceFilePath,strlen(deviceFilePath), head_crc);
	memcpy(&hid_buf[14],deviceFilePath,strlen(deviceFilePath));
	printfStructSend(hid_buf);
	res = hid_write(handle, (unsigned char*)hid_buf, BUF_LEN);
	res = hid_read_timeout(handle,(unsigned char*)&hid_cmd_respone_Head,BUF_LEN,1000);
	//m_response_hid_cmd_Head = (response_hid_cmd_Head *)recv_buf;
	m_response_hid_cmd_Head = &hid_cmd_respone_Head;
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
			m_ProgressCtrl.SetPos(0);
			return -1;
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
	long file_size = datalen;
	int buffer_offset = 0;
	int cnt = 0;
	int packNum = 0;
	int iPos = 0;
	int cnt_Need_Send_Times = file_size / BUF_LEN;
	int cnt_JianGe = cnt_Need_Send_Times / 100;
	if( cnt_JianGe == 0 )
	{
		cnt_JianGe = 1;
	}
	rev = len;
	while (1)
	{
		if (buffer_offset <= file_size) {
			if ((file_size - buffer_offset) > len) {
				memcpy (hid_buf + size_Head, data + buffer_offset, len);
				rev = len;
				buffer_offset += len;
				hid_head->data_status = 1;

			} else if (((file_size - buffer_offset) <= len) && ((file_size - buffer_offset) > 0)){
				memcpy (hid_buf + size_Head, data + buffer_offset, file_size - buffer_offset);
				rev = file_size - buffer_offset;
				buffer_offset = file_size;
				hid_head->data_status = 2;
			} else if ((file_size - buffer_offset) == 0){
				break;
			}
		}

		MSG msg;
		if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_CLOSE || msg.message == WM_DESTROY)
				break;
			TranslateMessage (&msg) ;
			DispatchMessage (&msg) ;
		}
		else
		{
			// 完成某些工作的其它行程序
			TRACE("hello world!\n");

		}

		Sleep(30);
		if(packNum % cnt_JianGe == 0)
		{
			m_ProgressCtrl.SetPos(iPos);
			iPos++;
		}
		packNum++;
		retryCount = 0;
HOU_SEND_DATA_POINT:

		memset(&hid_cmd_respone_Head, 0, sizeof(response_hid_cmd_Head));
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
		res = hid_write(handle, (unsigned char*)hid_buf, BUF_LEN);
		Sleep(5);
		memset(recv_buf,0,BUF_LEN);
		res = hid_read_timeout(handle,(unsigned char*)&hid_cmd_respone_Head,BUF_LEN,3000);
		//m_response_hid_cmd_Head = (response_hid_cmd_Head *)recv_buf;
		m_response_hid_cmd_Head = &hid_cmd_respone_Head;
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
			Sleep(200);
			goto HOU_SEND_DATA_POINT;
		}
		if ( m_response_hid_cmd_Head->response_status != CMD_EXCUTE_OK && retryCount < MAX_RETRY_COUNT && m_response_hid_cmd_Head->cmd == HOU_SEND_DATA)
		{
			TRACE("retryCount:%d\n",retryCount);
			retryCount++;
			Sleep(200);
			goto HOU_SEND_DATA_POINT;
		}
		else
		{
			if(retryCount == MAX_RETRY_COUNT)
			{
				AfxMessageBox(_T("发送升级包失败!"));
				m_ProgressCtrl.SetPos(0);
				return -1;
			}
		}
	}
	retryCount = 0;


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
	/*if(datalen > MAX_3M_LENGTH)
	{
		Sleep(20000);
	}*/
	memset(recv_buf,0,BUF_LEN);
	res = hid_read_timeout(handle,(unsigned char*)recv_buf,BUF_LEN,3000);
	TRACE("readlength:%d\n",res);
	//memset(m_response_hid_cmd_Head,0,sizeof(response_hid_cmd_Head));
	m_response_hid_cmd_Head = (response_hid_cmd_Head *)recv_buf;
	TRACE("report_id:%x,magic:%x,cmd:%x,data_status:%x,datalen:%x,response_status:%x,crc:%x\n",m_response_hid_cmd_Head->report_id,\
		m_response_hid_cmd_Head->magic,m_response_hid_cmd_Head->cmd,m_response_hid_cmd_Head->data_status,m_response_hid_cmd_Head->data_len,\
		m_response_hid_cmd_Head->response_status,m_response_hid_cmd_Head->crc);

	/*if (m_response_hid_cmd_Head->cmd != HOU_GET_RESULT)
	{
	retryCount++;
	goto HOU_GET_RESULT_POINT;
	}*/
	if ((m_response_hid_cmd_Head ->response_status != CMD_EXCUTE_OK && retryCount < MAX_RETRY_COUNT && m_response_hid_cmd_Head->cmd == HOU_GET_RESULT) || m_response_hid_cmd_Head->cmd != HOU_GET_RESULT) {
		retryCount++;
		goto HOU_GET_RESULT_POINT;
	}
	else
	{
		if(retryCount == MAX_RETRY_COUNT)
		{
			AfxMessageBox(_T("发送获取升级结果失败!"));
			m_ProgressCtrl.SetPos(0);
			return -1;
		}
	}
	/*if(m_response_hid_cmd_Head->response_status == CMD_EXCUTE_OK)
	{

	}*/
	return 0;
}

void CUSBDlg::OnBnClickedButtonGetServerInfo()
{
	hid_cmd_Head *hid_head = NULL;
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	//第一步：发送算法授权开始命令
	unsigned char hid_buf[BUF_LEN] = {0};
	unsigned char recv_buf[BUF_LEN] = {0};
	unsigned char data_head[12] = {0};
	int res  = -1;
	CString strSN  /*= _T("ae7117082a18d846e4ea433bcf7e3d2b")*/;
	m_editAlogirmSN.GetWindowTextW(strSN);
	int snLength = strSN.GetLength();
	int nBytes = WideCharToMultiByte(CP_ACP,0,strSN,snLength,NULL,0,NULL,NULL);
	char* c_SN = new char[ nBytes + 1];
	memset(c_SN,0,snLength + 1);
	WideCharToMultiByte(CP_OEMCP, 0, strSN, snLength, c_SN, nBytes, NULL, NULL);
	c_SN[nBytes] = 0;
	CString strCid /*= _T("1")*/;
	GetDlgItem(IDC_TEXT_CID)->GetWindowTextW(strCid);
	int cid = _ttoi(strCid);
	CString strPid  /*= _T("1825793026")*/;
	GetDlgItem(IDC_TEXT_PID)->GetWindowTextW(strPid);
	int pid = _ttoi(strPid);
	memset(&m_pAlgoththm_info,0,sizeof(algoththm_info));
	m_pAlgoththm_info.cid = cid;
	m_pAlgoththm_info.fid = pid;
	memcpy(m_pAlgoththm_info.sn,c_SN,nBytes);
	int size = sizeof(int)*2 + nBytes;

	if(strSN == _T("") || strCid == _T("") || strPid == _T(""))
	{
		AfxMessageBox(_T("CID,FID,SN不可以为空"));
		return;
	}

	/*m_pAlgoththm_info->report_id = HID_REPORT_ID;
	m_pAlgoththm_info->cmd = HAA_DEVICE_INFO;*/
	hid_head = (struct hid_cmd_Head *)hid_buf;
	hid_head->report_id = HID_REPORT_ID;
	hid_head->cmd = HAA_START;
	hid_head->magic = MAGIC;
	hid_head->data_status = HID_DATA_END;
	hid_head->data_len = size;
	memcpy(data_head,&hid_buf[0],12);
	unsigned short head_crc = soft_crc16(data_head,12,0xFFFF);
	unsigned short crc = soft_crc16((unsigned char*)&m_pAlgoththm_info,size,head_crc);
	hid_head->crc = crc;
	memcpy(&hid_buf[14],&m_pAlgoththm_info,size);
	printfStructSend(hid_buf);
	res = hid_write(handle, (unsigned char*)hid_buf, BUF_LEN);
	res = hid_read_timeout(handle,(unsigned char*)recv_buf,BUF_LEN,3000);
	if (res == 0)
	{
		hid_close(handle);
		hid_exit();
		return;
	}
	m_response_hid_cmd_Head = (response_hid_cmd_Head *)recv_buf;
	if (m_response_hid_cmd_Head->response_status == CMD_EXCUTE_OK)
	{

	}
	else
	{

	}

	//第二步：将pid，cid，sn通过 HAA_DEVICE_INFO 传给设备从而获取设备发送过来的服务器地址和端口
	memset(hid_head,0,sizeof(hid_cmd_Head));
	hid_head = (struct hid_cmd_Head *)hid_buf;
	hid_head->report_id = HID_REPORT_ID;
	hid_head->cmd = HAA_DEVICE_INFO;
	hid_head->magic = MAGIC;
	hid_head->data_status = HID_DATA_INVALID;
	memcpy(data_head,&hid_buf[0],12);
	hid_head->crc = soft_crc16(data_head,12,0xFFFF);
	res = hid_write(handle, (unsigned char*)hid_buf, BUF_LEN);
	res = hid_read_timeout(handle,(unsigned char*)recv_buf,BUF_LEN,3000);
	if (res == 0)
	{
		hid_close(handle);
		hid_exit();
		return;
	}
	m_IAACAuthInfoResponse = NULL;
	m_IAACAuthInfoResponse = (IAACAuthInfoResponse *)recv_buf;
	if (m_IAACAuthInfoResponse->res_hid_head.response_status == CMD_EXCUTE_OK)
	{
		memcpy(&m_iaacauthInfo,&(m_IAACAuthInfoResponse->iaacauthInfo),sizeof(IAACAuthInfo));
		memcpy(c_senddata,m_IAACAuthInfoResponse->iaacauthInfo.need_send_data,m_IAACAuthInfoResponse->iaacauthInfo.need_send_data_len);
		sendLength = m_IAACAuthInfoResponse->iaacauthInfo.need_send_data_len;
		ipAddress = URLToIPAddr(m_IAACAuthInfoResponse->iaacauthInfo.url);
		ipPort = m_IAACAuthInfoResponse->iaacauthInfo.port;
		//连接服务器，将need_send_data发送给服务器，从而获取服务器发送过来的信息
		//初始化
		AfxSocketInit();
		//创建 CSocket 对象
		CSocket aSocket;
		CString strIP(ipAddress);

		//初始化 CSocket 对象, 因为客户端不需要绑定任何端口和地址, 所以用默认参数即可
		if (!aSocket.Create())
		{
			AfxMessageBox(_T("初始化socket失败!"));
			hid_close(handle);
			hid_exit();
			return;
		}
		//连接指定的地址和端口
		if (aSocket.Connect(strIP, ipPort))
		{
			char szRecValue[1024] = { 0 };
			int xx = aSocket.Send(m_iaacauthInfo.need_send_data, m_iaacauthInfo.need_send_data_len);
			//接收服务器发送回来的内容(该方法会阻塞, 在此等待有内容接收到才继续向下执行)
			int yy = aSocket.Receive((void *)szRecValue, 1024);
			//头结构体14个字节，最后2个字节是CRC的值
			unsigned char hid_buf[BUF_LEN] = {0};
			unsigned char recv_buf[BUF_LEN] = {0};
			unsigned char data_head[12] = {0};
			int res = -1;
			hid_head = (struct hid_cmd_Head *)hid_buf;
			hid_head->report_id = HID_REPORT_ID;
			hid_head->cmd = HAA_WARRANT_INFO;
			hid_head->magic = MAGIC;
			hid_head->data_status = HID_DATA_END;
			hid_head->data_len = yy;
			memcpy(data_head,&hid_buf[0],12);
			unsigned short head_crc = soft_crc16(data_head,12,0xFFFF);
			unsigned short crc = soft_crc16((unsigned char*)szRecValue,yy,head_crc);
			hid_head->crc = crc;
			memcpy(&hid_buf[14],szRecValue,yy);
			printfStructSend(hid_buf);
			int retryCount = 0;
sendCMD_HAA_WARRANT_INFO:
			memset(recv_buf,0,BUF_LEN);
			Sleep(1000);
			res = hid_write(handle, (unsigned char*)hid_buf, BUF_LEN);
			res = hid_read_timeout(handle,(unsigned char*)recv_buf,BUF_LEN,3000);
			m_response_hid_cmd_Head = (response_hid_cmd_Head *)recv_buf;
			if (res == 0)
			{
				if(retryCount < 6)
				{
					retryCount++;
					goto sendCMD_HAA_WARRANT_INFO;
				}
				if (retryCount == 6)
				{
					AfxMessageBox(_T("授权失败!"));
					aSocket.Close();
					hid_close(handle);
					hid_exit();
					return;
				}

				/*AfxMessageBox(_T("授权失败!"));
				aSocket.Close();
				hid_close(handle);
				hid_exit();
				return;*/
				//retry

			}
			else
			{
				if (m_response_hid_cmd_Head->response_status == CMD_EXCUTE_OK && m_response_hid_cmd_Head->cmd == HAA_WARRANT_INFO)
				{
					AfxMessageBox(_T("授权成功!"));
				}
				else
				{
					//AfxMessageBox(_T("授权失败!"));
					if(retryCount < 6)
					{
						retryCount++;
						goto sendCMD_HAA_WARRANT_INFO;
					}
					if (retryCount == 6)
					{
						AfxMessageBox(_T("授权失败!"));
					}
				}
			}

		}
		else
		{
			AfxMessageBox(_T("连接服务器失败!"));
			aSocket.Close();
			hid_close(handle);
			hid_exit();
			return;
		}
		aSocket.Close();
	}
	else
	{

	}
	hid_close(handle);
	hid_exit();
}


char* CUSBDlg::URLToIPAddr(char *url)
{
	 DWORD  nIpAddr = 0;
	 WSADATA wsaData;
	 WSAStartup( MAKEWORD(2, 2), &wsaData);
	 if(strlen(url) < 1)
		 return 0;
	 nIpAddr = inet_addr(url);
	 if(INADDR_NONE == nIpAddr)
	 {
		//通过DNS转换为IP地址
		struct hostent *host = gethostbyname(url);
		if(NULL == host)
			return 0;
		//别名
		for(int i=0; host->h_aliases[i]; i++){
			TRACE("Aliases %d: %s\n", i+1, host->h_aliases[i]);
		}

		//地址类型
		TRACE("Address type: %s\n", (host->h_addrtype==AF_INET) ? "AF_INET": "AF_INET6");

		//IP地址
		for(int i=0; host->h_addr_list[i]; i++){
			TRACE("IP addr %d: %s\n", i+1, inet_ntoa( *(struct in_addr*)host->h_addr_list[i] ) );
		}
		//nIpAddr =*((unsigned long *)host->h_addr_list[0]);
		return inet_ntoa( *(struct in_addr*)host->h_addr_list[0] );
	 }
	 return "";
}

void CUSBDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CDialogEx::OnClose();
	AfxGetMainWnd()->PostMessage(WM_CLOSE, 0, 0);
}


void CUSBDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
}


void CUSBDlg::OnBnClickedButtonReadVersion()
{
	// TODO: 在此添加控件通知处理程序代码
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	CString strRes = ReadInfoFromDevice(handle,HID_BASIC_INFO_READ_VERSION);
	if (strRes == _T(""))
	{
		AfxMessageBox(_T("读版本号失败!"));
	}
	else
	{
		m_editReadVersion.SetWindowTextW(strRes);
	}
}


void CUSBDlg::OnBnClickedButtonReadAppkey()
{
	// TODO: 在此添加控件通知处理程序代码
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	CString strRes = ReadInfoFromDevice(handle,HID_BASIC_INFO_READ_APPKEY);
	if (strRes == _T(""))
	{
		AfxMessageBox(_T("读版AppKey失败!"));
	}
	else
	{
		m_editReadAppKey.SetWindowTextW(strRes);
	}
}


void CUSBDlg::OnBnClickedButtonWriteAppkey()
{
	// TODO: 在此添加控件通知处理程序代码
	hid_device *handle = openHIDDevice();
	if (handle == NULL)
	{
		return;
	}
	CString str;
	m_editWrite_AppKey.GetWindowText(str);
	if (str == "")
	{
		AfxMessageBox(_T("AppKey不可以为空"));
		hid_close(handle);
		hid_exit();
		return;
	}
	int res = WriteInfoToDevice(handle,str,HID_BASIC_INFO_APPKEY);
	if(res == 0)
	{
		AfxMessageBox(_T("写入AppKey成功!"));
	}
	else
	{
		AfxMessageBox(_T("写入AppKey失败!"));
	}
}
