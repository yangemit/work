// OpenPacketDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "USB.h"
#include "OpenPacketDialog.h"
#include "afxdialogex.h"


// COpenPacketDialog 对话框

IMPLEMENT_DYNAMIC(COpenPacketDialog, CDialog)

COpenPacketDialog::COpenPacketDialog(CWnd* pParent /*=NULL*/)
	: CDialog(COpenPacketDialog::IDD, pParent)
{
}

COpenPacketDialog::~COpenPacketDialog()
{
}

void COpenPacketDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PATH, m_edit_uvcconfig);
	DDX_Control(pDX, IDC_EDIT_PATH_UVCATTR, m_edit_uvcattr);
	DDX_Control(pDX, IDC_EDIT_PATH_UVCAPP, m_edit_uvcapp);
	DDX_Control(pDX, IDC_EDIT_PATH_UVCAPP2, m_edit_anticopy);
	DDX_Control(pDX, IDC_EDIT_PATH_SENSOR_DRIVER, m_edit_sensor_dirver);
	DDX_Control(pDX, IDC_EDIT_PATH_SENSOTR_SETTING, m_edit_sensor_settings);
	DDX_Control(pDX, IDC_EDIT_PATH_SENSOTR_APP_INIT, m_edit_app_init);
	DDX_Control(pDX, IDC_EDIT_PATH_FAST_INIT2, m_edit_fast_init);
	DDX_Control(pDX, IDC_EDIT_PATH_HID_TEST2, m_edit_hid_test);
}


BEGIN_MESSAGE_MAP(COpenPacketDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_PACKET, &COpenPacketDialog::OnBnClickedButtonPacket)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &COpenPacketDialog::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_UVCATTR, &COpenPacketDialog::OnBnClickedButtonOpenUvcattr)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_UVCAPP, &COpenPacketDialog::OnBnClickedButtonOpenUvcapp)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_UVCAPP2, &COpenPacketDialog::OnBnClickedButtonOpenUvcapp2)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_SENSOR_DRIVER, &COpenPacketDialog::OnBnClickedButtonOpenSensorDriver)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_SENSOR_SETTINGS, &COpenPacketDialog::OnBnClickedButtonOpenSensorSettings)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_APP_INIT, &COpenPacketDialog::OnBnClickedButtonOpenAppInit)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_FAST_INIT, &COpenPacketDialog::OnBnClickedButtonOpenFastInit)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_HID_TEST2, &COpenPacketDialog::OnBnClickedButtonOpenHidTest2)
END_MESSAGE_MAP()


// COpenPacketDialog 消息处理程序

long COpenPacketDialog::WritePacketFromFile(char* packet_buffer,long offset,CString filePath)
{
	short rev = 1;	//一次实际读到的文件大小
	FILE *fw_fd;
	USES_CONVERSION;
	char* path = T2A(filePath.GetBuffer(0));
	filePath.ReleaseBuffer();
	if(fopen_s(&fw_fd,path,"rb") != 0)
	{
		AfxMessageBox(_T("打开文件失败!"));
	}
	long file_size;
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
	return file_size;
}

int COpenPacketDialog::CalculatePacketNum()
{
	int packet_num = 0;
	if(m_cs_uvcconfig != "")
	{
		packet_num++;
	}
	if(m_cs_uvcattr != "")
	{
		packet_num++;
	}
	if(m_cs_uvcapp != "")
	{
		packet_num++;
	}
	if(m_cs_anticopy != "")
	{
		packet_num++;
	}
	if(m_cs_sensor_dirver != "")
	{
		packet_num++;
	}
	if(m_cs_sensor_settings != "")
	{
		packet_num++;
	}
	if(m_cs_app_init != "")
	{
		packet_num++;
	}
	if(m_cs_fast_init != "")
	{
		packet_num++;
	}
	if(m_cs_hid_test != "")
	{
		packet_num++;
	}
	return packet_num;
}
void COpenPacketDialog::OnBnClickedButtonPacket()
{
	if(m_cs_uvcconfig == "" && m_cs_uvcattr == "" && m_cs_uvcapp == "" && \
		m_cs_anticopy == "" && m_cs_sensor_dirver == "" && m_cs_sensor_settings == "" &&\
		m_cs_app_init == "" && m_cs_fast_init == "" && m_cs_hid_test == "")
	{
		AfxMessageBox(_T("文件名不可以全部为空!"));
		return;
	}
	char *packet_buffer = (char*)malloc(10*1024*1024);
	memset(&m_hid_update_model,0,sizeof(hid_update_model));
	memset(&m_hid_update_head_start,0,sizeof(hid_update_head_start));
	m_hid_update_head_start.magic = 0x5a5a;
	m_hid_update_head_start.pack_sum = CalculatePacketNum();

	int size_update_mode_head = sizeof(hid_update_model);
	int size_hid_update_head_start = sizeof(hid_update_head_start);

	memcpy(packet_buffer,&m_hid_update_head_start,size_hid_update_head_start);
	long offset = size_hid_update_head_start + size_update_mode_head;
	long file_size = 0;
	/*if(m_cs_uvcconfig != "")
	{
		file_size = WritePacketFromFile(packet_buffer,offset,m_cs_uvcconfig);
		m_hid_update_model.cmd = HOU_UPDATE_UVCCONFIG;
		m_hid_update_model.offset = offset;
		m_hid_update_model.data_len = file_size;
		memcpy(packet_buffer+size_hid_update_head_start,&m_hid_update_model,size_update_mode_head);
		offset += file_size;
	}
	if(m_cs_uvcattr != "")
	{
		file_size = WritePacketFromFile(packet_buffer,offset + size_update_mode_head,m_cs_uvcattr);
		memset(&m_hid_update_model,0,sizeof(hid_update_model));
		m_hid_update_model.cmd = HOU_UPDATE_UVCATTR;
		m_hid_update_model.offset = offset;
		m_hid_update_model.data_len = file_size;
		memcpy(packet_buffer+offset,&m_hid_update_model,size_update_mode_head);
		offset += file_size;
		offset += size_update_mode_head;
	}
	if(m_cs_uvcapp != "")
	{
		file_size = WritePacketFromFile(packet_buffer,offset + size_update_mode_head,m_cs_uvcapp);
		memset(&m_hid_update_model,0,sizeof(hid_update_model));
		m_hid_update_model.cmd = HOU_UPDATE_UVCAPP;
		m_hid_update_model.offset = offset;
		m_hid_update_model.data_len = file_size;
		memcpy(packet_buffer+offset,&m_hid_update_model,size_update_mode_head);
		offset += file_size;
		offset += size_update_mode_head;
	}
	if(m_cs_anticopy != "")
	{
		file_size = WritePacketFromFile(packet_buffer,offset + size_update_mode_head,m_cs_anticopy);
		memset(&m_hid_update_model,0,sizeof(hid_update_model));
		m_hid_update_model.cmd = HOU_UPDATE_UVCAPP_ANTICOPY;
		m_hid_update_model.offset = offset;
		m_hid_update_model.data_len = file_size;
		memcpy(packet_buffer+offset,&m_hid_update_model,size_update_mode_head);
		offset += file_size;
		offset += size_update_mode_head;
	}
	if(m_cs_sensor_dirver != "")
	{
		file_size = WritePacketFromFile(packet_buffer,offset + size_update_mode_head,m_cs_sensor_dirver);
		memset(&m_hid_update_model,0,sizeof(hid_update_model));
		m_hid_update_model.cmd = HOU_UPDATE_SENSOR_DIRVER;
		m_hid_update_model.offset = offset;
		m_hid_update_model.data_len = file_size;
		memcpy(packet_buffer+offset,&m_hid_update_model,size_update_mode_head);
		offset += file_size;
		offset += size_update_mode_head;
	}
	if(m_cs_sensor_settings != "")
	{
		file_size = WritePacketFromFile(packet_buffer,offset + size_update_mode_head,m_cs_sensor_settings);
		memset(&m_hid_update_model,0,sizeof(hid_update_model));
		m_hid_update_model.cmd = HOU_UPDATE_SENSOR_SETTING;
		m_hid_update_model.offset = offset;
		m_hid_update_model.data_len = file_size;
		memcpy(packet_buffer+offset,&m_hid_update_model,size_update_mode_head);
		offset += file_size;
		offset += size_update_mode_head;
	}
	if(m_cs_app_init != "")
	{
		file_size = WritePacketFromFile(packet_buffer,offset + size_update_mode_head,m_cs_app_init);
		memset(&m_hid_update_model,0,sizeof(hid_update_model));
		m_hid_update_model.cmd = HOU_UPDATE_APP_INIT;
		m_hid_update_model.offset = offset;
		m_hid_update_model.data_len = file_size;
		memcpy(packet_buffer+offset,&m_hid_update_model,size_update_mode_head);
		offset += file_size;
		offset += size_update_mode_head;
	}
	if(m_cs_fast_init != "")
	{
		file_size = WritePacketFromFile(packet_buffer,offset + size_update_mode_head,m_cs_fast_init);
		memset(&m_hid_update_model,0,sizeof(hid_update_model));
		m_hid_update_model.cmd = HOU_UPDATE_FAST_INIT;
		m_hid_update_model.offset = offset;
		m_hid_update_model.data_len = file_size;
		memcpy(packet_buffer+offset,&m_hid_update_model,size_update_mode_head);
		offset += file_size;
		offset += size_update_mode_head;
	}
	if(m_cs_hid_test != "")
	{
		file_size = WritePacketFromFile(packet_buffer,offset + size_update_mode_head,m_cs_hid_test);
		memset(&m_hid_update_model,0,sizeof(hid_update_model));
		m_hid_update_model.cmd = HOU_UPDATE_HID_TEST;
		m_hid_update_model.offset = offset;
		m_hid_update_model.data_len = file_size;
		memcpy(packet_buffer+offset,&m_hid_update_model,size_update_mode_head);
		offset += file_size;
		offset += size_update_mode_head;
	}*/

	if(m_cs_uvcconfig != "")
	{
		file_size = WritePacketFromFile(packet_buffer,offset,m_cs_uvcconfig);
		m_hid_update_model.cmd = HOU_UPDATE_UVCCONFIG;
		m_hid_update_model.offset = offset;
		m_hid_update_model.data_len = file_size;
		memcpy(packet_buffer+size_hid_update_head_start,&m_hid_update_model,size_update_mode_head);
		offset += file_size;
	}
	if(m_cs_uvcattr != "")
	{
		if(m_cs_uvcconfig == "")
		{
			file_size = WritePacketFromFile(packet_buffer,offset,m_cs_uvcattr);
			m_hid_update_model.cmd = HOU_UPDATE_UVCATTR;
			m_hid_update_model.offset = offset;
			m_hid_update_model.data_len = file_size;
			memcpy(packet_buffer+size_hid_update_head_start,&m_hid_update_model,size_update_mode_head);
			offset += file_size;
		}
		else
		{
			file_size = WritePacketFromFile(packet_buffer,offset + size_update_mode_head,m_cs_uvcattr);
			memset(&m_hid_update_model,0,sizeof(hid_update_model));
			m_hid_update_model.cmd = HOU_UPDATE_UVCATTR;
			m_hid_update_model.offset = offset;
			m_hid_update_model.data_len = file_size;
			memcpy(packet_buffer+offset,&m_hid_update_model,size_update_mode_head);
			offset += file_size;
			offset += size_update_mode_head;
		}
	}
	if(m_cs_hid_test != "")
	{
		if(m_cs_uvcconfig == "" && m_cs_uvcattr == "")
		{
			file_size = WritePacketFromFile(packet_buffer,offset,m_cs_hid_test);
			m_hid_update_model.cmd = HOU_UPDATE_HID_TEST;
			m_hid_update_model.offset = offset;
			m_hid_update_model.data_len = file_size;
			memcpy(packet_buffer+size_hid_update_head_start,&m_hid_update_model,size_update_mode_head);
			offset += file_size;
		}
		else
		{
			file_size = WritePacketFromFile(packet_buffer,offset + size_update_mode_head,m_cs_hid_test);
			memset(&m_hid_update_model,0,sizeof(hid_update_model));
			m_hid_update_model.cmd = HOU_UPDATE_HID_TEST;
			m_hid_update_model.offset = offset;
			m_hid_update_model.data_len = file_size;
			memcpy(packet_buffer+offset,&m_hid_update_model,size_update_mode_head);
			offset += file_size;
			offset += size_update_mode_head;
		}
	}
	if(m_cs_anticopy != "")
	{
		if(m_cs_uvcconfig == "" && m_cs_uvcattr == "" && m_cs_hid_test == "")
		{
			file_size = WritePacketFromFile(packet_buffer,offset,m_cs_anticopy);
			m_hid_update_model.cmd = HOU_UPDATE_UVCAPP_ANTICOPY;
			m_hid_update_model.offset = offset;
			m_hid_update_model.data_len = file_size;
			memcpy(packet_buffer+size_hid_update_head_start,&m_hid_update_model,size_update_mode_head);
			offset += file_size;
		}
		else
		{
			file_size = WritePacketFromFile(packet_buffer,offset + size_update_mode_head,m_cs_anticopy);
			memset(&m_hid_update_model,0,sizeof(hid_update_model));
			m_hid_update_model.cmd = HOU_UPDATE_UVCAPP_ANTICOPY;
			m_hid_update_model.offset = offset;
			m_hid_update_model.data_len = file_size;
			memcpy(packet_buffer+offset,&m_hid_update_model,size_update_mode_head);
			offset += file_size;
			offset += size_update_mode_head;
		}
	}
	if(m_cs_sensor_dirver != "")
	{
		if(m_cs_uvcconfig == "" && m_cs_uvcattr == "" && m_cs_hid_test == ""\
			&& m_cs_anticopy == "")
		{
			file_size = WritePacketFromFile(packet_buffer,offset,m_cs_sensor_dirver);
			m_hid_update_model.cmd = HOU_UPDATE_SENSOR_DIRVER;
			m_hid_update_model.offset = offset;
			m_hid_update_model.data_len = file_size;
			memcpy(packet_buffer+size_hid_update_head_start,&m_hid_update_model,size_update_mode_head);
			offset += file_size;
		}
		else
		{
			file_size = WritePacketFromFile(packet_buffer,offset + size_update_mode_head,m_cs_sensor_dirver);
			memset(&m_hid_update_model,0,sizeof(hid_update_model));
			m_hid_update_model.cmd = HOU_UPDATE_SENSOR_DIRVER;
			m_hid_update_model.offset = offset;
			m_hid_update_model.data_len = file_size;
			memcpy(packet_buffer+offset,&m_hid_update_model,size_update_mode_head);
			offset += file_size;
			offset += size_update_mode_head;
		}
	}
	if(m_cs_sensor_settings != "")
	{
		if(m_cs_uvcconfig == "" && m_cs_uvcattr == "" && m_cs_hid_test == ""\
			&& m_cs_anticopy == "" && m_cs_sensor_dirver == "")
		{
			file_size = WritePacketFromFile(packet_buffer,offset,m_cs_sensor_settings);
			m_hid_update_model.cmd = HOU_UPDATE_SENSOR_SETTING;
			m_hid_update_model.offset = offset;
			m_hid_update_model.data_len = file_size;
			memcpy(packet_buffer+size_hid_update_head_start,&m_hid_update_model,size_update_mode_head);
			offset += file_size;
		}
		else
		{
			file_size = WritePacketFromFile(packet_buffer,offset + size_update_mode_head,m_cs_sensor_settings);
			memset(&m_hid_update_model,0,sizeof(hid_update_model));
			m_hid_update_model.cmd = HOU_UPDATE_SENSOR_SETTING;
			m_hid_update_model.offset = offset;
			m_hid_update_model.data_len = file_size;
			memcpy(packet_buffer+offset,&m_hid_update_model,size_update_mode_head);
			offset += file_size;
			offset += size_update_mode_head;
		}
	}
	if(m_cs_app_init != "")
	{
		if(m_cs_uvcconfig == "" && m_cs_uvcattr == "" && m_cs_hid_test == ""\
			&& m_cs_anticopy == "" && m_cs_sensor_dirver == "" && m_cs_sensor_settings == "")
		{
			file_size = WritePacketFromFile(packet_buffer,offset,m_cs_app_init);
			m_hid_update_model.cmd = HOU_UPDATE_APP_INIT;
			m_hid_update_model.offset = offset;
			m_hid_update_model.data_len = file_size;
			memcpy(packet_buffer+size_hid_update_head_start,&m_hid_update_model,size_update_mode_head);
			offset += file_size;
		}
		else
		{
			file_size = WritePacketFromFile(packet_buffer,offset + size_update_mode_head,m_cs_app_init);
			memset(&m_hid_update_model,0,sizeof(hid_update_model));
			m_hid_update_model.cmd = HOU_UPDATE_APP_INIT;
			m_hid_update_model.offset = offset;
			m_hid_update_model.data_len = file_size;
			memcpy(packet_buffer+offset,&m_hid_update_model,size_update_mode_head);
			offset += file_size;
			offset += size_update_mode_head;
		}
	}
	if(m_cs_fast_init != "")
	{
		if(m_cs_uvcconfig == "" && m_cs_uvcattr == "" && m_cs_hid_test == ""\
			&& m_cs_anticopy == "" && m_cs_sensor_dirver == "" && m_cs_sensor_settings == ""\
			&& m_cs_app_init == "")
		{
			file_size = WritePacketFromFile(packet_buffer,offset,m_cs_fast_init);
			m_hid_update_model.cmd = HOU_UPDATE_FAST_INIT;
			m_hid_update_model.offset = offset;
			m_hid_update_model.data_len = file_size;
			memcpy(packet_buffer+size_hid_update_head_start,&m_hid_update_model,size_update_mode_head);
			offset += file_size;
		}
		else
		{
			file_size = WritePacketFromFile(packet_buffer,offset + size_update_mode_head,m_cs_fast_init);
			memset(&m_hid_update_model,0,sizeof(hid_update_model));
			m_hid_update_model.cmd = HOU_UPDATE_FAST_INIT;
			m_hid_update_model.offset = offset;
			m_hid_update_model.data_len = file_size;
			memcpy(packet_buffer+offset,&m_hid_update_model,size_update_mode_head);
			offset += file_size;
			offset += size_update_mode_head;
		}
	}
	if(m_cs_uvcapp != "")
	{
		if(m_cs_uvcconfig == "" && m_cs_uvcattr == "" && m_cs_hid_test == ""\
			&& m_cs_anticopy == "" && m_cs_sensor_dirver == "" && m_cs_sensor_settings == ""\
			&& m_cs_app_init == "" && m_cs_fast_init == "")
		{
			file_size = WritePacketFromFile(packet_buffer,offset,m_cs_uvcapp);
			m_hid_update_model.cmd = HOU_UPDATE_UVCAPP;
			m_hid_update_model.offset = offset;
			m_hid_update_model.data_len = file_size;
			memcpy(packet_buffer+size_hid_update_head_start,&m_hid_update_model,size_update_mode_head);
			offset += file_size;
		}
		else
		{
			file_size = WritePacketFromFile(packet_buffer,offset + size_update_mode_head,m_cs_uvcapp);
			memset(&m_hid_update_model,0,sizeof(hid_update_model));
			m_hid_update_model.cmd = HOU_UPDATE_UVCAPP;
			m_hid_update_model.offset = offset;
			m_hid_update_model.data_len = file_size;
			memcpy(packet_buffer+offset,&m_hid_update_model,size_update_mode_head);
			offset += file_size;
			offset += size_update_mode_head;
		}
	}
	FILE *fp;
	if(fopen_s(&fp,".\\packet.bin","wb+") != 0)
	{
		AfxMessageBox(_T("打开文件失败!"));
		return;
	}
	fwrite(packet_buffer, offset, 1, fp);
    fclose(fp);
	free(packet_buffer);
	packet_buffer = NULL;
	AfxMessageBox(_T("文件组包packet.bin成功，文件在当前目录!"));
}


void COpenPacketDialog::OnBnClickedButtonOpen()
{
	// TODO: 在此添加控件通知处理程序代码
	CString defaultDir = L"E:\\FileTest";
	CString fileName = L"";
	TCHAR filter[] = _T("文本文件(*.txt)||所有文件(*.*)||");
	CFileDialog openFileDlg(TRUE, defaultDir, fileName, OFN_HIDEREADONLY|OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\test.doc";
	INT_PTR result = openFileDlg.DoModal();
	if(result == IDOK) {
		m_cs_uvcconfig = openFileDlg.GetPathName();
		m_edit_uvcconfig.SetWindowTextW(m_cs_uvcconfig);
	}
}


void COpenPacketDialog::OnBnClickedButtonOpenUvcattr()
{
	// TODO: 在此添加控件通知处理程序代码
	CString defaultDir = L"E:\\FileTest";
	CString fileName = L"";
	TCHAR filter[] = _T("文本文件(*.txt)||所有文件(*.*)||");
	CFileDialog openFileDlg(TRUE, defaultDir, fileName, OFN_HIDEREADONLY|OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\test.doc";
	INT_PTR result = openFileDlg.DoModal();
	if(result == IDOK) {
		m_cs_uvcattr = openFileDlg.GetPathName();
		m_edit_uvcattr.SetWindowTextW(m_cs_uvcattr);
	}
}


void COpenPacketDialog::OnBnClickedButtonOpenUvcapp()
{
	// TODO: 在此添加控件通知处理程序代码
	CString defaultDir = L"E:\\FileTest";
	CString fileName = L"";
	TCHAR filter[] = _T("文本文件(*.txt)||所有文件(*.*)||");
	CFileDialog openFileDlg(TRUE, defaultDir, fileName, OFN_HIDEREADONLY|OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\test.doc";
	INT_PTR result = openFileDlg.DoModal();
	if(result == IDOK) {
		m_cs_uvcapp = openFileDlg.GetPathName();
		m_edit_uvcapp.SetWindowTextW(m_cs_uvcapp);
	}
}


void COpenPacketDialog::OnBnClickedButtonOpenUvcapp2()
{
	// TODO: 在此添加控件通知处理程序代码
	CString defaultDir = L"E:\\FileTest";
	CString fileName = L"";
	TCHAR filter[] = _T("文本文件(*.txt)||所有文件(*.*)||");
	CFileDialog openFileDlg(TRUE, defaultDir, fileName, OFN_HIDEREADONLY|OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\test.doc";
	INT_PTR result = openFileDlg.DoModal();
	if(result == IDOK) {
		m_cs_anticopy = openFileDlg.GetPathName();
		m_edit_anticopy.SetWindowTextW(m_cs_anticopy);
	}
}


void COpenPacketDialog::OnBnClickedButtonOpenSensorDriver()
{
	// TODO: 在此添加控件通知处理程序代码
	CString defaultDir = L"E:\\FileTest";
	CString fileName = L"";
	TCHAR filter[] = _T("文本文件(*.txt)||所有文件(*.*)||");
	CFileDialog openFileDlg(TRUE, defaultDir, fileName, OFN_HIDEREADONLY|OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\test.doc";
	INT_PTR result = openFileDlg.DoModal();
	if(result == IDOK) {
		m_cs_sensor_dirver = openFileDlg.GetPathName();
		m_edit_sensor_dirver.SetWindowTextW(m_cs_sensor_dirver);
	}
}


void COpenPacketDialog::OnBnClickedButtonOpenSensorSettings()
{
	// TODO: 在此添加控件通知处理程序代码
	CString defaultDir = L"E:\\FileTest";
	CString fileName = L"";
	TCHAR filter[] = _T("文本文件(*.txt)||所有文件(*.*)||");
	CFileDialog openFileDlg(TRUE, defaultDir, fileName, OFN_HIDEREADONLY|OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\test.doc";
	INT_PTR result = openFileDlg.DoModal();
	if(result == IDOK) {
		m_cs_sensor_settings = openFileDlg.GetPathName();
		m_edit_sensor_settings.SetWindowTextW(m_cs_sensor_settings);
	}
}


void COpenPacketDialog::OnBnClickedButtonOpenAppInit()
{
	// TODO: 在此添加控件通知处理程序代码
	CString defaultDir = L"E:\\FileTest";
	CString fileName = L"";
	TCHAR filter[] = _T("文本文件(*.txt)||所有文件(*.*)||");
	CFileDialog openFileDlg(TRUE, defaultDir, fileName, OFN_HIDEREADONLY|OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\test.doc";
	INT_PTR result = openFileDlg.DoModal();
	if(result == IDOK) {
		m_cs_app_init = openFileDlg.GetPathName();
		m_edit_app_init.SetWindowTextW(m_cs_app_init);
	}
}


void COpenPacketDialog::OnBnClickedButtonOpenFastInit()
{
	// TODO: 在此添加控件通知处理程序代码
	CString defaultDir = L"E:\\FileTest";
	CString fileName = L"";
	TCHAR filter[] = _T("文本文件(*.txt)||所有文件(*.*)||");
	CFileDialog openFileDlg(TRUE, defaultDir, fileName, OFN_HIDEREADONLY|OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\test.doc";
	INT_PTR result = openFileDlg.DoModal();
	if(result == IDOK) {
		m_cs_fast_init = openFileDlg.GetPathName();
		m_edit_fast_init.SetWindowTextW(m_cs_fast_init);
	}
}


void COpenPacketDialog::OnBnClickedButtonOpenHidTest2()
{
	// TODO: 在此添加控件通知处理程序代码
	CString defaultDir = L"E:\\FileTest";
	CString fileName = L"";
	TCHAR filter[] = _T("文本文件(*.txt)||所有文件(*.*)||");
	CFileDialog openFileDlg(TRUE, defaultDir, fileName, OFN_HIDEREADONLY|OFN_READONLY, filter, NULL);
	openFileDlg.GetOFN().lpstrInitialDir = L"E:\\FileTest\\test.doc";
	INT_PTR result = openFileDlg.DoModal();
	if(result == IDOK) {
		m_cs_hid_test = openFileDlg.GetPathName();
		m_edit_hid_test.SetWindowTextW(m_cs_hid_test);
	}
}
