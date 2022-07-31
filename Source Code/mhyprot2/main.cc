#include "main.h"
#include "comunication.h"
#include "events.h"
OB_CALLBACK_REGISTRATION obRegistro = { 0 };
OB_OPERATION_REGISTRATION opRegistro = { 0 };
PVOID hCallbackHandle;
BOOLEAN obHooked = FALSE;
/////////////////////////////////////////////////////////////////


extern "C" {

	/// <summary>
	/// It's Driver Unload
	/// </summary>
	/// <param name="pDriverObj">Object to Register Driver/param>
	VOID DriverUnload(PDRIVER_OBJECT pDriverObj) {

		PsRemoveLoadImageNotifyRoutine(
			(PLOAD_IMAGE_NOTIFY_ROUTINE)ImageLoadCallback
		);

		//Vamos desreferenciar o par�metro inutilizado
		UNREFERENCED_PARAMETER(pDriverObj);
		//Vamos remover nosso callback da OB, usando o HANDLE retornado e armazenado em mem�ria quando o DriverEntry preparou a nossa fun��o
		if (obHooked) {
			ObUnRegisterCallbacks(hCallbackHandle);
			obHooked = FALSE;
		}

		IoDeleteSymbolicLink(&dos);
		IoDeleteDevice(pDriverObj->DeviceObject);
	}


	/// <summary>
	///		DriverEntry
	/// </summary>
	/// <param name="pDriverObj">Objeto de registro do Driver</param>
	/// <param name="pRegistryString">Path de registro do Driver</param>
	/// <returns>Retorna status_sucess j� ap�s o Driver ser carregado</returns>
	NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObj, PUNICODE_STRING pRegistryString) {

		// Criando uma flag de status para armazenar o retorno das syscalls
		NTSTATUS status;
		// Vamos definir o m�todo a ser executado quando encerrarmos nosso driver com a SC stop flag
		pDriverObj->DriverUnload = DriverUnload;
		// Vamos exibir um hello wolrd para mostrar que tudo ocorreu bem
		DbgPrintEx(0, 0, "Hello Mihoyo :D\n");

		/*
		PsSetLoadImageNotifyRoutine(
			(PLOAD_IMAGE_NOTIFY_ROUTINE)ImageLoadCallback
		);*/

		// Vamos preparar um callback para a ObRegisterCallbacks
		// Vamos obter a vers�o do filtro
		obRegistro.Version = ObGetFilterVersion();
		// Vamos definir um contador para nossa opera��o de registro
		obRegistro.OperationRegistrationCount = 1;
		// Vamos definir um registro de contexto nullo porque no contexto atual n�o vai ser usado
		obRegistro.RegistrationContext = NULL;
		// Vamos definir a altitude ao obregistre
		RtlInitUnicodeString(&obRegistro.Altitude, L"321000");

		// Definimos o tipo como tipo de processo que ser� interceptado
		opRegistro.ObjectType = PsProcessType;
		// Vamos definir as opera��es e permiss�es para nossa intercepta��o do callback, nesse contexto vamos interceptar
		// as HANDLE do sistema operacional ou seja quando um estado de handle para um processo definido vamos inteceptar negar
		// por exemplo podemos negar R/W memory ou encerrar um processo
		opRegistro.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
		// Vamos definir nossa fun��o de callback que vai inteceptar o contexto e informa��es dos processos
		opRegistro.PreOperation = KeMhYCallbacks;
		// Vamos preparar nossa opera��o de registro para as callbacks
		obRegistro.OperationRegistration = &opRegistro;
		// Vamos interceptar a obCallbacks com nossos novos dados e come�ar a usar nossa fun��o para interceptar os dados
		status = ObRegisterCallbacks(&obRegistro, &hCallbackHandle);
		// Vamos verificar se a nossa flag foi um sucesso e nossa callback foi criada com sucesso
		if (!NT_SUCCESS(status)) {
			//Quando ocorrer um erro na callback, a maioria dos erros pode ser por causa da verifica��o de integridade do /integritycheck
			DbgPrintEx(0, 0, "ERROR TO ALLOCATION MEMORY !\n");
		}
		else {
			obHooked = TRUE;
		}

		KdCreateDeviceSymbolLink(pDriverObj);

		KdScanProcesso("\\??\\C:\\Users\\Epic_Joao_1123\\Desktop\\Driver\\Debug\\mhyprot2.sys");

		// Vamos desreferenciar os par�metros que n�o estamos usando
		UNREFERENCED_PARAMETER(pRegistryString);
		UNREFERENCED_PARAMETER(pDriverObj);

		// Estado de sucesso para o inicializador da SC
		return STATUS_SUCCESS;
	}

}