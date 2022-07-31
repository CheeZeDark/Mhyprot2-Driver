#include "events.h"
#include "comunication.h"

extern "C" {
/// <summary>
///		GetProcessNameByProcessID - M�todo respons�vel por retornar o nome de um processo a partir de seu PID
/// </summary>
/// <param name="PID">PID do processo/module</param>
/// <returns>Vamos retornar o nome do processo se ele existir | do contr�rio retornamos falso com a Macro do kernel API</returns>
char* GetProcessNameByProcessID(HANDLE PID) {
	//Vamos criar uma flag de estado para o retorno das API'S
	NTSTATUS status;
	// vamos inicializar uma estrutura da kernel usada para armazenar processo PE
	PEPROCESS EProcess = NULL;
	// A partir do PID � poss�vel encontrar o processo e recuperar informa��es da SSDT
	status = PsLookupProcessByProcessId(PID, &EProcess);
	// se o estado n�o for um sucesso
	if (!NT_SUCCESS(status))
	{
		// Vamos retornar falso para o chamador do m�todo
		return FALSE;
	}
	// Vamos desreferenciar o EPROCESS criado
	ObDereferenceObject(EProcess);
	// vamos obter o nome da imagem a partir de um extern da kernel e retornar um ponteiro para seu nome
	return (char*)PsGetProcessImageFileName(EProcess);
}


/// <summary>
///		KeMhYCallbacks - M�todo que vai interceptar os HANDLES
///		Podemos negar handles para opera��es de Leitura e Escrita na mem�ria de determinado processo, ou at� que ele seja encerrado
/// </summary>
/// <param name="ContextoDeRegistro">Contexto de Registro da callback</param>
/// <param name="pInformacoesDeOperacoes">Informa��es das opera��es da mem�ria</param>
/// <returns>OB_PREOP_SUCCESS estado de sucessso para m�todo sobreposto</returns>
OB_PREOP_CALLBACK_STATUS KeMhYCallbacks(PVOID ContextoDeRegistro, POB_PRE_OPERATION_INFORMATION pInformacoesDeOperacoes) {
	// Obtemos o PID do contexto do processo atual que foi enviado pelo sistema
	HANDLE PID = PsGetProcessId((PEPROCESS)pInformacoesDeOperacoes->Object);
	// Vamos definir um char array para armazenar o tamanho do processo e comparar posteriormente
	char szNomeDoProcesso[16] = { 0 };
	// Vamos desreferenciar o par�metro inutilizado
	UNREFERENCED_PARAMETER(ContextoDeRegistro);
	//Vamos copiar o nome do processo para nosso char array, usando o nosso m�todo para obter o nome a partir do seu PID
	strcpy(szNomeDoProcesso, GetProcessNameByProcessID(PID));
	//Vamos comparar se o nome do processo atual � do Jogo Genshin Impact
	if (!_stricmp(szNomeDoProcesso, "notepad.exe"))//"GenshinImpact.exe"
	{
		// Vamos verificar se a opera��o do sistema operacional � a de cria��o de HANDLES
		if (pInformacoesDeOperacoes->Operation == OB_OPERATION_HANDLE_CREATE)
		{
			// Vamos verificar se o estado do acesso foi para encerrar o processo
			if ((pInformacoesDeOperacoes->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_TERMINATE) == PROCESS_TERMINATE)
			{
				// vamos negar a permiss�o para encerrar um processo
				pInformacoesDeOperacoes->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_TERMINATE;
				DbgPrintEx(0, 0, "Foi detectado uma tentativa de encerrar nosso processo protegido !\n");
			}
			// Vamos verificar se o estado do acesso foi para uma opera��o de inicio
			if ((pInformacoesDeOperacoes->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_OPERATION) == PROCESS_VM_OPERATION)
			{
				// Vamos negar a permiss�o e n�o permitir que o processo seja iniciado
				pInformacoesDeOperacoes->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_OPERATION;
				DbgPrintEx(0, 0, "Foi detectado uma tentativa de in�cio do processo protegido !\n");
			}
			// Vamos verificar se o estado do acesso foi para readprocessmemory/fun��es de leitura de mem�ria de um processo
			if ((pInformacoesDeOperacoes->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_READ) == PROCESS_VM_READ)
			{
				// Vamos nengar a permiss�o de acesso para leitura do processo
				pInformacoesDeOperacoes->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_READ;
				DbgPrintEx(0, 0, "Foi detectado uma tentativa de leitura da mem�ria nosso processo protegido !\n");
			}
			// Vamos verificar se o estado do acesso foi para writeprocessmemory/fun��o de escrita na mem�ria do processo
			if ((pInformacoesDeOperacoes->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_WRITE) == PROCESS_VM_WRITE)
			{
				// Vamos negar a permiss�o de acesso para a escrita na mem�ria do processo
				pInformacoesDeOperacoes->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_WRITE;
				DbgPrintEx(0, 0, "Foi detectado uma tentativa de escrita na mem�ria do nosso processo protegido !\n");
				//Se alguem tentar escrever na mem�ria do jogo
				KeMhYTerminateProcess(PID);
			}
		}
	}
	// vamos retornar que a opera��es foi feita com sucesso para o callback
	return OB_PREOP_SUCCESS;
}


/// <summary>
///		KeMhYTerminateProcess - Encerra um processo no n�vel de usu�rio por meio do seu PID
/// </summary>
/// <param name="PID">PID(PROCESS ID) DO PROCESSO NO QUAL DESEJA-SE ENCERRAR</param>
/// <returns>RETORNAMOS UMA FLAG DA NTSTATUS, SENDO ELA DE SUCESSO OU DE ERRO</returns>
NTSTATUS KeMhYTerminateProcess(HANDLE PID) {
	DbgPrint("Tentando matar essa porra");
	// Criamos uma flag de NTSTATUS para armazenar nosso estado de retorno das winapi
	NTSTATUS status;
	// Criamos um PEPROCESS para guardar a refer�ncia do processo encontrado
	PEPROCESS Peprocess;
	// Criamos o process handle para armazenar o handle do nosso processo no qual queremos fechar
	HANDLE ProcessHandle;
	// Se Tivermos um PID
	if (PID) {
		// Vamos inicializar o PROCESSHANDLE
		ProcessHandle = NULL;
		// Vamos inicialiazr nosso PEPROCESS
		Peprocess = NULL;
		// Vamos procurar o processo usando o PID
		status = PsLookupProcessByProcessId(PID, &Peprocess);
		// Vamos verificar se Peprocess n�o � zero, se temos algo nele
		if (Peprocess) {
			// Vamos abrir o processo pelo PEPROCESS e assumir seu controle armazenando o resultado do novo HANDLE em ProcessHandle
			status = ObOpenObjectByPointer(Peprocess, 0, NULL, 0, NULL, 0, &ProcessHandle);
			// Vamos verifciar se n�o ocorreu um erro ao tentar obter o HANDLE do processo
			if (!NT_SUCCESS(status)) {
				// Se algo deu errado vamos retornar que ocorreu um erro e que n�o podemos trabalhar com o processo
				return STATUS_ERROR_PROCESS_NOT_IN_JOB;
			}
			else {
				// Vamos encerrar o processo pelo HANDLE que encontramos passando o c�digo zero para ele
				ZwTerminateProcess(ProcessHandle, 0);
				// Vamos fechar o HANDLE criado para o PROCESSO
				ZwClose(ProcessHandle);
				//Vamos desreferenciar nosso processo
				ObDereferenceObject(Peprocess);
			}
		}
	}
	// Vamos retornar o estado de sucesso para o chamados do m�todo
	return STATUS_SUCCESS;
}

/// <summary>
///		KeMhYEnumModulesOfProcess - Obtem o nome de um processo e seus modulos carregados da PEB a partir de um PID
/// </summary>
/// <param name="PID">PID(Process ID) do processo no qual deseja-se buscar</param>
/// <returns>Retorna a flag de sucesso ou erro NTSTATUS</returns>
NTSTATUS KeMhYEnumModulesOfProcess(HANDLE PID) {
	// Criamos um PEPROCESS para armazenar a refer�ncia do processo encontrado
	PEPROCESS peProcess = NULL;
	// Definimos uma flag de NTSTATUS para armazenar o retorno das API do sistema operacional
	NTSTATUS status;
	// SIZE_T DA PEB
	SIZE_T PEB = 0;
	// SIZE_T DA LDR
	SIZE_T LDR = 0;
	// NOSSA LISTA DE MODULOS COM CABE�ALHOS
	PLIST_ENTRY ModListHead = 0;
	// NOSSA LISTA DE MODULOS
	PLIST_ENTRY Module = 0;
	// NOSSO ESTADO
	KAPC_STATE ks;
	// CONTADOR DE MODULOS NA MEM�RIA
	int contagem_modulos = 0;
	// Char para armazenar o nome do processo
	char szNomeDoProcesso[20] = { 0 };
	// Obtendo o nome do processo pelo seu PID
	strcpy(szNomeDoProcesso, GetProcessNameByProcessID(PID));
	// Recuperando informa��es do PEPROCESS pelo PID
	status = PsLookupProcessByProcessId(PID, &peProcess);
	// Verificando se foi poss�vel recuperar dados
	if (!NT_SUCCESS(status)) {
		// retornando erro caso n�o tenha sido poss�vel recuperar dados
		return STATUS_ERROR_PROCESS_NOT_IN_JOB;
	}
	// DBGPRINT para mostrar o processo
	DbgPrintEx(0, 0, "Listando processos para: %s \n", szNomeDoProcesso);
	//Verificando se temos dados no PEPROCESS
	if (!peProcess) {
		// retornamos erro caso n�o tenha sido poss�vel recuperar os dados
		return STATUS_ERROR_PROCESS_NOT_IN_JOB;
	}
	// Recuperando informa��es da PEB usando a extern e o PEPROCESS
	PEB = (SIZE_T)PsGetProcessPeb(peProcess);
	// Verificando se temos algo na size_t da PEB
	if (!PEB) {
		// Se n�o tivermos nada retornamos erro
		return STATUS_ERROR_PROCESS_NOT_IN_JOB;
	}
	// Atacando a stack do processo e armazenando seu estado
	KeStackAttachProcess(peProcess, &ks);
	

	/// <summary>
	///		ATEN��O AQUI EM BAIXO ERA UMA ROTINA PROTEGIDA PELA TECNOLOGIA DE PACKING DO DRIVER, EU CONSEGUI RECONSTRUIR
	///		POREM � MUITO PROVAVEL QUE NO ORIGINAL ISSO SEJA COMPLETAMENTE DIFERENTE, POREM O COMPORTAMENTO � O MESMO.
	///		
	///		CR�DITOS:
	///			OBRIGADO A TODOS OS PESQUISADORES QUE POSTARAM PRINT DA ESTRUTURA IOTCL SEM ELA N�O SERIA POSS�VEL RECONSTRUIR, ERA NECESS�RIO ENTENDER O PROTOCOLO
	/// </summary>
	__try
	{
		// VAMOS OBTER A PEB
		LDR = PEB + (SIZE_T)0x018; // Offset LDR da PEB
		// LENDO
		ProbeForRead((CONST PVOID)LDR, 8, 8);
		// VAMOS OBTER O CABE�ALHO DO MODLISTHEAD
		ModListHead = (PLIST_ENTRY)(*(PULONG64)LDR + 0x010); //InLoadOrderModuleList
		// LENDO
		ProbeForRead((CONST PVOID)ModListHead, 8, 8);
		// DANDO UM FLINK E RECUPERANDO O MODULO
		Module = ModListHead->Flink;
		// ENQUANTO O MODULO FOR DIFERENTE DO FINAL
		while (ModListHead != Module) {
			//PRINTANDO NO DEBUGGER OS DADOS DOS MODULOS
			DbgPrintEx(0, 0, "Module Base: %p, Tamanho: %ld, Module Path: %wZ\n",
					   (PVOID)(((PLDR_DATA_TABLE_ENTRY)Module)->DllBase),
					   (ULONG)(((PLDR_DATA_TABLE_ENTRY)Module)->SizeOfImage),
					   &(((PLDR_DATA_TABLE_ENTRY)Module)->FullDllName));
			// DANDO FLINK PARA ANDAR PELOS MODULOS
			Module = Module->Flink;
			// INCREMENTANDO O CONTADOR
			contagem_modulos++;
			// LENDO
			ProbeForRead((CONST PVOID)Module, 80, 8);
		}
		// PRINTANDO NO DEBUGGER A QUANTIDADE DE M�DULOS CARREGADOS PELO PROCESSO
		DbgPrintEx(0, 0, "Quantidade de modulos: %d \n", contagem_modulos);
	}
	// VERIFCANDO EXCESS�ES
	__except (EXCEPTION_EXECUTE_HANDLER) 
	{
		// EXIBINDO ERRO DE LEITURA
		DbgPrintEx(0, 0, "N�o foi poss�vel ler a PEB");
	}
	// DANDI DETACH DA STACK
	KeUnstackDetachProcess(&ks);
	// RETORNANDO O ESTADO DE SUCESSO !
	return STATUS_SUCCESS;
}


/// <summary>
///		Esse m�todo buscara por padr�es em assinaturas de opcodes, baseado nos opcodes pegos em mem�ria e usando uma mascara
/// </summary>
/// <param name="ModuleBase">M�dule base do PE na mem�ria</param>
/// <param name="ModuleSize">Tamanho da imagem PE na mem�ria</param>
/// <param name="padrao">Padr�o const char*(macro de tipo definido byte), contendo o padr�o de opcodes da mem�ria</param>
/// <param name="mascara">Mascara para cada opcode da emm�ria, sendo substituido por ? os opcodes que variam</param>
/// <returns>STATUS_SUCCESS ou STATUS_UNSUCCESSFUL</returns>
NTSTATUS KdProcurarPadraoAssinatura(UINT64 ModuleBase, UINT64 ModuleSize, BYTE* padrao, char* mascara) {

	//Esse m�todo vai retorna true se o processo bater com o padr�o de opcodes e a macara
	if (KdencontrarAssinatura(ModuleBase, ModuleSize, padrao, mascara)) {
		DbgPrintEx(0, 0, "SUCESSO, FOI ENCONTRADO NESSE PROCESSO UM NOME ERRADO !!");
		return STATUS_SUCCESS; // Se for encontrado
	}
	else {
		DbgPrintEx(0, 0, "N�O FOI ENCONTRADO NADA NESSE PROCESSO !");
		return STATUS_UNSUCCESSFUL; // Se n�o for encontrado
	}

}

/// <summary>
///		Esse m�todo buscara por um processo usando seu PID
/// </summary>
/// <param name="PID">PID do processo</param>
/// <returns>PEPROCESS struct com a refer�ncia para o processo em quest�o</returns>
PEPROCESS KdObtenhaUmProcessoUsandoSeuPID(HANDLE PID) {
	PEPROCESS processo; //Criando a refer�ncia para um PEPROCESS struct
	if (NT_SUCCESS(PsLookupProcessByProcessId(PID, &processo))) { // RECUPERANDO A REFER�NCIA PARA O PEPROCESS
		return processo; // Retornando o PEPROCESS struct preenchida
	}
	else {
		return NULL; // Retornando NULL pois n�o foi encontrada a refer�ncia
	}
}

/*
	Esse m�todo verifica se existe um processo com aquele PID em execu��o
	retorna a flag status_sucess se o processo for encontrado no contexto do sistema operacional
*/
NTSTATUS KdExisteProcessoPID(PEPROCESS peprocess, HANDLE PID) { return PsLookupProcessByProcessId(PID, &peprocess); }


/*
	ImageLoadCallBack routine, essa rotina faz um hook no contexto de load de PE, DLL, etc
	e devolve informa��es �teis para tomada de decis�o pelo Driver
*/
PLOAD_IMAGE_NOTIFY_ROUTINE ImageLoadCallback(PUNICODE_STRING PE, HANDLE PID, PIMAGE_INFO PeInfo) {
	
	// Esse processo carregado � o processo que eu deveria proteger ?
	if (wcsstr(PE->Buffer, KdGetProcessBlackList())) {
	DbgPrint("PE: %ls", PE->Buffer); // Unicode posso usar o buffer para obter um PWCH
		KdSetBaseAddrRW(PeInfo->ImageBase); // Recuperamos o imageBase na mem�ria virtual
		KdSetPidAddrRW(PID); // Recuperamos o PID
	}
	UNREFERENCED_PARAMETER(PE); // Desreferenciamos as refer�ncias n�o usadas
	UNREFERENCED_PARAMETER(PID);  // Desreferenciamos as refer�ncias n�o usadas
	UNREFERENCED_PARAMETER(PeInfo);  // Desreferenciamos as refer�ncias n�o usadas
	return STATUS_SUCCESS; // Retornamos ao contexto do sistema a flag de sucesso
}

}