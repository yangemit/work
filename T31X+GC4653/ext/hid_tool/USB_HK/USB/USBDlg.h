
// USBDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "hidapi.h"
#include "OpenPacketDialog.h"
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string.h>
#include <afxsock.h>


#define WM_SOCKET WM_USER + 100
#define MAX_PACKET_LENGTH		10*1024*1024
#define MAX_3M_LENGTH				3*1024*1024

#define HID_FW_DATA_END                         0xff
#define MAX_STR			256
#define  MAX_RETRY_COUNT	6

#define ALGORITHM_AUTHORIZED	0x1F01

#define MAGIC					0x5a5a
#define BUF_LEN					1024
#define HID_REPORT_ID			0x01

//向设备写入一些基本信息
#define HID_BASIC_INFO			0x2000
#define HID_BASIC_INFO_MANUFACTURE		(HID_BASIC_INFO + 0x001)
#define HID_BASIC_INFO_SN				(HID_BASIC_INFO + 0x011)
#define HID_BASIC_INFO_PRODUCT			(HID_BASIC_INFO + 0x021)
#define HID_BASIC_INFO_VID				(HID_BASIC_INFO + 0x031)
#define HID_BASIC_INFO_PID				(HID_BASIC_INFO + 0x041)
#define HID_BASIC_INFO_BCD				(HID_BASIC_INFO + 0x051)
#define HID_BASIC_INFO_MODEL			(HID_BASIC_INFO + 0xf01)
#define HID_BASIC_INFO_CMEI				(HID_BASIC_INFO + 0xf11)
#define HID_BASIC_INFO_APPKEY			(HID_BASIC_INFO + 0xf21)
//读取设备信息
#define HID_BASIC_INFO_READ_MANUFACTURE		(HID_BASIC_INFO + 0x000)
#define HID_BASIC_INFO_READ_SN				(HID_BASIC_INFO + 0x010)
#define HID_BASIC_INFO_READ_PRODUCT			(HID_BASIC_INFO + 0x020)
#define HID_BASIC_INFO_READ_VID				(HID_BASIC_INFO + 0x030)
#define HID_BASIC_INFO_READ_PID				(HID_BASIC_INFO + 0x040)
#define HID_BASIC_INFO_READ_BCD				(HID_BASIC_INFO + 0x050)
#define HID_BASIC_INFO_READ_MODEL			(HID_BASIC_INFO + 0xf00)
#define HID_BASIC_INFO_READ_CMEI			(HID_BASIC_INFO + 0xf10)
#define HID_BASIC_INFO_READ_APPKEY			(HID_BASIC_INFO + 0xf20)


//算法授权
#define HID_COMMON_USAGE	0X0000
#define HID_ALGORITHM_AUTHORIZE 0X1000
#define HAA_START		(HID_ALGORITHM_AUTHORIZE + 0x100)
#define HAA_END			(HID_ALGORITHM_AUTHORIZE + 0x110)
#define HAA_DEVICE_INFO	(HID_ALGORITHM_AUTHORIZE + 0x200)
#define HAA_WARRANT_INFO	(HID_ALGORITHM_AUTHORIZE + 0x210)

#define HAA_GET_START	(HID_ALGORITHM_AUTHORIZE + 0x000)
#define HAA_SET_START	(HID_ALGORITHM_AUTHORIZE + 0x010)

/* data status */
#define HID_DATA_INVALID		0x00
#define HID_DATA_VALID			0x01
#define HID_DATA_END			0x02

/* CMD status */
#define CMD_EXCUTE_OK           0x00
#define CMD_EXCUTE_ERROR        0x01
#define CMD_REPORTID_ERROR      0x02
#define CMD_MAGIC_ERROR         0x03
#define CMD_CRC_ERROR           0x04
#define CMD_UNSUPPORT_FUNCTION  0x05

// CUSBDlg 对话框
#pragma pack(1)
struct hid_cmd_Head
{
	unsigned char report_id;
	unsigned short magic;		//0x5aa5
	unsigned short cmd;
	unsigned char data_status;	//1表示有数据，0表示无数据，2表示有数据且数据结束
	unsigned short data_len;
	unsigned int pack_num;
	unsigned short crc;
};


struct response_hid_cmd_Head
{
	unsigned char report_id;
	unsigned short magic;
	unsigned short cmd;
	unsigned char data_status;	
	unsigned short data_len;
	unsigned int response_status;
	unsigned short crc;
};

struct response_hid_cmd_HeadWithData
{
	unsigned char report_id;
	unsigned short magic;
	unsigned short cmd;
	unsigned char data_status;	
	unsigned short data_len;
	unsigned int response_status;
	unsigned short crc;
	char data[256];
	//char* data;
};

struct algoththm_info
{
	int cid;
	int fid;
	char sn[256];
};

struct IAACAuthInfo
{
	char url[64];
	char ip[64];
	int port;
	unsigned char need_send_data[128];
	int need_send_data_len;
	unsigned char* need_recv_data;
	int need_recv_data_len;
};

struct IAACAuthInfoResponse
{
	response_hid_cmd_Head res_hid_head;
	IAACAuthInfo iaacauthInfo;
};
#pragma pack()

class CUSBDlg : public CDialogEx
{
// 构造
public:
	CUSBDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_USB_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	int updateFileFromPacket(hid_device* handle,char* data,int cmd_file_type,unsigned long datalen,char* deviceFilePath);
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString strContent;
	CString filePath;
	CString filePath_UvcAttr;
	CString filePath_UvcApp;
	CString filePath_UvcApp2;
	CString filePath_Sensor_Driver;
	CString filePath_Sensor_Settings;
	CString filePath_App_Init;
	CString filePath_Fast_Init;
	CString filePath_Hid_Test;
	CString filePath_Updatepacket;
	CListCtrl m_ListInfo;
	hid_cmd_Head *hid_head;
	algoththm_info m_pAlgoththm_info;
	IAACAuthInfoResponse* m_IAACAuthInfoResponse;
	IAACAuthInfo m_iaacauthInfo;
	response_hid_cmd_Head* m_response_hid_cmd_Head;
	response_hid_cmd_HeadWithData* m_response_hid_cmd_HeadWithData;
	hid_update_model *m_hid_update_model;
	hid_update_model *m_hid_update_modelTemp;
	hid_update_head_start *m_hid_update_head_start;
	POINT old;
	afx_msg void OnBnClickedButtonScan();
	afx_msg void OnBnClickedButtonSn();
	unsigned short soft_crc16(unsigned char *buf, unsigned int len, unsigned short crc);
	CEdit m_Edit_SN;
	int WriteInfoToDevice(hid_device* handle,CString info,int cmd);
	afx_msg void OnBnClickedButtonVid();
	void printfStructSend(unsigned char *buf);
	afx_msg void OnNMCustomdrawListInfo(NMHDR *pNMHDR, LRESULT *pResult);
	hid_device* openHIDDevice();
	void updateFile(hid_device* handle,CString filePath,int cmd_file_type);
	bool updateFileSuccess(hid_device* handle,CString filePath,int cmd_file_type);
	CEdit m_editPID;
	CEdit m_editVID;
	CEdit m_editBCD;
	CEdit m_editManufacture;
	afx_msg void OnBnClickedButtonPid();
	afx_msg void OnBnClickedButtonManufacture();
	afx_msg void OnBnClickedButtonBcd();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void ReSize(void);
	CEdit m_editWrite_Model;
	CEdit m_editWrite_Cmei;
	afx_msg void OnBnClickedButtonWriteCmei();
	afx_msg void OnBnClickedButtonModel();
	CEdit m_editRead_SN;
	CEdit m_editRead_PID;
	CEdit m_editRead_VID;
	CEdit m_editRead_CMEI;
	CEdit m_editRead_MANUFACTURER;
	CEdit m_editRead_BCD;
	CEdit m_editRead_Model;
	afx_msg void OnBnClickedButtonReadSn();
	CString ReadInfoFromDevice(hid_device* handle,int cmd);
	afx_msg void OnBnClickedButtonReadManufacture();
	afx_msg void OnBnClickedButtonReadPid();
	afx_msg void OnBnClickedButtonReadBcd();
	afx_msg void OnBnClickedButtonReadVid();
	afx_msg void OnBnClickedButtonReadModel();
	afx_msg void OnBnClickedButtonReadCmei();
	CString AnsiToUnicode(char * szAnsi, int len);
	afx_msg void OnBnClickedButtonOpenPacket();
	CEdit m_edit_path_packet;
	afx_msg void OnBnClickedButtonOpenpacket();
	afx_msg void OnBnClickedButtonUpdatePacket();
	afx_msg void OnBnClickedButtonGetServerInfo();
	int send_recv(IAACAuthInfo *authInfo);
	CEdit m_editAlogirmSN;
	unsigned char c_senddata[128];
	int sendLength;
	char* URLToIPAddr(char *url);
	char* ipAddress;
	int ipPort;
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	CProgressCtrl m_ProgressCtrl;
	CStatic m_Static_Update_Info;
};
