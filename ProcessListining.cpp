#include <windows.h>
#include <psapi.h>
#include <tchar.h>
#include <iostream>
#include <vector>

struct ProcessInfo {
    DWORD processID = 0;
    std::wstring processName {};
    SIZE_T memoryAddress = 0;
};

// Função para pegar o nome do processo dado seu handle
std::wstring GetProcessName(HANDLE hProcess) {
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

    // Pega o nome do executável
    if (GetModuleBaseName(hProcess, nullptr, szProcessName, sizeof(szProcessName) / sizeof(TCHAR))) {
        return std::wstring(szProcessName);
    }

    return L"<unknown>";
}

// Função para listar processos
std::vector<ProcessInfo> ListProcesses() {
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    std::vector<ProcessInfo> processList;

    // Recebe a lista de IDs de processos
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        return processList; // Se falhar, retorna uma lista vazia
    }

    // Calcula o número de processos
    cProcesses = cbNeeded / sizeof(DWORD);

    // Itera por cada processo
    for (unsigned int i = 0; i < cProcesses; i++) {
        if (aProcesses[i] != 0) {
            // Abre o processo para leitura
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);

            if (hProcess != nullptr) {
                ProcessInfo pInfo;
                pInfo.processID = aProcesses[i];
                pInfo.processName = GetProcessName(hProcess);

                // Alocação de memória usada pelo processo
                PROCESS_MEMORY_COUNTERS pmc;
                if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                    pInfo.memoryAddress = pmc.WorkingSetSize;
                }

                processList.push_back(pInfo);

                // Fecha o handle do processo
                CloseHandle(hProcess);
            }
        }
    }

    return processList;
}

int main() {
    std::vector<ProcessInfo> processes = ListProcesses();

    // Imprime as informações
    std::wcout << L"PID\tNome do Processo\tAlocação de Memória\n";
    for (const auto& proc : processes) {
        std::wcout << proc.processID << L"\t" << proc.processName << L"\t" << proc.memoryAddress << L" bytes\n";
    }

    return 0;
}
