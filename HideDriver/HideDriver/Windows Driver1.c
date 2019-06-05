/*
ʱ�䣺2018��10��10�� 09:48:59
���ߣ�https://github.com/Sqdwr
ѧ������ϰ��Ϯ���ԣ�https://github.com/ZhuHuiBeiShaDiao
*/
#include <ntddk.h>
#include "GET_MIPROCESSLOADERENTRY.h"
#include "SYSTEM_MODULE_STRUCT.h"

BOOLEAN GetDriverObjectByName(PDRIVER_OBJECT *DriverObject, WCHAR *DriverName)
{
	PDRIVER_OBJECT TempObject = NULL;
	UNICODE_STRING u_DriverName = { 0 };
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	RtlInitUnicodeString(&u_DriverName, DriverName);
	Status = ObReferenceObjectByName(&u_DriverName, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, &TempObject);
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("��ȡ��������%wsʧ��!�������ǣ�%x!\n", Status));
		*DriverObject = NULL;
		return FALSE;
	}

	*DriverObject = TempObject;
	return TRUE;
}

BOOLEAN SupportSEH(PDRIVER_OBJECT DriverObject)
{
	//��Ϊ������������ժ��֮��Ͳ���֧��SEH��
	//������SEH�ַ��Ǹ��ݴ������ϻ�ȡ������ַ���ж��쳣�ĵ�ַ�Ƿ��ڸ�������
	//��Ϊ������û�ˣ��ͻ������
	//ѧϰ����Ϯ�����ķ������ñ��˵�����������������ϵĵ�ַ

	PDRIVER_OBJECT BeepDriverObject = NULL;;
	PLDR_DATA_TABLE_ENTRY LdrEntry = NULL;

	GetDriverObjectByName(&BeepDriverObject, L"\\Driver\\beep");
	if (BeepDriverObject == NULL)
		return FALSE;

	//MiProcessLoaderEntry��������ڲ������Ldr�е�DllBaseȻ��ȥRtlxRemoveInvertedFunctionTable�����ҵ���Ӧ����
	//֮�����Ƴ��������ݲ�������..�������û�е�DllBase��û������SEH������ԭ��û��...
	//����������ϵͳ��Driver\\beep��������...
	LdrEntry = (PLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;
	LdrEntry->DllBase = BeepDriverObject->DriverStart;
	ObDereferenceObject(BeepDriverObject);
	return TRUE;
}

VOID InitInLoadOrderLinks(PLDR_DATA_TABLE_ENTRY LdrEntry)
{
	InitializeListHead(&LdrEntry->InLoadOrderLinks);
	InitializeListHead(&LdrEntry->InMemoryOrderLinks);
}

VOID Reinitialize(PDRIVER_OBJECT DriverObject, PVOID Context, ULONG Count)
{
	MiProcessLoaderEntry m_MiProcessLoaderEntry = NULL;
	BOOLEAN bFlag = FALSE;
	ULONG *p = NULL;

	m_MiProcessLoaderEntry = Get_MiProcessLoaderEntry();
	if (m_MiProcessLoaderEntry == NULL)
		return;

	// bFlag = SupportSEH(DriverObject);

	m_MiProcessLoaderEntry(DriverObject->DriverSection, 0);
	InitInLoadOrderLinks((PLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection);

	DriverObject->DriverSection = NULL;
	DriverObject->DriverStart = NULL;
	DriverObject->DriverSize = NULL;
	DriverObject->DriverUnload = NULL;
	DriverObject->DriverInit = NULL;
	DriverObject->DeviceObject = NULL;

	/*if (bFlag)
	{
		__try {
			*p = 0x100;
		}
		__except (1)
		{
			KdPrint(("SEH��ȷ����\n"));
		}
	}*/
}

VOID Unload(PDRIVER_OBJECT DriverObject)
{
	KdPrint(("Unload Success!\n"));
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegString)
{
	KdPrint(("Entry Driver!\n"));
	IoRegisterDriverReinitialization(DriverObject, Reinitialize, NULL);
	DriverObject->DriverUnload = Unload;
	return STATUS_SUCCESS;
}
