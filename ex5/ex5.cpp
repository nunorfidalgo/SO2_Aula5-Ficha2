#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <time.h>

#define MAX 40

typedef struct {
	int lim_i, lim_s;
} dado;

TCHAR *palavra_a_pesquisar, *filename;

HANDLE	hMutex;
int nsearch, total = 0;


DWORD WINAPI Thread(LPVOID param);

void gotoxy(int x, int y);

int _tmain(int argc, LPTSTR argv[]) {
	int i, nthread, tam, n_bytes, N, c;
	FILE	*file;
	DWORD * threadId; //Id da thread a ser criada
	HANDLE * hT; //HANDLE ponteiro para cada thread a ser criada
	dado * arrayDados;
	DWORD inicio, fim;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	if (argc != 4) {
		_tprintf(TEXT("\nusage: nT nP <ficheiro>, nT -> numero threads, nP -> numero a pesquisar..."));
		return 1;
	}

	// tempo de execução inicial 
	inicio = GetTickCount();
	_tprintf(TEXT("\nTempo inicial: %d"), inicio);

	N = nthread = _ttoi(argv[1]);
	palavra_a_pesquisar = argv[2];
	_tprintf(TEXT("\nPalavra a pesquisar: %s"), palavra_a_pesquisar);
	filename = argv[3];
	_tprintf(TEXT("\nNome ficheiro: %s"), filename);

	arrayDados = (dado *)malloc(nthread * sizeof(dado));
	hT = (HANDLE *)malloc(nthread * sizeof(HANDLE));
	threadId = (DWORD *)malloc(nthread * sizeof(DWORD));

	// saber tamanho do ficheiro
	if ((_tfopen_s(&file, argv[3], TEXT("r"))) != 0) {
		_tprintf(TEXT("\nErro ao abrir ficheiro..."));
		return 1;
	}
	fseek(file, 0, SEEK_END);
	n_bytes = tam = ftell(file);
	fclose(file);
	_tprintf(TEXT("\nTamanho do ficheiro: %d bytes"), tam);

	// Criar as N threads
	_tprintf(TEXT("\nCriar as %d threads"), nthread);

	c = (n_bytes / N) + 1;

	for (i = 0; i < nthread; i++) {
		arrayDados[i].lim_i = i * c;
		if (i < N - 1)
			arrayDados[i].lim_s = (i + 1)*c;
		else
			arrayDados[i].lim_s = n_bytes;

		hT[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread, (LPVOID)&arrayDados[i], 0, &threadId[i]);
		if (hT[i] != NULL)
			_tprintf(TEXT("\nLancei uma thread[%d] com id %d."), i, threadId[i]);
		else
			_tprintf(TEXT("\nErro ao criar Thread\n"));
	}
	WaitForMultipleObjects(nthread, hT, 1, INFINITE);

	_tprintf(TEXT("\nTodas as threads terminaram\n"));
	for (int i = 0; i < nthread; i++)
		CloseHandle(hT[i]);



	//Libertar arrays dinâmicos
	free(arrayDados);
	free(hT);
	_tprintf(TEXT("[Thread Principal %d] Vou terminar..."), GetCurrentThreadId());
	fim = GetTickCount();
	_tprintf(TEXT("\nTempo final: %d milisegundos.\n"), fim - inicio);
	return 0;
}

/* ----------------------------------------------------- */
/* "Thread" - Função associada à Thread */
/* ----------------------------------------------------- */
DWORD WINAPI Thread(LPVOID param) {
	FILE *f;
	TCHAR linha[MAX];
	dado *ptrDado = (dado *)param;

	if (_tfopen_s(&f, filename, TEXT("r")) != 0) {
		_tprintf(TEXT("\n[Erro] Ficheiro %s não encontrado..."), filename);
		return -1;
	}

	//	Posicionar no inico do meu intervalo
	fseek(f, ptrDado->lim_i, SEEK_SET);

	while (_fgetts(linha, MAX, f) != NULL) {
		if (_tcsstr(linha, palavra_a_pesquisar) != NULL) {
			_tprintf(TEXT("\n[Thread %d] encontrou na linha : %s"), GetCurrentThreadId(), linha);
		}
		if (ftell(f) > ptrDado->lim_s) // Já cheguei ao meu limite
			break;
	}
	fclose(f);
	return 0;
}

void gotoxy(int x, int y) {
	static HANDLE hStdout = NULL;
	COORD coord;
	coord.X = x;
	coord.Y = y;
	if (!hStdout)
		hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hStdout, coord);
}