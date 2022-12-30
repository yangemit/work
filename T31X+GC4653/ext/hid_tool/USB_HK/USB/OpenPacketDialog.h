#pragma once
#include "afxwin.h"
#include <iostream>
#include <stdio.h>


// COpenPacketDialog �Ի���
//�����ļ�
#define HID_ONLINE_UPDATE		0x3000
#define HOU_START						(HID_ONLINE_UPDATE + 0x0)
#define HOU_SEND_DATA					(HID_ONLINE_UPDATE + 0x200)
#define HOU_GET_RESULT					(HID_ONLINE_UPDATE + 0x300)
#define HOU_UPDATE_UVCAPP				(HID_ONLINE_UPDATE + 0x110)
#define HOU_UPDATE_UVCAPP_ANTICOPY		(HID_ONLINE_UPDATE + 0x111)
#define HOU_UPDATE_UVCCONFIG			(HID_ONLINE_UPDATE + 0x112)
#define HOU_UPDATE_UVCATTR				(HID_ONLINE_UPDATE + 0x113)
#define HOU_UPDATE_SENSOR_DIRVER		(HID_ONLINE_UPDATE + 0x114)
#define HOU_UPDATE_SENSOR_SETTING		(HID_ONLINE_UPDATE + 0x115)
#define HOU_UPDATE_APP_INIT				(HID_ONLINE_UPDATE + 0x116)
#define HOU_UPDATE_FAST_INIT			(HID_ONLINE_UPDATE + 0x120)
#define HOU_UPDATE_ALL_INIT				(HID_ONLINE_UPDATE + 0x150)
#define HOU_UPDATE_HID_TEST				(HID_ONLINE_UPDATE + 0x1f0)
#define HOU_UPDATE_END					(HID_ONLINE_UPDATE + 0x400)
#pragma pack(1)
typedef struct def_hid_update_model
{
	unsigned short cmd;								//������ģ��
	unsigned long offset;							//��ǰģ�����ݵ�ƫ��
	unsigned long data_len;							//��ģ������ݳ���
}hid_update_model,*p_hid_update_model;
typedef struct def_hid_update_head_start
{
	unsigned int magic;								//hid�������ı�ʶ
	unsigned int pack_sum;							//�ļ����ܸ���
}hid_update_head_start,*p_hid_update_head_start;
struct PackAttribute
{
	unsigned int magic;								//hid�������ı�ʶ
	unsigned int pack_sum;							//�ļ����ܸ���
	hid_update_model hid_update_model[256];			//���ݰ�����Ϣ
	unsigned short crc;								//��ǰģ�������У��
};
#pragma pack()
class COpenPacketDialog : public CDialog
{
	DECLARE_DYNAMIC(COpenPacketDialog)

public:
	COpenPacketDialog(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~COpenPacketDialog();
	long WritePacketFromFile(char* packet_buffer,long offset,CString filePath);
	int CalculatePacketNum();
	CString m_cs_uvcconfig;
	CString m_cs_uvcattr;
	CString m_cs_uvcapp;
	CString m_cs_anticopy;
	CString m_cs_sensor_dirver;
	CString m_cs_sensor_settings;
	CString m_cs_app_init;
	CString m_cs_fast_init;
	CString m_cs_hid_test;
	PackAttribute m_PackAttribute;
	hid_update_model m_hid_update_model;
	hid_update_head_start m_hid_update_head_start;
	

// �Ի�������
	enum { IDD = IDD_DIALOG_OPEN_PACKET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonPacket();
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonOpenUvcattr();
	afx_msg void OnBnClickedButtonOpenUvcapp();
	afx_msg void OnBnClickedButtonOpenUvcapp2();
	afx_msg void OnBnClickedButtonOpenSensorDriver();
	afx_msg void OnBnClickedButtonOpenSensorSettings();
	afx_msg void OnBnClickedButtonOpenAppInit();
	afx_msg void OnBnClickedButtonOpenFastInit();
	afx_msg void OnBnClickedButtonOpenHidTest2();
	CEdit m_edit_uvcconfig;
	CEdit m_edit_uvcattr;
	CEdit m_edit_uvcapp;
	CEdit m_edit_anticopy;
	CEdit m_edit_sensor_dirver;
	CEdit m_edit_sensor_settings;
	CEdit m_edit_app_init;
	CEdit m_edit_fast_init;
	CEdit m_edit_hid_test;
	afx_msg void OnEnChangeEditPathSensorDriver();
};
