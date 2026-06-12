#include <vector>
#include <string>
#include <windows.h>
#include <windowsx.h>
#include <richedit.h>
#include <commctrl.h>
#include <shellapi.h>
#include <tchar.h>
#include <stdio.h>
#include "PluginInterface.h"
#include "Scintilla.h"
#include <Python.h>
#include <direct.h> 
#include <fstream>
#include <sstream>
#include <regex>
#include <thread>

// Identifiants uniques pour less composants graphiques
#define IDC_STATIC -1


// Définitions des ID 

#define IDC_KEY_INPUT    1001
#define IDC_MODEL_COMBO  1002
#define IDC_BTN_SAVE     1003
#define IDC_BTN_REFRESH  1004
#define IDC_KEY_TOGGLE   1005

// --- IDs BOITE CONSOLE ---
#define IDD_CONSOLE_PANEL  2000
#define IDC_CONSOLE_EDIT   2001
#define IDC_CONSOLE_INPUT  2002
#define IDC_CONSOLE_PROMPT 2003

// --- IDs FORMULAIRE NEW SCRIPT ---
#define IDD_NEW_SCRIPT_DLG   2010
#define IDC_EDIT_LOCAL_NAME  2011
#define IDC_EDIT_PROVIDER    2012
#define IDC_EDIT_API_URL     2013
#define IDC_EDIT_MODEL_URL   2014
#define IDC_EDIT_REG_URL     2015
#define IDC_STATIC_REG_URL   2016
#define IDC_EDIT_API_KEY     2017
#define IDC_STATIC_NAME      2018
#define IDC_STATIC_PROVIDER  2019
#define IDC_STATIC_API_URL   2020
#define IDC_STATIC_MODEL_URL 2021
#define IDC_STATIC_API_KEY   2022
#define IDOK_DO_SCRIPT       2023

// --- IDs de la fenetre liste/edition scripts utilisateur ---
#define IDD_MANAGE_SCRIPTS    2024
#define IDC_LIST_SCRIPTS      2025
#define IDC_BTN_EDIT_SCRIPT   2026
#define IDC_BTN_DELETE_SCRIPT 2027
#define IDC_BTN_CREATE_SCRIPT 2028

// --- ID Aide ---
#define IDD_HELP_DLG         2030
#define IDC_HELP_RICHEDIT    2031
#define IDC_HELP_BTN_OK      2032

//--- ID VERSIONINFO ---
#define IDC_VERSION_TEXT     2100
#define IDD_VERSION          2101

// --- IDENTIFIANTS BOÎTE CONFIGURATION ---
#define IDD_MAIN_CONFIG_DLG     2200
#define IDC_CHK_CONSOLE         2201
#define IDC_CHK_FORCECLOSE      2202
#define IDC_CMB_ACTIVE_API      2203
#define IDC_BTN_MANAGE_API      2204
#define IDC_STATIC_API_PROVIDER 2205

// --- IDENTIFIANTS BOÎTE QUESTION SUR SELECTION ---
#define IDD_PROMPT_IA       2300
#define IDC_PROMPT_TEXT     2301
#define IDC_CHK_SAVE_LOG    2302
#define IDC_STATIC_ASK_IA   2303

// --- IDENTIFIANTS BOÎTE CLÉ MANQUANTE ---
#define IDD_SETUP_API_DLG     2500
#define IDC_SETUP_API_LINK    2501
#define IDC_SETUP_API_KEY     2502

// --- IDENTIFIANTS BOÎTE DE CHAT ---
#define IDD_CHAT_DLG                3000
#define IDC_CHAT_STATIC_MODEL       3001
#define IDC_CHAT_MODEL_COMBO        3002
#define IDC_CHAT_HISTORY            3003
#define IDC_CHAT_INPUT              3004
#define IDC_BTN_SEND                3005
#define IDC_BTN_CLEAR               3006
#define IDC_CHK_SAVE_CHAT           3007

#ifndef ODS_HOT
    #define ODS_HOT 0x0010
#endif

// CONSOLE MENU CONTEXTUEL
#define ID_CONSOLE_COPY       40001
#define ID_CONSOLE_SELECTALL  40002
#define ID_CONSOLE_CLEAR      40003

#define IDM_FILE_CLOSE        41003


#ifndef NPPM_CLOSEFILE
    #define NPPM_CLOSEFILE (WM_USER + 1022)
#endif


#define NPPMSG                 (WM_USER + 1000)
#define NPPM_DMMSHOW           (NPPMSG + 30) // 2054
#define NPPM_DMMHIDE           (NPPMSG + 31) // 2055
#define NPPM_DMMREGASDCKDLG    (NPPMSG + 33) // 2057 
#define NPPM_SETMENUITEMCHECK  (NPPMSG + 40) // 2064 

#define DWS_DF_CONT_LEFT       1
#define DWS_DF_CONT_RIGHT      2
#define DWS_DF_CONT_TOP        4
#define DWS_DF_CONT_BOTTOM     8
#define DWS_ICONTAB            16
#define DWS_ICONBAR            32



#define IDM_VIEW_GOTO_ANOTHER_VIEW 10001  

#ifndef TBDATA_DEFINED
    #define TBDATA_DEFINED
    typedef struct {
        HWND hClient;
        const wchar_t *pszName;
        int dlgID;
        UINT uMask;
        HICON hIconTab;
        const wchar_t *pszAddInfo;
        RECT rcFloat;
        int iPrevCont;
        const wchar_t *pszModuleName; //  ModuleName (Requis par Notepad++)
    } tTbData;
#endif

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "shell32.lib")   // ← ajouter cette ligne

// Variables globales
int g_cmdIdPoserQuestion = 0; // Stockera l'ID de votre commande généré par Notepad++
bool g_isScintillaContextMenu = false;

// Variable globale pour stocker l'instance de la DLL
HINSTANCE hInst = NULL;
HINSTANCE g_hInst = NULL;
HINSTANCE g_hInstance = NULL;

HWND g_hConsole = NULL;
HWND g_hChatDlg = NULL;

HMODULE g_hModule = NULL;
HMODULE g_hRichEditLib = NULL;
HMODULE hRichEditLib = LoadLibraryW(L"riched20.dll");

HBRUSH g_hBrushFond = NULL;
HBRUSH g_hBrushControle = NULL; // fond du contrôle IDC_VERSION_TEXT

bool g_ConsoleVisible = false;       // Suivi propre de la visibilité
bool g_bForceCloseOnDelete = false;
wchar_t g_pluginDir[MAX_PATH] = {0};

void AppendRtfToRichEdit(HWND hRichEdit, const std::string& rtfContent);
void ToggleConsole();
void AppendTextToConsoleW(const wchar_t* wText);
void AppendTextToConsole(const char* text);
void LogToConsole(const wchar_t* message);
void CheckFirstRunAndSetup();

// Les fonctions de notre menu
void CreerNouveauScript();
void GererScriptsUtilisateur();
void LancerAssistant();
void lancerAssistantConfig();
void ToggleConsole(); 
void OuvrirConfiguration();
void AfficherAide();
void AfficherVersion();
// declare fonction entrée menu + menu contextuel
void ActionContextuelIA();

// --- PROTOTYPES POUR LE CHAT FLOTTANT ---
INT_PTR CALLBACK DlgProcChat(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void AppendTextToCustomEdit(HWND hEdit, const wchar_t* wText);

static NppData nppData;

static HMODULE hPythonModule = NULL;

//  menu
static FuncItem funcFunc[8];         

#ifndef NPPM_GETCURRENTSCI
    #define NPPM_GETCURRENTSCI (WM_USER + 1000 + 43)
#endif


struct PromptData {
    std::string prompt;
    HWND hParent;
};

// Variable globale temporaire pour stocker la question de l'utilisateur
std::wstring g_LangFilePath = L""; // Contiendra le chemin complet vers le .ini
std::string g_ActiveLang = "french";   // Langue par défaut
std::string MarkdownToRtfCPlusPlus(std::string text, bool singledoc = true);
bool g_bSaveSelectionLog = true;

// Conversion de WChar vers String (UTF-8)
std::string WstrToUtf8(const wchar_t* wstr) {
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    std::string str(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], size, NULL, NULL);
    return str;
} 

// Conversion wchar_t* → RTF (via Markdown) pour AppendRtfToRichEdit
// Utilisable partout dans le fichier
// Ajout du paramètre isFragment, activé par défaut pour le chat
std::string WcharToRtf(const wchar_t* wstr, bool isFragment = true) {
    // Utilisation de ta fonction existante WstrToUtf8
    return MarkdownToRtfCPlusPlus(WstrToUtf8(wstr), isFragment);
}

// ============================================================================
// FONCTIONS MULTILINGUE
// ============================================================================

/**
 * Récupère une chaîne traduite depuis le système de configuration Python.
 * Gère parfaitement l'encodage UTF-8 (fini les caractères bizarres !)
 */
std::wstring GetPluginTranslation(const std::string& section, const std::string& key, const std::wstring& defaultValue) {
    if (!Py_IsInitialized()) return defaultValue;
    
    PyObject* pModule = PyImport_ImportModule("config_manager");
    if (!pModule) return defaultValue;
    
    PyObject* pClass = PyObject_GetAttrString(pModule, "Traducteur");
    if (!pClass) {
        Py_DECREF(pModule);
        return defaultValue;
    }
    
    PyObject* pResult = PyObject_CallMethod(pClass, "get_text", "sss", section.c_str(), key.c_str(), "");
    if (!pResult) {
        Py_DECREF(pClass);
        Py_DECREF(pModule);
        return defaultValue;
    }
    
    std::wstring wResult = defaultValue;
    const char* utf8Str = PyUnicode_AsUTF8(pResult);
    
    if (utf8Str && strlen(utf8Str) > 0) {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0);
        if (size_needed > 0) {
            std::vector<wchar_t> buffer(size_needed);
            MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, &buffer[0], size_needed);
            wResult = std::wstring(&buffer[0]);
        }
    }
    
    Py_DECREF(pResult);
    Py_DECREF(pClass);
    Py_DECREF(pModule);
    return wResult;
}

/**
 * Fonction unifiée pour lire l'INI.
 * Désormais, elle envoie la requête au Python pour éviter les bugs d'accents de l'API Windows.
 */
std::wstring GetLangText(const wchar_t* section, const wchar_t* key, const wchar_t* def = L"") {
    // 1. Conversion du nom de la section en UTF-8 pour Python
    char sectionA[128];
    WideCharToMultiByte(CP_UTF8, 0, section, -1, sectionA, 128, NULL, NULL);

    // 2. Conversion de la clé en UTF-8 pour Python
    char keyA[128];
    WideCharToMultiByte(CP_UTF8, 0, key, -1, keyA, 128, NULL, NULL);

    // 3. Appel au moteur Python !
    std::wstring result = GetPluginTranslation(sectionA, keyA, def);
    
    // Fallback si Python renvoie vide alors qu'il ne devrait pas
    if (result.empty()) return std::wstring(def);
    return result;
}

/**
 * Demande une traduction à Python en injectant une variable dynamique (ex: {provider})
 */
std::wstring GetLangTextWithVar(const wchar_t* section, const wchar_t* key, const wchar_t* def, const char* varName, const wchar_t* varValue) {
    if (!Py_IsInitialized()) return std::wstring(def);

    char sectionA[128], keyA[128], varValueA[256];
    WideCharToMultiByte(CP_UTF8, 0, section, -1, sectionA, 128, NULL, NULL);
    WideCharToMultiByte(CP_UTF8, 0, key, -1, keyA, 128, NULL, NULL);
    WideCharToMultiByte(CP_UTF8, 0, varValue, -1, varValueA, 256, NULL, NULL);

    PyObject* pModule = PyImport_ImportModule("config_manager");
    if (!pModule) return std::wstring(def);

    PyObject* pClass = PyObject_GetAttrString(pModule, "Traducteur");
    if (!pClass) { Py_DECREF(pModule); return std::wstring(def); }

    PyObject* pFunc = PyObject_GetAttrString(pClass, "get_text");
    if (!pFunc) { Py_DECREF(pClass); Py_DECREF(pModule); return std::wstring(def); }

    // On prépare les arguments standards
    PyObject* pArgs = PyTuple_Pack(3, PyUnicode_FromString(sectionA), PyUnicode_FromString(keyA), PyUnicode_FromString(""));
    
    // On prépare le dictionnaire (Les **kwargs en Python !)
    PyObject* pKwargs = PyDict_New();
    PyDict_SetItemString(pKwargs, varName, PyUnicode_FromString(varValueA));

    // On appelle Python qui va formater la chaîne pour nous
    PyObject* pResult = PyObject_Call(pFunc, pArgs, pKwargs);

    std::wstring wResult = def;
    if (pResult) {
        const char* utf8Str = PyUnicode_AsUTF8(pResult);
        if (utf8Str && strlen(utf8Str) > 0) {
            int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0);
            if (size_needed > 0) {
                std::vector<wchar_t> buffer(size_needed);
                MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, &buffer[0], size_needed);
                wResult = std::wstring(&buffer[0]);
            }
        }
        Py_DECREF(pResult);
    }

    Py_DECREF(pKwargs);
    Py_DECREF(pArgs);
    Py_DECREF(pFunc);
    Py_DECREF(pClass);
    Py_DECREF(pModule);

    return wResult;
}

// MACROS POUR LES TRADUCTIONS :
#define _T_TITLE(key) GetLangText(L"UI", key, L"").c_str()
#define _T_UI(id) GetLangText(L"UI", L#id, L"").c_str()
#define _T_MSG(key, def) GetLangText(L"MESSAGES", key, def).c_str()

/**
 * Récupère le nom de la langue active sous forme de chaîne (ex: "french", "english") depuis python
 */
std::string GetActiveLanguage() {
    if (!Py_IsInitialized()) return "french";
    
    PyObject* pModule = PyImport_ImportModule("config_manager");
    if (!pModule) return "french";
    
    PyObject* pFunc = PyObject_GetAttrString(pModule, "obtenir_langue_active");
    if (!pFunc) {
        Py_DECREF(pModule);
        return "french";
    }
    
    PyObject* pResult = PyObject_CallObject(pFunc, NULL);
    std::string lang = "french";
    
    if (pResult) {
        const char* langStr = PyUnicode_AsUTF8(pResult);
        if (langStr) {
            lang = langStr;
        }
        Py_DECREF(pResult);
    }
    
    Py_DECREF(pFunc);
    Py_DECREF(pModule);
    return lang;
}

std::wstring ObtenirInfoDLL(const std::wstring& cle) {
    wchar_t cheminDLL[MAX_PATH];
    GetModuleFileNameW(GetModuleHandleW(L"AssistantIA.dll"), cheminDLL, MAX_PATH);

    DWORD dwHandle;
    DWORD tailleInfo = GetFileVersionInfoSizeW(cheminDLL, &dwHandle);
    if (tailleInfo == 0) return L"";

    std::vector<BYTE> buffer(tailleInfo);
    if (!GetFileVersionInfoW(cheminDLL, dwHandle, tailleInfo, buffer.data())) {
        return L"";
    }

    struct LANGANDCODEPAGE {
        WORD wLanguage;
        WORD wCodePage;
    } *pTranslate;
    UINT cbTranslate;

    if (VerQueryValueW(buffer.data(), L"\\VarFileInfo\\Translation", (LPVOID*)&pTranslate, &cbTranslate)) {
        wchar_t sousBloc[256];
        // On construit le chemin dynamiquement avec la CLÉ passée en paramètre
        swprintf_s(sousBloc, 256, L"\\StringFileInfo\\%04x%04x\\%s", 
                   pTranslate[0].wLanguage, pTranslate[0].wCodePage, cle.c_str());

        wchar_t* valeurExtraite;
        UINT tailleValeur;
        if (VerQueryValueW(buffer.data(), sousBloc, (LPVOID*)&valeurExtraite, &tailleValeur)) {
            return std::wstring(valeurExtraite);
        }
    }
    return L"";
}

void DetecterLangueTot() {
    // 1. Valeur de secours
    std::string codeLangue = "french";
    
    wchar_t szAppData[MAX_PATH];
    if (GetEnvironmentVariableW(L"APPDATA", szAppData, MAX_PATH) == 0) return;
    
    // 2. Utilisation de std::wstring pour le chemin complet
    std::wstring basePath = std::wstring(szAppData) + L"\\Notepad++\\plugins\\config\\AssistantIA\\";
    std::wstring cachePath = basePath + L"last_lang.txt";
    
    // 3. Lecture du cache
    std::ifstream cacheFile(cachePath);
    if (cacheFile.is_open()) {
        std::string langSauvegardee;
        if (cacheFile >> langSauvegardee && !langSauvegardee.empty()) {
            codeLangue = langSauvegardee;
        }
        cacheFile.close();
    }
    
    // 4. Mise à jour des variables globales
    g_ActiveLang = codeLangue; 

    // 5. CORRECTION : Conversion propre en wstring pour le chemin de fichier
    int len = MultiByteToWideChar(CP_UTF8, 0, g_ActiveLang.c_str(), -1, NULL, 0);
    std::wstring wLang(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, g_ActiveLang.c_str(), -1, &wLang[0], len);
    
    // On enlève le caractère nul de fin si nécessaire (MultiByteToWideChar l'inclut)
    if (wLang.back() == L'\0') wLang.pop_back();

    // 6. Construction propre du chemin de langue
    g_LangFilePath = basePath + L"lang\\" + wLang + L".ini";
    LogToConsole(g_LangFilePath.c_str());
}

// ============================================================================
// FONCTIONS 
// ============================================================================
std::string GetActiveProvider() {
    std::string provider = "groq"; // Valeur par défaut
    
    PyObject* pModule = PyImport_ImportModule("config_manager");
    if (pModule) {
        LogToConsole(_T_MSG(L"MSG_ASSISTANTIA_OUV_b68b",L"[AssistantIA] ouverture de config_manager")); 
        PyObject* pFunc = PyObject_GetAttrString(pModule, "obtenir_provider_actif");
        if (pFunc && PyCallable_Check(pFunc)) {
            PyObject* pRes = PyObject_CallObject(pFunc, NULL);
            if (pRes) {
                // Copie immédiate en std::string avant de DECREF
                provider = PyUnicode_AsUTF8(pRes);
                Py_DECREF(pRes);
            }
            Py_DECREF(pFunc);
        }
        Py_DECREF(pModule);
    }
    
    // 1. Conversion en std::wstring
    std::wstring wProvider(provider.begin(), provider.end());
    
    // 2. Construction du message complet
    std::wstring logMessage = _T_MSG(L"MSG_GETACTIVEPROVID_6e73",L"[GetActiveProvider()] Fournisseur: ") + wProvider + L".";
    
    // 3. Envoi à la console en convertissant en const wchar_t* via .c_str()
    LogToConsole(logMessage.c_str());
    
    
    return provider;
}

// Chemin du fichier INI
void GetIniPath(wchar_t* path) {
    wchar_t szAppData[MAX_PATH];
    GetEnvironmentVariableW(L"APPDATA", szAppData, MAX_PATH);
    wsprintfW(path, L"%s\\Notepad++\\plugins\\config\\AssistantIA\\AssistantIA.ini", szAppData);
}

// Fonction pour envoyer un message dans la console
void LogToConsole(const wchar_t* message) {
    if (!g_hConsole) return; // Si la console n'est pas initialisée, on ne fait rien
    
    // On récupère le handle de la zone de texte (remplace IDC_CONSOLE_EDIT par ton vrai ID)
    HWND hEdit = GetDlgItem(g_hConsole, IDC_CONSOLE_EDIT);
    if (!hEdit) return;
    
    // 1. Ajouter l'heure pour faire "pro" (Optionnel mais pratique)
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t timePrefix[64];
    wsprintfW(timePrefix, L"[%02d:%02d:%02d] ", st.wHour, st.wMinute, st.wSecond);
    
    // 2. Préparer le message final avec un retour à la ligne
    std::wstring finalMessage = std::wstring(timePrefix) + message + L"\r\n";
    
    // 3. Récupérer la longueur du texte actuel pour se placer à la fin
    int ndx = GetWindowTextLengthW(hEdit);
    
    // 4. Placer le curseur à la fin
    SendMessageW(hEdit, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
    
    // 5. Insérer le nouveau texte
    SendMessageW(hEdit, EM_REPLACESEL, 0, (LPARAM)finalMessage.c_str());
}

// lire un booléen
bool ReadIniBool(const wchar_t* key, bool defaultValue) {
    wchar_t path[MAX_PATH];
    GetIniPath(path);
    int val = GetPrivateProfileIntW(L"Settings", key, defaultValue ? 1 : 0, path);
    return (val != 0);
}

// écrire un booléen
void WriteIniBool(const wchar_t* key, bool value) {
    wchar_t path[MAX_PATH];
    GetIniPath(path);
    WritePrivateProfileStringW(L"Settings", key, value ? L"1" : L"0", path);
    // Construction de la chaîne en C++
    std::wstring logMsg = std::wstring(L"") + _T_MSG(L"MSG_INFO_WRITEINIBO_24a3", L"[INFO] WriteIniBool appelé pour la clé :") + key + _T_MSG(L"MSG_VALEUR_830b",L" | Valeur : ") + (value ? _T_MSG(L"MSG_ACTIVE",L"Actif") : _T_MSG(L"MSG_INACTIVE",L"Inactif"));
    
    
    // Envoi à la console
    LogToConsole(logMsg.c_str());
}

// Fonction qui gère la boîte de dialogue du Prompt
INT_PTR CALLBACK PromptIADlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_INITDIALOG:
        // On stocke le pointeur (lParam) transmis par DialogBoxParamW
        SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
        
        // 1. Traduction du Titre de la fenêtre
        SetWindowTextW(hWnd, _T_UI(IDD_PROMPT_IA));
        // 2. Traduction des labels et autres éléments
        SetDlgItemTextW(hWnd, IDC_STATIC_ASK_IA, _T_UI(IDC_STATIC_ASK_IA));
        SetDlgItemTextW(hWnd, IDOK, _T_UI(IDOK));
        SetDlgItemTextW(hWnd, IDC_CHK_SAVE_LOG, _T_UI(IDC_CHK_SAVE_LOG));
        SetDlgItemTextW(hWnd, IDCANCEL, _T_UI(IDCANCEL));
            
            
        SetFocus(GetDlgItem(hWnd, IDC_PROMPT_TEXT));
        CheckDlgButton(hWnd, IDC_CHK_SAVE_LOG, g_bSaveSelectionLog ? BST_CHECKED : BST_UNCHECKED);
        return FALSE; 
        
        case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            // 2. On récupère le pointeur vers notre structure
            PromptData* pData = (PromptData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            
            // 2. Ta logique actuelle, intacte
            wchar_t wBuffer[2048] = {0};
            GetDlgItemTextW(hWnd, IDC_PROMPT_TEXT, wBuffer, 2048);
            
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, wBuffer, -1, NULL, 0, NULL, NULL);
            if (size_needed > 0) {
                std::string utf8Prompt(size_needed, 0);
                WideCharToMultiByte(CP_UTF8, 0, wBuffer, -1, &utf8Prompt[0], size_needed, NULL, NULL);
                if (!utf8Prompt.empty() && utf8Prompt.back() == '\0') utf8Prompt.pop_back();
                
                // 3. On remplit la variable via le pointeur
                if (pData) {
                    pData->prompt = utf8Prompt;
                }
            }
            
            // 4. Ton code de sauvegarde inchangé
            g_bSaveSelectionLog = (IsDlgButtonChecked(hWnd, IDC_CHK_SAVE_LOG) == BST_CHECKED);
            WriteIniBool(L"SaveSelectionLog", g_bSaveSelectionLog);
            
            EndDialog(hWnd, IDOK);
            return TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hWnd, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

// --- 1. La procédure de la boîte "Clé Manquante" ---
INT_PTR CALLBACK DlgProcSetupApi(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_INITDIALOG:
            // Met le focus directement dans le champ de saisie
            SetFocus(GetDlgItem(hwndDlg, IDC_SETUP_API_KEY));
            return FALSE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_SETUP_API_LINK) {
                // --- AJOUT DYNAMIQUE : Récupération de l'URL d'inscription depuis Python ---
                bool urlTrouvee = false;
                
                PyObject* pModule = PyImport_ImportModule("config_manager");
                if (pModule) {
                    PyObject* pFunc = PyObject_GetAttrString(pModule, "obtenir_registration_url");
                    if (pFunc && PyCallable_Check(pFunc)) {
                        PyObject* pResult = PyObject_CallObject(pFunc, NULL);
                        
                        if (pResult && PyUnicode_Check(pResult)) {
                            // Récupération de la chaîne UTF-8 (Python)
                            const char* urlA = PyUnicode_AsUTF8(pResult);
                            
                            if (urlA && strlen(urlA) > 0) {
                                // Conversion en wchar_t (Unicode) pour ShellExecuteW
                                int wchars_num = MultiByteToWideChar(CP_UTF8, 0, urlA, -1, NULL, 0);
                                wchar_t* urlW = new wchar_t[wchars_num];
                                MultiByteToWideChar(CP_UTF8, 0, urlA, -1, urlW, wchars_num);
                                
                                // Ouvre l'URL du fournisseur ACTIF dans le navigateur par défaut
                                ShellExecuteW(NULL, L"open", urlW, NULL, NULL, SW_SHOWNORMAL);
                                
                                delete[] urlW;
                                urlTrouvee = true;
                            }
                            Py_DECREF(pResult);
                        }
                        Py_DECREF(pFunc);
                    }
                    Py_DECREF(pModule);
                }
                
                // REPLI (FALLBACK) : Si l'URL est vide dans le JSON ou si Python échoue, 
                if (!urlTrouvee) {
                    // 1. Récupérer le nom du provider actif pour la recherche
                    PyObject* pModule = PyImport_ImportModule("config_manager");
                    PyObject* pNameFunc = PyObject_GetAttrString(pModule, "obtenir_provider_actif");
                    PyObject* pNameRes = PyObject_CallObject(pNameFunc, NULL);
                    
                    std::string providerName = "AI API"; // Nom par défaut
                    if (pNameRes && PyUnicode_Check(pNameRes)) {
                        providerName = PyUnicode_AsUTF8(pNameRes);
                    }
                    Py_XDECREF(pNameRes);
                    Py_XDECREF(pNameFunc);
                    Py_DECREF(pModule);

                    // 2. Construire l'URL de recherche Google (formatée pour URL)
                    // Exemple : "https://www.google.com/search?q=API+key+registration+groq"
                    std::string searchUrl = "https://www.google.com/search?q=API+key+registration+" + providerName;
                    
                    // 3. Ouvrir la recherche
                    int wchars_num = MultiByteToWideChar(CP_UTF8, 0, searchUrl.c_str(), -1, NULL, 0);
                    wchar_t* urlW = new wchar_t[wchars_num];
                    MultiByteToWideChar(CP_UTF8, 0, searchUrl.c_str(), -1, urlW, wchars_num);
                    
                    ShellExecuteW(NULL, L"open", urlW, NULL, NULL, SW_SHOWNORMAL);
                    
                    delete[] urlW;
                }
                
                return TRUE;
            }
            
            if (LOWORD(wParam) == IDOK) {
                wchar_t keyW[256];
                GetWindowTextW(GetDlgItem(hwndDlg, IDC_SETUP_API_KEY), keyW, 256);
                
                if (wcslen(keyW) < 10) {
                    MessageBoxW(hwndDlg, L"La clé saisie semble invalide ou trop courte.", L"Clé incorrecte", MB_OK | MB_ICONWARNING);
                    return TRUE;
                }

                char keyA[256];
                WideCharToMultiByte(CP_UTF8, 0, keyW, -1, keyA, 256, NULL, NULL);

                // Sauvegarde via Python
                PyObject* pModule = PyImport_ImportModule("config_manager");
                if (pModule) {
                    PyObject* pFunc = PyObject_GetAttrString(pModule, "sauvegarder_config_actuelle");
                    if (pFunc && PyCallable_Check(pFunc)) {
                        // On injecte la clé et on force le modèle par défaut pour s'assurer que ça marche
                        PyObject* pArgs = PyTuple_Pack(2, PyUnicode_FromString(keyA), PyUnicode_FromString("llama-3.3-70b-versatile"));
                        Py_XDECREF(PyObject_CallObject(pFunc, pArgs));
                        Py_DECREF(pArgs);
                        Py_DECREF(pFunc);
                    }
                    Py_DECREF(pModule);
                }

                EndDialog(hwndDlg, IDOK);
                return TRUE;
            }
            if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, IDCANCEL);
                return TRUE;
            }
            break;
    }
    return FALSE;
}

// --- 2. Le "Garde-Barrière" qui vérifie l'état de la clé ---
bool VerifierEtDemanderCle() {
    bool hasKey = false;
    
    // On interroge Python pour savoir si une clé est déjà enregistrée
    PyObject* pModule = PyImport_ImportModule("config_manager");
    if (pModule) {
        PyObject* pFuncKey = PyObject_GetAttrString(pModule, "obtenir_api_key");
        if (pFuncKey && PyCallable_Check(pFuncKey)) {
            PyObject* pResKey = PyObject_CallObject(pFuncKey, NULL);
            if (pResKey) {
                const char* keyA = PyUnicode_AsUTF8(pResKey);
                if (keyA && strlen(keyA) > 10) { // Une vraie clé API fait plus de 10 caractères
                    hasKey = true;
                }
                Py_DECREF(pResKey);
            }
            Py_DECREF(pFuncKey);
        }
        Py_DECREF(pModule);
    }

    if (hasKey) return true; // Tout va bien, on laisse passer !

    // Pas de clé : on bloque l'action en cours et on affiche la boîte de secours
    INT_PTR res = DialogBoxParamW(g_hInst, MAKEINTRESOURCE(IDD_SETUP_API_DLG), nppData._nppHandle, DlgProcSetupApi, 0);
    
    // Retourne 'true' seulement si l'utilisateur a collé une clé et cliqué sur Enregistrer
    return (res == IDOK); 
}

void ActionContextuelIA() {
    LogToConsole(_T_MSG(L"MSG_ACTIONCONTEXTUE_6cd4",L"[ActionContextuelIA] Lancement de la commande IA...")); 
    
    int currentEdit = (int)SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCI, 0, 0);
    HWND hSci = (currentEdit == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
    
    if (!hSci) {
        LogToConsole(_T_MSG(L"MSG_ACTIONCONTEXTUE_ccf6",L"[ActionContextuelIA] Erreur : Impossible de trouver l'éditeur."));
        return;
    }
    
    // --- ÉTAPE A : RÉCUPÉRATION DU TEXTE ---
    // 1. On demande la taille exacte de la sélection ET on ajoute +1 pour le '\0' final
    int selLength = SendMessage(hSci, SCI_GETSELTEXT, 0, 0) + 1;
    if (selLength <= 1) {
        LogToConsole(_T_MSG(L"MSG_ACTIONCONTEXTUE_1afe",L"[ActionContextuelIA] Bloqué : Pas de sélection.")); 
        MessageBoxW(nppData._nppHandle, 
            _T_MSG(L"MSG_VEUILLEZ_S_LECT_cc3c", L"Veuillez sélectionner du texte..."), 
            _T_MSG(L"MSG_S_LECTION_REQUI_a68c", L"Sélection requise"), 
        MB_OK | MB_ICONWARNING);
        return;
    }
    
    // 2. On crée le vecteur à la bonne taille, rempli de zéros par précaution
    std::vector<char> buffer(selLength, '\0');
    
    // 3. On copie le texte depuis Notepad++ vers notre vecteur
    SendMessage(hSci, SCI_GETSELTEXT, 0, (LPARAM)buffer.data());
    
    // 4. On transforme ça en objet string propre
    std::string texteSelectionne(buffer.data());
    
    // --- ÉTAPE B : PROMPT UTILISATEUR ---
    PromptData data; 
    
    HINSTANCE hInstPlugin = GetModuleHandleW(L"AssistantIA.dll");
    
    // Utilisation de DialogBoxParamW pour passer l'adresse de 'data' à la fenêtre
    INT_PTR result = DialogBoxParamW(hInstPlugin, MAKEINTRESOURCEW(IDD_PROMPT_IA), nppData._nppHandle, PromptIADlgProc, (LPARAM)&data);
    
    // Si l'utilisateur valide (IDOK) et que le texte n'est pas vide
    if (result != IDOK || data.prompt.empty()) {
        LogToConsole(_T_MSG(L"MSG_ACTIONANNULEEOUVIDE",L"[ActionContextuelIA] Action annulée ou vide.")); 
        return;
    }
    // --- ÉTAPE C : APPEL À L'API PYTHON ---
    std::string codeRecuU8 = "";
    LogToConsole(_T_MSG(L"MSG_ACTIONCONTEXTUE_bcd7",L"[ActionContextuelIA] Requête API en cours de traitement..."));
    
    // 1. FORCE LE RAFRAÎCHISSEMENT VISUEL DE LA CONSOLE
    // Sans cela, comme le thread va bloquer juste après, le texte ne s'afficherait qu'APRÈS la réponse !
    if (g_hConsole) {
        UpdateWindow(g_hConsole);
        HWND hEdit = GetDlgItem(g_hConsole, IDC_CONSOLE_EDIT);
        if (hEdit) UpdateWindow(hEdit);
    }
    
    // 2. AFFICHE LE SABLIER DE CHARGEMENT WINDOWS
    HCURSOR hWaitCursor = LoadCursor(NULL, IDC_WAIT);
    HCURSOR hOldCursor = SetCursor(hWaitCursor);
    
    // Récupère la valeur actuelle depuis Python (toujours à jour)
    std::string activeProv = GetActiveProvider(); 
    
    // Utilise cette valeur pour charger le module
    std::string moduleName = "providers." + activeProv;
    PyObject* pModule = PyImport_ImportModule(moduleName.c_str());
    if (pModule) {
        PyObject* pFunc = PyObject_GetAttrString(pModule, "demander_code_sur_selection");
        if (pFunc && PyCallable_Check(pFunc)) {
            
            // SÉCURITÉ ANTI-CRASH : DecodeUTF8 avec "replace" ignore les octets corrompus au lieu de renvoyer NULL
            PyObject* pUserPrompt = PyUnicode_DecodeUTF8(data.prompt.c_str(), data.prompt.length(), "replace");
            PyObject* pTexteSel = PyUnicode_DecodeUTF8(texteSelectionne.c_str(), texteSelectionne.length(), "replace");
            
            // Ultime filet de sécurité : si ça échoue quand même, on force une chaîne vide
            if (!pUserPrompt) pUserPrompt = PyUnicode_FromString("");
            if (!pTexteSel) pTexteSel = PyUnicode_FromString("");
            
            // Création des arguments : 3 paramètres (Consigne, Code, Sauvegarder)
            PyObject* pArgs = PyTuple_Pack(3, pUserPrompt, pTexteSel, PyBool_FromLong(g_bSaveSelectionLog));
            
            // PyTuple_Pack s'est approprié les références, on doit relâcher les nôtres pour éviter les fuites de mémoire
            Py_DECREF(pUserPrompt);
            Py_DECREF(pTexteSel);
            
            PyObject* pRes = PyObject_CallObject(pFunc, pArgs);
            if (pRes) {
                // Vérification supplémentaire que la réponse est bien lisible
                const char* tempStr = PyUnicode_AsUTF8(pRes);
                if (tempStr) {
                    codeRecuU8 = tempStr; 
                    } else {
                    codeRecuU8 = WstrToUtf8(_T_MSG(L"MSG_ERREUR_C_LA_R_P_89c4",L"// Erreur [C++] : La réponse de l'IA n'est pas du texte valide.")); 
                }
                Py_DECREF(pRes);
                LogToConsole(_T_MSG(L"MSG_ACTIONCONTEXTUE_2f5c",L"[ActionContextuelIA] Réponse de l'IA reçue avec succès.")); 
                } else {
                codeRecuU8 = WstrToUtf8(_T_MSG(L"MSG_ERREUR_PYTHON_L_7480",L"// Erreur [Python] : L'exécution de l'API a échoué."));
                if (PyErr_Occurred()) PyErr_Print();
            }
            Py_DECREF(pArgs);
            Py_DECREF(pFunc);
            } else {
            codeRecuU8 = WstrToUtf8(_T_MSG(L"MSG_ERREUR_C_FONCTI_f33e",L"// Erreur [C++] : Fonction demander_code_sur_selection() introuvable.")); 
        }
        Py_DECREF(pModule);
        } else {
        codeRecuU8 = WstrToUtf8(_T_MSG(L"MSG_ERREUR_C_IMPOSS_f277",L"// Erreur [C++] : Impossible de charger le module Python.")); 
        // --- LA LIGNE MAGIQUE À AJOUTER ---
        // On demande à Python d'imprimer l'explication exacte du crash !
        if (PyErr_Occurred()) {
            PyErr_Print(); 
        }
    }
    
    SetCursor(hOldCursor);
    
    // --- ÉTAPE D : FICHIER TEMPORAIRE ET SPLIT VIEW ---
    wchar_t tempDir[MAX_PATH];
    wchar_t path[MAX_PATH];
    
    if (GetTempPathW(MAX_PATH, tempDir) > 0) {
        swprintf_s(path, MAX_PATH, L"%sAssistantIA.txt", tempDir);
        } else {
        LogToConsole(_T_MSG(L"MSG_ACTIONCONTEXTUE_68d9",L"[ActionContextuelIA] Erreur système : Dossier Temp inaccessible."));
        return;
    }
    
    // 1. On mémorise la vue (0 = Principale, 1 = Secondaire) du code source
    //int vueOrigine = (int)SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCI, 0, 0);
    
    // 2. On demande à Notepad++ de basculer sur le fichier SI celui-ci est déjà ouvert
    BOOL dejaOuvert = (BOOL)SendMessage(nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)path);
    
    if (dejaOuvert) {
        SendMessage(nppData._nppHandle, WM_COMMAND, IDM_FILE_CLOSE, 0);
        
    } //else {
    // SCÉNARIO B : Le fichier n'est pas ouvert. 
    // On l'écrit physiquement sur le disque.
    std::ofstream outFile(path, std::ios::out | std::ios::trunc | std::ios::binary);
    if (outFile.is_open()) {
        outFile.write(codeRecuU8.c_str(), codeRecuU8.length());
        outFile.close();
        } else {
        LogToConsole(_T_MSG(L"MSG_ACTIONCONTEXTUE_446c",L"[ActionContextuelIA] Erreur d'écriture disque.")); 
        return;
    }
    // Et on l'ouvre normalement dans Notepad++
    //SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)path);
    //}
    
    // --- ÉTAPE E : OUVERTURE ET LOGIQUE DE SPLIT VIEW ---
    // 1. On mémorise la vue où se trouve le fichier d'origine
    int vueOrigine = (int)SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCI, 0, 0);
    
    // 2. On demande à Notepad++ d'ouvrir le fichier temporaire
    SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)path);
    
    // 3. On regarde où Notepad++ a ouvert ce fichier
    int vueActuelle = (int)SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCI, 0, 0);
    
    // 4. Si le fichier s'est ouvert du même côté, on l'envoie de l'autre côté !
    if (vueActuelle == vueOrigine) {
        // On utilise PostMessage pour laisser Notepad++ finir d'ouvrir le fichier avant de le bouger
        PostMessage(nppData._nppHandle, WM_COMMAND, IDM_VIEW_GOTO_ANOTHER_VIEW, 0);
        LogToConsole(_T_MSG(L"MSG_ACTIONCONTEXTUE_e7b4", L"[ActionContextuelIA] Fichier déplacé dans la vue opposée avec succès."));
    }
}

void LoadSettings() {
    g_ConsoleVisible = ReadIniBool(L"ConsoleVisible", true);
    g_bForceCloseOnDelete = ReadIniBool(L"ForceClose", false);
    g_bSaveSelectionLog = ReadIniBool(L"SaveSelectionLog", true);
    
    // Lecture du nom de l'API (Par défaut : vide ou le premier script trouvé)
    wchar_t iniPath[MAX_PATH];
    GetIniPath(iniPath);
}

// -- 1. Sous-classement de Scintilla --
LRESULT CALLBACK ScintillaSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    if (uMsg == WM_CONTEXTMENU) {
        g_isScintillaContextMenu = true;
    }
    // --- NOUVEAU : Interception du clic sur notre item ---
    else if (uMsg == WM_COMMAND && LOWORD(wParam) == g_cmdIdPoserQuestion && g_cmdIdPoserQuestion != 0) {
        ActionContextuelIA(); // On force le lancement de la fonction
        return 0; // On indique à Windows que l'action a été gérée avec succès
    }
    
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

// -- 2. Sous-classement de Notepad++ --
LRESULT CALLBACK NppSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    if (uMsg == WM_INITMENUPOPUP && g_isScintillaContextMenu) {
        HMENU hMenu = (HMENU)wParam;
        
        if (GetMenuState(hMenu, g_cmdIdPoserQuestion, MF_BYCOMMAND) == -1) {
            InsertMenuW(hMenu, 0, MF_BYPOSITION | MF_STRING, g_cmdIdPoserQuestion, _T_MSG(L"MSG_POSER_UNE_QUEST_cf8b",L"Poser une question à l'IA")); 
            InsertMenuW(hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
        }
        
        g_isScintillaContextMenu = false;
    }
    // --- NOUVEAU : Au cas où le clic remonte jusqu'à la fenêtre principale ---
    else if (uMsg == WM_COMMAND && LOWORD(wParam) == g_cmdIdPoserQuestion && g_cmdIdPoserQuestion != 0) {
        ActionContextuelIA(); 
        return 0; 
    }
    
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK DlgProcMainConfig(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_INITDIALOG: {
            // --- TRADUCTION DYNAMIQUE DE L'UI ---
            SetWindowTextW(hWnd, _T_TITLE(L"IDD_MAIN_CONFIG_DLG"));
            SetDlgItemTextW(hWnd, IDC_BTN_MANAGE_API, _T_UI(IDC_BTN_MANAGE_API));
            SetDlgItemTextW(hWnd, IDOK, _T_UI(IDOK));
            SetDlgItemTextW(hWnd, IDCANCEL, _T_UI(IDCANCEL));
            
            // 1. Initialisation des deux Checkboxes (.ini)
            CheckDlgButton(hWnd, IDC_CHK_CONSOLE, g_ConsoleVisible ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hWnd, IDC_CHK_FORCECLOSE, g_bForceCloseOnDelete ? BST_CHECKED : BST_UNCHECKED);
            
            // 2. Scan des dossiers pour lister les scripts d'API (*.py)
            HWND hCombo = GetDlgItem(hWnd, IDC_CMB_ACTIVE_API);
            SendMessageW(hCombo, CB_RESETCONTENT, 0, 0);
            
            std::vector<std::wstring> searchPaths;
            
            // Chemin d'installation (built-in)
            wchar_t progPath[MAX_PATH];
            swprintf_s(progPath, MAX_PATH, L"%s\\providers\\*.py", g_pluginDir);
            searchPaths.push_back(progPath);
            
            // Chemin utilisateur (AppData)
            wchar_t szAppData[MAX_PATH];
            GetEnvironmentVariableW(L"APPDATA", szAppData, MAX_PATH);
            wchar_t appDataPath[MAX_PATH];
            swprintf_s(appDataPath, MAX_PATH, L"%s\\Notepad++\\plugins\\config\\AssistantIA\\providers\\*.py", szAppData);
            searchPaths.push_back(appDataPath);
            
            for (const auto& path : searchPaths) {
                WIN32_FIND_DATAW ffd;
                HANDLE hFind = FindFirstFileW(path.c_str(), &ffd);
                if (hFind != INVALID_HANDLE_VALUE) {
                    do {
                        if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                            wchar_t providerName[MAX_PATH];
                            wcscpy_s(providerName, ffd.cFileName);
                            wchar_t* dot = wcsrchr(providerName, L'.');
                            if (dot) *dot = L'\0';
                            
                            if (_wcsicmp(providerName, L"__init__") != 0 && _wcsicmp(providerName, L"template") != 0) {
                                // Évite d'ajouter des doublons si présent dans les deux dossiers
                                if (SendMessageW(hCombo, CB_FINDSTRINGEXACT, -1, (LPARAM)providerName) == CB_ERR) {
                                    SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)providerName);
                                }
                            }
                        }
                    } while (FindNextFileW(hFind, &ffd) != 0);
                    FindClose(hFind);
                }
            }
            
            // 3. Demander à Python quel est le provider actif pour le sélectionner
            std::string activeProv = GetActiveProvider(); 
            
            // Convertis en wchar_t pour l'UI
            wchar_t activeProvW[64];
            MultiByteToWideChar(CP_UTF8, 0, activeProv.c_str(), -1, activeProvW, 64);
            
            // Met à jour ton combo box
            int idx = SendMessageW(hCombo, CB_FINDSTRINGEXACT, -1, (LPARAM)activeProvW);
            if (idx != CB_ERR) SendMessageW(hCombo, CB_SETCURSEL, idx, 0);
            return TRUE;
        }
        
        case WM_COMMAND: {
            WORD controlID = LOWORD(wParam);
            
            // CLIC SUR LE BOUTON "CONFIGURER" (Ouvre ton formulaire dynamique)
            if (controlID == IDC_BTN_MANAGE_API) {
                HWND hCombo = GetDlgItem(hWnd, IDC_CMB_ACTIVE_API);
                int sel = (int)SendMessageW(hCombo, CB_GETCURSEL, 0, 0);
                if (sel != CB_ERR) {
                    wchar_t selApiW[64];
                    char selApiA[64];
                    SendMessageW(hCombo, CB_GETLBTEXT, sel, (LPARAM)selApiW);
                    WideCharToMultiByte(CP_UTF8, 0, selApiW, -1, selApiA, 64, NULL, NULL);
                    
                    // On dit immédiatement à Python de basculer sur ce provider
                    PyObject* pModule = PyImport_ImportModule("config_manager");
                    if (pModule) {
                        PyObject* pFunc = PyObject_GetAttrString(pModule, "definir_provider_actif");
                        if (pFunc) {
                            PyObject* pArgs = PyTuple_Pack(1, PyUnicode_FromString(selApiA));
                            Py_XDECREF(PyObject_CallObject(pFunc, pArgs));
                            Py_DECREF(pArgs);
                            Py_DECREF(pFunc);
                        }
                        Py_DECREF(pModule);
                    }
                    
                    // Vérification immédiate "en amont" au changement de choix
                    VerifierEtDemanderCle();
                    
                }
                
                // On appelle TA fonction existante qui affiche la boîte formulaire (DlgProcConfig)
                VerifierEtDemanderCle();
                lancerAssistantConfig(); 
                return TRUE;
            }
            
            // CLIC SUR OK
            if (controlID == IDOK) {
                // Sauvegarde des états de l'application (.ini)
                g_ConsoleVisible = (IsDlgButtonChecked(hWnd, IDC_CHK_CONSOLE) == BST_CHECKED);
                g_bForceCloseOnDelete = (IsDlgButtonChecked(hWnd, IDC_CHK_FORCECLOSE) == BST_CHECKED);
                WriteIniBool(L"ConsoleVisible", g_ConsoleVisible);
                WriteIniBool(L"ForceClose", g_bForceCloseOnDelete);
                
                // Sauvegarde finale du choix de l'API active dans le JSON
                HWND hCombo = GetDlgItem(hWnd, IDC_CMB_ACTIVE_API);
                int sel = (int)SendMessageW(hCombo, CB_GETCURSEL, 0, 0);
                if (sel != CB_ERR) {
                    wchar_t selApiW[64];
                    char selApiA[64];
                    SendMessageW(hCombo, CB_GETLBTEXT, sel, (LPARAM)selApiW);
                    WideCharToMultiByte(CP_UTF8, 0, selApiW, -1, selApiA, 64, NULL, NULL);
                    
                    PyObject* pModule = PyImport_ImportModule("config_manager");
                    if (pModule) {
                        PyObject* pFunc = PyObject_GetAttrString(pModule, "definir_provider_actif");
                        if (pFunc) {
                            PyObject* pArgs = PyTuple_Pack(1, PyUnicode_FromString(selApiA));
                            Py_XDECREF(PyObject_CallObject(pFunc, pArgs));
                            Py_DECREF(pArgs);
                            Py_DECREF(pFunc);
                        }
                        Py_DECREF(pModule);
                    }
                }
                
                // Ultime vérification de clé avant de fermer la fenêtre de configuration
                VerifierEtDemanderCle();
                
                // Application dynamique immédiate du panneau de la console
                if (g_hConsole) {
                    ::SendMessage(nppData._nppHandle, g_ConsoleVisible ? NPPM_DMMSHOW : NPPM_DMMHIDE, 0, (LPARAM)g_hConsole);
                    ::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcFunc[3]._cmdID, g_ConsoleVisible);
                }
                
                EndDialog(hWnd, IDOK);
                return TRUE;
            }
            
            if (controlID == IDCANCEL) {
                EndDialog(hWnd, IDCANCEL);
                return TRUE;
            }
            break;
        }
    }
    return FALSE;
}

// Fonction de capture pour l'Input de la console
LRESULT CALLBACK ConsoleInputSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    HWND hDlg = (HWND)dwRefData;
    
    // Si l'utilisateur appuie sur Entrée
    if (uMsg == WM_KEYDOWN && wParam == VK_RETURN) {
        // On notifie la fenêtre parente (la console) qu'une commande est prête
        SendMessageW(hDlg, WM_COMMAND, MAKEWPARAM(IDC_CONSOLE_INPUT, 1), (LPARAM)hWnd);
        return 0; // On intercepte le message pour éviter le "bip" système
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

// Procédure de la fenêtre console
INT_PTR CALLBACK DlgProcConsole(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    switch (Message) {
        case WM_INITDIALOG: {
            HWND hInput = GetDlgItem(hWnd, IDC_CONSOLE_INPUT);
            HWND hStatic = GetDlgItem(hWnd, IDC_CONSOLE_PROMPT);
            
            
            
            // Activer le subclassing sur le champ de saisie (Touche Entrée)
            if (hInput) {
                SetWindowSubclass(hInput, ConsoleInputSubclassProc, 1, (DWORD_PTR)hWnd);
            }
            
            // Messages d'accueil
            AppendTextToConsoleW(_T_MSG(L"MSG_CONSOLE_ASSISTA_f4d8",L"--- Console AssistantIA Prête ---\r\n"));
            AppendTextToConsoleW(_T_MSG(L"MSG_ASTUCE_TAPEZ_ER_baea",L"Astuce : Tapez /error ou /success pour tester les couleurs.\r\n\r\n"));
            return TRUE;
        }
        
        case WM_SIZE: {
            HWND hEdit = GetDlgItem(hWnd, IDC_CONSOLE_EDIT);
            HWND hInput = GetDlgItem(hWnd, IDC_CONSOLE_INPUT);
            HWND hStatic = GetDlgItem(hWnd, IDC_CONSOLE_PROMPT);
            if (hEdit && hInput) {
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);
                int inputHeight = 24; // Hauteur standard de l'input
                
                MoveWindow(hEdit, 0, 0, width, height - inputHeight, TRUE);
                MoveWindow(hInput, 45, height - inputHeight, width, inputHeight, TRUE);
                MoveWindow(hStatic, 5, height - inputHeight + 5, width, inputHeight, TRUE);
            }
            return TRUE;
        }
        
        case WM_CONTEXTMENU: {
            HWND hEdit = GetDlgItem(hWnd, IDC_CONSOLE_EDIT);
            if ((HWND)wParam == hEdit) {
                HMENU hMenu = CreatePopupMenu();
                AppendMenuW(hMenu, MF_STRING, ID_CONSOLE_COPY, _T_MSG(L"MSG_COPIER",L"Copier")); 
                AppendMenuW(hMenu, MF_STRING, ID_CONSOLE_SELECTALL, _T_MSG(L"MSG_TOUT_S_LECTIONN_e5c2",L"Tout sélectionner")); 
                AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenuW(hMenu, MF_STRING, ID_CONSOLE_CLEAR, _T_MSG(L"MSG_EFFACER_LA_CONS_b28d",L"Effacer la console")); 
                
                int xPos = GET_X_LPARAM(lParam);
                int yPos = GET_Y_LPARAM(lParam);
                if (xPos == -1 && yPos == -1) {
                    RECT rc; GetWindowRect(hEdit, &rc);
                    xPos = rc.left + (rc.right - rc.left) / 2;
                    yPos = rc.top + (rc.bottom - rc.top) / 2;
                }
                TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, xPos, yPos, 0, hWnd, NULL);
                DestroyMenu(hMenu);
                return TRUE;
            }
            break;
        }
        
        case WM_COMMAND: {
            HWND hEdit = GetDlgItem(hWnd, IDC_CONSOLE_EDIT);
            HWND hInput = GetDlgItem(hWnd, IDC_CONSOLE_INPUT);
            
            switch (LOWORD(wParam)) {
                case ID_CONSOLE_COPY: SendMessageW(hEdit, WM_COPY, 0, 0); return TRUE;
                case ID_CONSOLE_SELECTALL: { CHARRANGE cr = { 0, -1 }; SendMessageW(hEdit, EM_EXSETSEL, 0, (LPARAM)&cr); return TRUE; }
                case ID_CONSOLE_CLEAR: SetWindowTextW(hEdit, L""); return TRUE;
            }
            
            // Envoi de la commande
            if (LOWORD(wParam) == IDC_CONSOLE_INPUT && HIWORD(wParam) == 1) {
                wchar_t buffer[1024];
                GetWindowTextW(hInput, buffer, 1024);
                
                if (wcslen(buffer) > 0) {
                    std::wstring echo = L">> " + std::wstring(buffer) + L"\r\n";
                    AppendTextToConsoleW(echo.c_str());
                    
                    if (_wcsicmp(buffer, L"/error") == 0) {
                        AppendTextToConsoleW(_T_MSG(L"MSG_ERREUR_LE_FILTR_1b67",L"[ERREUR] Le filtre fonctionne ! C'est écrit en rouge.\r\n"));
                    }
                    else if (_wcsicmp(buffer, L"/success") == 0) {
                        AppendTextToConsoleW(_T_MSG(L"MSG_SUCCES_LE_FILTR_dbe0",L"[SUCCÈS] Le filtre fonctionne ! C'est écrit en vert.\r\n"));
                    }
                    else {
                        char cmdA[1024];
                        WideCharToMultiByte(CP_UTF8, 0, buffer, -1, cmdA, 1024, NULL, NULL);
                        PyRun_SimpleString(cmdA);
                    }
                    SetWindowTextW(hInput, L"");
                }
                return TRUE;
            }
            break;
        }
    }
    return FALSE;
}

// Procédure de la boîte de dialogue (Gestion des événements)
INT_PTR CALLBACK DlgProcConfig(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
        {
            SetWindowTextW(hwndDlg, _T_MSG(L"MSG_CONFIGURATION_D_4504",L"Configuration de l'Assistant IA")); 
            
            // Variables pour stocker nos chaînes dynamiques
            wchar_t providerW[64] = L"IA"; // Valeur de secours si Python fait grève
            wchar_t labelKey[128] = {0};
            
            // --- 1. INTERROGATION DE PYTHON (Qui est le boss actuel ?) ---
            PyObject* pModule = PyImport_ImportModule("config_manager");
            if (pModule) 
            {
                PyObject* pFuncProv = PyObject_GetAttrString(pModule, "obtenir_provider_actif");
                if (pFuncProv && PyCallable_Check(pFuncProv)) {
                    PyObject* pResProv = PyObject_CallObject(pFuncProv, NULL);
                    if (pResProv) {
                        const char* provA = PyUnicode_AsUTF8(pResProv);
                        MultiByteToWideChar(CP_UTF8, 0, provA, -1, providerW, 64);
                        
                        // Petite coquetterie : on passe la première lettre en majuscule (ex: groq -> Groq)
                        if (providerW[0] >= L'a' && providerW[0] <= L'z') {
                            providerW[0] = providerW[0] - 32;
                        }
                        Py_DECREF(pResProv);
                    }
                    Py_DECREF(pFuncProv);
                }
            }
            
            // On prépare le libellé de la clé dynamiquement au format : "Clé API providerActif :"
            swprintf_s(labelKey, _T_MSG(L"MSG_CL_API_PROVIDER_bc41",L"Clé API %s :"), providerW);
            
            
            // --- 2. CRÉATION DYNAMIQUE DES COMPOSANTS (Avec les variables) ---
            // Le label utilise maintenant notre 'labelKey' dynamique !
            CreateWindowExW(0, L"STATIC", labelKey, WS_CHILD | WS_VISIBLE, 15, 15, 200, 18, hwndDlg, NULL, NULL, NULL);
            
            // Le champ texte un peu moins large (330 au lieu de 380)
            HWND hwndKey = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_PASSWORD, 15, 35, 330, 22, hwndDlg, (HMENU)IDC_KEY_INPUT, NULL, NULL);
            
            // Le bouton Switch juste à côté
            CreateWindowExW(0, L"BUTTON", L"👁", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 350, 34, 45, 24, hwndDlg, (HMENU)IDC_KEY_TOGGLE, NULL, NULL);
            CreateWindowExW(0, L"STATIC", _T_MSG(L"MSG_MOD_LE_ACTIF_fd41",L"Modèle actif :"), WS_CHILD | WS_VISIBLE, 15, 70, 200, 18, hwndDlg, NULL, NULL, NULL); 
            HWND hwndCombo = CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP, 15, 90, 250, 150, hwndDlg, (HMENU)IDC_MODEL_COMBO, NULL, NULL);
            CreateWindowExW(0, L"BUTTON", _T_MSG(L"MSG_SYNCHRONISER_d773",L"🔄 Synchroniser"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 275, 89, 120, 24, hwndDlg, (HMENU)IDC_BTN_REFRESH, NULL, NULL);
            CreateWindowExW(0, L"BUTTON", _T_MSG(L"MSG_ENREGISTRER_LA__34a5",L"Enregistrer la configuration"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 15, 140, 380, 30, hwndDlg, (HMENU)IDC_BTN_SAVE, NULL, NULL);
            
            SetWindowPos(hwndDlg, NULL, 0, 0, 425, 220, SWP_NOMOVE);
            
            
            // --- 3. REMPLISSAGE DES DONNÉES (Clé et Modèles) ---
            if (pModule) 
            {
                // A. On récupère et on affiche la Clé API du provider actif
                PyObject* pFuncKey = PyObject_GetAttrString(pModule, "obtenir_api_key");
                if (pFuncKey && PyCallable_Check(pFuncKey)) {
                    PyObject* pResKey = PyObject_CallObject(pFuncKey, NULL);
                    if (pResKey) {
                        const char* keyA = PyUnicode_AsUTF8(pResKey);
                        wchar_t keyW[256];
                        MultiByteToWideChar(CP_UTF8, 0, keyA, -1, keyW, 256);
                        SetWindowTextW(hwndKey, keyW);
                        Py_DECREF(pResKey);
                    }
                    Py_DECREF(pFuncKey);
                }
                
                // B. On récupère le modèle sélectionné par défaut
                wchar_t selectedModelW[128] = L"";
                PyObject* pFuncSel = PyObject_GetAttrString(pModule, "obtenir_modele_selectionne");
                if (pFuncSel && PyCallable_Check(pFuncSel)) {
                    PyObject* pResSel = PyObject_CallObject(pFuncSel, NULL);
                    if (pResSel) {
                        MultiByteToWideChar(CP_UTF8, 0, PyUnicode_AsUTF8(pResSel), -1, selectedModelW, 128);
                        Py_DECREF(pResSel);
                    }
                    Py_DECREF(pFuncSel);
                }
                
                // C. On récupère la liste des modèles du provider actif et on remplit le ComboBox
                PyObject* pFuncList = PyObject_GetAttrString(pModule, "obtenir_liste_modeles_str");
                if (pFuncList && PyCallable_Check(pFuncList)) {
                    PyObject* pResList = PyObject_CallObject(pFuncList, NULL);
                    if (pResList) {
                        const char* listA = PyUnicode_AsUTF8(pResList);
                        wchar_t listW[2048];
                        MultiByteToWideChar(CP_UTF8, 0, listA, -1, listW, 2048);
                        
                        wchar_t* context = NULL;
                        wchar_t* token = wcstok_s(listW, L",", &context);
                        int indexToSelect = 0;
                        int currentIndex = 0;
                        
                        while (token != NULL) {
                            SendMessageW(hwndCombo, CB_ADDSTRING, 0, (LPARAM)token);
                            if (wcscmp(token, selectedModelW) == 0) {
                                indexToSelect = currentIndex;
                            }
                            currentIndex++;
                            token = wcstok_s(NULL, L",", &context);
                        }
                        SendMessageW(hwndCombo, CB_SETCURSEL, indexToSelect, 0);
                        Py_DECREF(pResList);
                    }
                    Py_DECREF(pFuncList);
                }
                Py_DECREF(pModule); // On libère enfin le module Python
            }
            
            // --- 4. POLICE SYSTÈME MODERNE ---
            HFONT hFont = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            SendMessageW(hwndDlg, WM_SETFONT, (WPARAM)hFont, TRUE);
            EnumChildWindows(hwndDlg, [](HWND hChild, LPARAM lp) -> BOOL {
                SendMessageW(hChild, WM_SETFONT, (WPARAM)lp, TRUE);
                return TRUE;
            }, (LPARAM)hFont);
            
            return TRUE;
        }
        
        case WM_COMMAND:
        {
            int controlId = LOWORD(wParam);
            int notificationCode = HIWORD(wParam);
            
            if (controlId == IDC_BTN_SAVE) // Clic sur Enregistrer
            {
                HWND hwndKey = GetDlgItem(hwndDlg, IDC_KEY_INPUT);
                HWND hwndCombo = GetDlgItem(hwndDlg, IDC_MODEL_COMBO);
                
                // 1. Extraction de la clé API entrée par l'utilisateur
                wchar_t keyW[256] = L"";
                GetWindowTextW(hwndKey, keyW, 256);
                
                // 2. Extraction du modèle sélectionné dans la ComboBox
                wchar_t modelW[128] = L"";
                int selIdx = (int)SendMessageW(hwndCombo, CB_GETCURSEL, 0, 0);
                if (selIdx != CB_ERR) {
                    SendMessageW(hwndCombo, CB_GETLBTEXT, selIdx, (LPARAM)modelW);
                }
                
                // 3. Conversion UTF-16 (Windows) vers UTF-8 (Python/JSON)
                char keyA[256];
                char modelA[128];
                WideCharToMultiByte(CP_UTF8, 0, keyW, -1, keyA, 256, NULL, NULL);
                WideCharToMultiByte(CP_UTF8, 0, modelW, -1, modelA, 128, NULL, NULL);
                
                // 4. Appel de la passerelle Python pour enregistrer dans le JSON
                PyObject* pModule = PyImport_ImportModule("config_manager");
                bool succes = false;
                if (pModule) {
                    PyObject* pFunc = PyObject_GetAttrString(pModule, "sauvegarder_config_actuelle");
                    if (pFunc && PyCallable_Check(pFunc)) {
                        PyObject* pArgs = PyTuple_Pack(2, PyUnicode_FromString(keyA), PyUnicode_FromString(modelA));
                        PyObject* pRes = PyObject_CallObject(pFunc, pArgs);
                        
                        if (pRes) {
                            if (PyObject_IsTrue(pRes)) {
                                succes = true;
                            }
                            Py_DECREF(pRes);
                        }
                        Py_DECREF(pArgs);
                        Py_DECREF(pFunc);
                    }
                    Py_DECREF(pModule);
                }
                
                // 5. Verdict et fermeture si tout est OK
                if (succes) {
                    MessageBoxW(hwndDlg, _T_MSG(L"MSG_CONFIGURATION_S_a5b2",L"Configuration sauvegardée avec succès !"), L"Succès", MB_OK | MB_ICONINFORMATION);
                    EndDialog(hwndDlg, IDOK); // On ferme proprement la fenêtre
                    } else {
                    MessageBoxW(hwndDlg, _T_MSG(L"MSG_ERREUR_LORS_DE__1d1f",L"Erreur lors de la sauvegarde de la configuration."), L"Échec", MB_OK | MB_ICONERROR);
                    // On ne ferme pas la fenêtre pour permettre à l'utilisateur de corriger
                }
                return TRUE;
            }
            else if (controlId == IDC_KEY_TOGGLE) // Clic sur l'œil 👁
            {
                HWND hwndKey = GetDlgItem(hwndDlg, IDC_KEY_INPUT);
                wchar_t passwordChar = (wchar_t)SendMessageW(hwndKey, EM_GETPASSWORDCHAR, 0, 0);
                
                if (passwordChar != 0) {
                    SendMessageW(hwndKey, EM_SETPASSWORDCHAR, 0, 0);
                    } else {
                    SendMessageW(hwndKey, EM_SETPASSWORDCHAR, (WPARAM)L'•', 0);
                }
                InvalidateRect(hwndKey, NULL, TRUE);
                return TRUE;
            }
            else if (controlId == IDC_BTN_REFRESH) // Clic sur Mettre à jour / Synchroniser
            {
                HWND hwndKey = GetDlgItem(hwndDlg, IDC_KEY_INPUT);
                HWND hwndCombo = GetDlgItem(hwndDlg, IDC_MODEL_COMBO);
                
                // 1. Récupération de la clé API du champ de saisie
                wchar_t keyW[256] = L"";
                GetWindowTextW(hwndKey, keyW, 256);
                
                char keyA[256];
                WideCharToMultiByte(CP_UTF8, 0, keyW, -1, keyA, 256, NULL, NULL);
                
                // 2. Appel de config_manager.setup_premier_demarrage(keyA)
                PyObject* pModule = PyImport_ImportModule("config_manager");
                if (pModule) {
                    PyObject* pFuncSetup = PyObject_GetAttrString(pModule, "setup_premier_demarrage");
                    if (pFuncSetup && PyCallable_Check(pFuncSetup)) {
                        PyObject* pArgs = PyTuple_Pack(1, PyUnicode_FromString(keyA));
                        PyObject* pResJson = PyObject_CallObject(pFuncSetup, pArgs);
                        
                        if (pResJson) {
                            const char* jsonStr = PyUnicode_AsUTF8(pResJson);
                            
                            // --- NOUVEAU : LOG DE LA RÉPONSE BRUTE DANS LA CONSOLE ---
                            wchar_t wJsonStr[2048];
                            MultiByteToWideChar(CP_UTF8, 0, jsonStr, -1, wJsonStr, 2048);
                            std::wstring logMsg = std::wstring(_T_MSG(L"MSG_SYNC_API_RETOUR_9d07",L"[Sync API] Retour Python : ")) + wJsonStr;
                            LogToConsole(logMsg.c_str());
                            // ---------------------------------------------------------
                            
                            if (jsonStr && strstr(jsonStr, "\"status\": \"success\"")) {
                                MessageBoxW(hwndDlg, _T_MSG(L"MSG_MOD_LES_SYNCHRO_8168",L"Modèles synchronisés avec succès depuis l'API !"), L"Success", MB_OK | MB_ICONINFORMATION);
                                
                                PyObject* pFuncList = PyObject_GetAttrString(pModule, "obtenir_liste_modeles_str");
                                if (pFuncList && PyCallable_Check(pFuncList)) {
                                    PyObject* pResList = PyObject_CallObject(pFuncList, NULL);
                                    if (pResList) {
                                        const char* modelsA = PyUnicode_AsUTF8(pResList);
                                        
                                        if (modelsA && strlen(modelsA) > 0) {
                                            SendMessageW(hwndCombo, CB_RESETCONTENT, 0, 0);
                                            int wlen = MultiByteToWideChar(CP_UTF8, 0, modelsA, -1, NULL, 0);
                                            wchar_t* modelsW = new wchar_t[wlen];
                                            MultiByteToWideChar(CP_UTF8, 0, modelsA, -1, modelsW, wlen);
                                            
                                            wchar_t* context = NULL;
                                            wchar_t* token = wcstok_s(modelsW, L",", &context);
                                            while (token != NULL) {
                                                SendMessageW(hwndCombo, CB_ADDSTRING, 0, (LPARAM)token);
                                                token = wcstok_s(NULL, L",", &context);
                                            }
                                            SendMessageW(hwndCombo, CB_SETCURSEL, 0, 0);
                                            delete[] modelsW;
                                        }
                                        Py_DECREF(pResList);
                                    }
                                    Py_DECREF(pFuncList);
                                }
                                
                                } else {
                                // --- NOUVEAU : AFFICHE L'ERREUR EXACTE DU SERVEUR DANS LA BOÎTE DE DIALOGUE ---
                                MessageBoxW(hwndDlg, wJsonStr, _T_MSG(L"MSG_ERREUR_DE_SYNCH_df65",L"Erreur de synchronisation API"), MB_OK | MB_ICONERROR);
                            }
                            Py_DECREF(pResJson);
                        }
                        Py_DECREF(pArgs);
                        Py_DECREF(pFuncSetup);
                    }
                    Py_DECREF(pModule);
                }
                return TRUE;
            }
            else if (controlId == IDCANCEL) // Clic sur la croix ou Échap
            {
                EndDialog(hwndDlg, IDCANCEL);
                return TRUE;
            }
            break;
        }
    }
    return FALSE;
}

// POINT D'ENTRÉE DE LA DLL
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        hInst       = (HINSTANCE)hModule;   // selon tes besoins
        g_hInst     = (HINSTANCE)hModule;   // selon tes besoins
        g_hModule   = hModule;              // selon tes besoins
        g_hInstance = (HINSTANCE)hModule;   // ✓ corrigé

        g_hRichEditLib = LoadLibraryW(L"Msftedit.dll");

        // Répertoire du plugin
        GetModuleFileNameW(hModule, g_pluginDir, MAX_PATH);
        wchar_t* lastSlash = wcsrchr(g_pluginDir, L'\\');
        if (lastSlash) {
            *lastSlash = L'\0';
            wchar_t pythonDllPath[MAX_PATH];
            swprintf_s(pythonDllPath, MAX_PATH, L"%s\\python314.dll", g_pluginDir);
            hPythonModule = LoadLibraryW(pythonDllPath);
        }
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        if (hPythonModule)  FreeLibrary(hPythonModule);
        if (g_hRichEditLib) FreeLibrary(g_hRichEditLib);
    }
    return TRUE;
}

void lancerAssistantConfig()
{
    // On s'assure que l'environnement Python est OK
    PyRun_SimpleString(
        "import sys, os\n"
        "npp_dir = os.path.dirname(sys.executable)\n"
        "plugin_path = os.path.join(npp_dir, 'plugins', 'AssistantIA')\n"
        "if plugin_path not in sys.path: sys.path.append(plugin_path)\n"
    );
    
    // Récupération de la poignée de Notepad++ fournie par l'API des plugins
    // Note : nppData._nppHandle est la variable globale standard des plugins Notepad++
    HWND hwndParent = nppData._nppHandle; 
    
    // --- CRÉATION DU GABARIT DE LA FENÊTRE EN MÉMOIRE ---
    // On dimensionne une structure de dialogue Win32 à la volée
    unsigned char alignment_buffer[1024];
    LPDLGTEMPLATEW lpdt = (LPDLGTEMPLATEW)alignment_buffer;
    
    lpdt->style = DS_SETFONT | DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU;
    lpdt->dwExtendedStyle = 0;
    lpdt->cdit = 0; // On ajoute nos composants dynamiquement après, ou Windows gère le conteneur
    lpdt->x = 0;  lpdt->y = 0;
    lpdt->cx = 280; lpdt->cy = 150; // Taille idéale de la fenêtre (unités de dialogue)
    
    // On spécifie la police système moderne (Segoe UI, taille 9)
    LPWSTR lpw = (LPWSTR)(lpdt + 1);
    *lpw++ = 0; // Pas de menu
    *lpw++ = 0; // Classe par défaut
    
    // Titre de la fenêtre
    wcscpy_s(lpw, 50, _T_MSG(L"MSG_ASSISTANT_IA_cdfc",L"Assistant IA"));
    lpw += wcslen(lpw) + 1;
    
    // Taille de police et Nom
    *lpw++ = 9;
    wcscpy_s(lpw, 50, L"Segoe UI");
    lpw += wcslen(lpw) + 1;
    
    // --- INJECTION DES COMPOSANTS GRAPHIQUE DANS NOTRE CONTENEUR ---
    // On va intercepter le WM_INITDIALOG pour créer dynamiquement les boutons 
    // et inputs au pixel près à l'intérieur de ce conteneur lors du rendu.
    
    // On lance la boîte de dialogue sur l'écran actif de Notepad++ !
    DialogBoxIndirectParamW(NULL, lpdt, hwndParent, DlgProcConfig, 0);
}

extern "C" __declspec(dllexport) BOOL isUnicode() { return TRUE; }

extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam) { return TRUE; }

// INITIALISATION DU MOTEUR PYTHON
extern "C" __declspec(dllexport) void setInfo(NppData notepadPlusData)
{
    nppData = notepadPlusData;
}

extern "C" __declspec(dllexport) const wchar_t * getName() { return L"Assistant IA"; }

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbFunc)
{
    // 1. Lire le cache texte de la langue active
    DetecterLangueTot();

    // 2. ALLUMAGE ET CONFIGURATION COMPLÈTE DU MOTEUR PYTHON
    if (!Py_IsInitialized()) {
        PyConfig config;
        PyConfig_InitIsolatedConfig(&config);
        
        // Récupérer le chemin complet de ta DLL
        wchar_t pluginDir[MAX_PATH];
        GetModuleFileNameW(g_hModule, pluginDir, MAX_PATH);
        
        // Retirer "AssistantIA.dll" pour ne garder que le dossier du plugin
        wchar_t* lastSlash = wcsrchr(pluginDir, L'\\');
        if (lastSlash) {
            *lastSlash = L'\0'; 
        }
        
        // Définition des chemins absolus vers ton sous-dossier propre
        std::wstring pythonHome = std::wstring(pluginDir) + L"\\python3.14.5";
        std::wstring pythonZip = pythonHome + L"\\python314.zip";
        
        // --- NOUVEAU : On prépare le chemin AppData pour les scripts utilisateurs ---
        wchar_t szAppData[MAX_PATH];
        GetEnvironmentVariableW(L"APPDATA", szAppData, MAX_PATH);
        std::wstring appDataPluginPath = std::wstring(szAppData) + L"\\Notepad++\\plugins\\config\\AssistantIA";
        
        // Configuration de la "Maison" de Python
        PyStatus status = PyConfig_SetString(&config, &config.home, pythonHome.c_str());
        
        // FORÇAGE EXPLICITE DU SYS.PATH (On court-circuite Notepad++.exe)
        if (!PyStatus_Exception(status)) {
            // 1. Le ZIP contenant la bibliothèque standard
            status = PyWideStringList_Append(&config.module_search_paths, pythonZip.c_str());
        }
        if (!PyStatus_Exception(status)) {
            // 2. Le sous-dossier pour tes scripts (config_manager.py, logger.py)
            status = PyWideStringList_Append(&config.module_search_paths, pythonHome.c_str());
        }
        if (!PyStatus_Exception(status)) {
            // 3. CORRECTION : La racine du plugin (Pour qu'il trouve le dossier "providers" d'usine !)
            status = PyWideStringList_Append(&config.module_search_paths, pluginDir);
        }
        if (!PyStatus_Exception(status)) {
            // 4. CORRECTION : Le dossier AppData (Pour qu'il trouve les "providers" créés par l'utilisateur !)
            status = PyWideStringList_Append(&config.module_search_paths, appDataPluginPath.c_str());
        }
        
        // On dit à Python : "Utilise uniquement cette liste"
        config.module_search_paths_set = 1;
        
        // Initialisation finale du moteur
        if (!PyStatus_Exception(status)) {
            status = Py_InitializeFromConfig(&config);
        }
        
        PyConfig_Clear(&config);
        
        if (PyStatus_Exception(status)) {
            wchar_t pythonErr[512] = {0};
            if (status.err_msg) {
                MultiByteToWideChar(CP_ACP, 0, status.err_msg, -1, pythonErr, 512);
            } else {
                wcscpy_s(pythonErr, 512, _T_MSG(L"MSG_ERREUR_D_INITIA_93d2",L"Erreur d'initialisation inconnue.")); 
            }
            wchar_t boiteMessage[1024];
            swprintf_s(boiteMessage, 1024, _T_MSG(L"MSG_ECHEC_DE_L_INIT_922e",L"Échec de l'initialisation de Python.\n\nRaison :\n%s"), pythonErr); 
            MessageBoxW(NULL, boiteMessage, _T_MSG(L"MSG_ASSISTANT_IA_DI_a6fd",L"Assistant IA - Diagnostic Moteur"), MB_OK | MB_ICONERROR); 
        
    } else {
            // --- PYTHON EST ALLUMÉ, ON LE CONFIGURE IMMÉDIATEMENT ---
            PyRun_SimpleString("import sys; sys.stderr = sys.stdout");
            // A. Injection du path
            PyObject* sysModule = PyImport_ImportModule("sys");
            if (sysModule) {
                PyObject* pathList = PyObject_GetAttrString(sysModule, "path");
                if (pathList) {
                    // On pointe vers le sous-dossier pour charger les scripts .py
                    std::wstring pythonScriptPath = std::wstring(g_pluginDir) + L"\\python3.14.5";
                    PyObject* pPathStr = PyUnicode_FromWideChar(pythonScriptPath.c_str(), -1);
                    if (pPathStr) {
                        PyList_Append(pathList, pPathStr);
                        Py_DECREF(pPathStr);
                    }
                    Py_DECREF(pathList);
                }
                Py_DECREF(sysModule);
            }
            
            // B. Soudure logger
            PyObject* pLogModule = PyImport_ImportModule("logger");
            if (pLogModule) {
                PyObject* pFuncLog = PyObject_GetAttrString(pLogModule, "init_logger");
                if (pFuncLog && PyCallable_Check(pFuncLog)) {
                    PyObject* pArgs = PyTuple_New(1);
                    PyTuple_SetItem(pArgs, 0, PyLong_FromLongLong((long long)&AppendTextToConsole));
                    PyObject* pRes = PyObject_CallObject(pFuncLog, pArgs);
                    if (pRes) Py_DECREF(pRes);
                    Py_DECREF(pArgs);
                    Py_DECREF(pFuncLog);
                }
                Py_DECREF(pLogModule);
            } else {
                // SI L'IMPORT ÉCHOUE : ON FORCE L'AFFICHAGE DU MESSAGE D'ERREUR
                if (PyErr_Occurred()) {
                    PyErr_Print(); // Cela va maintenant s'afficher dans votre console grâce à la redirection
                }
            }
            
            // C. SYNCHRONISATION LANGUE IMMEDIATE AVANT LES MENUS
            PyObject* pCM = PyImport_ImportModule("config_manager");
            if (pCM) {
                PyObject* pFunc = PyObject_GetAttrString(pCM, "synchroniser_langue_npp");
                if (pFunc) {
                    PyObject* pArgs = PyTuple_Pack(1, PyUnicode_FromString(g_ActiveLang.c_str()));
                    Py_XDECREF(PyObject_CallObject(pFunc, pArgs));
                    Py_DECREF(pArgs);
                    Py_DECREF(pFunc);
                }
                Py_DECREF(pCM);
            }else {
                // SI L'IMPORT ÉCHOUE : ON FORCE L'AFFICHAGE DU MESSAGE D'ERREUR
                if (PyErr_Occurred()) {
                    PyErr_Print(); // Cela va maintenant s'afficher dans votre console grâce à la redirection
                }
            }
        }
    }

    *nbFunc = 8; // On annonce à Notepad++ qu'on a 8 menus
    
    // =========================================================
    // MAINTENANT PYTHON SAIT QU'IL EST MULTILINGUE
    // =========================================================

    // Menu 1 : Nouveau Script
    wcscpy_s(funcFunc[0]._itemName, 64, _T_MSG(L"MSG_NOUVEAU_SCRIPT_666b", L"Nouveau Script.")); 
    funcFunc[0]._pFunc = CreerNouveauScript;
    funcFunc[0]._init2Check = false;
    funcFunc[0]._pShKey = NULL;
    
    // Menu 2 : Edition Scripts Utilisateur
    wcscpy_s(funcFunc[1]._itemName, 64, _T_MSG(L"MSG_EDITION_SCRIPTS_7f62", L"Edition Scripts Utilisateur")); 
    funcFunc[1]._pFunc = GererScriptsUtilisateur;
    funcFunc[1]._init2Check = false;
    funcFunc[1]._pShKey = NULL;
    
    // Menu 3 : AssistantIA (Le cœur du réacteur)
    wcscpy_s(funcFunc[2]._itemName, 64, _T_MSG(L"MSG_ASSISTANTIA_OUV_91c4", L"AssistantIA (Ouvrir un chat)")); 
    funcFunc[2]._pFunc = LancerAssistant;
    funcFunc[2]._init2Check = false;
    funcFunc[2]._pShKey = NULL;
    
    // Menu 4 : Ouvrir la Console
    wcscpy_s(funcFunc[3]._itemName, 64, _T_MSG(L"MSG_AFFICHER_MASQUE_7131", L"Afficher/Masquer la Console")); 
    funcFunc[3]._pFunc = ToggleConsole;
    funcFunc[3]._init2Check = false;
    funcFunc[3]._pShKey = NULL;
    
    // Menu 5 : Configuration
    wcscpy_s(funcFunc[4]._itemName, 64, _T_MSG(L"MSG_CONFIGURATION", L"Configuration")); 
    funcFunc[4]._pFunc = OuvrirConfiguration;
    funcFunc[4]._init2Check = false;
    funcFunc[4]._pShKey = NULL;
    
    // Menu 6 : Aide
    wcscpy_s(funcFunc[5]._itemName, 64, _T_MSG(L"MSG_AIDE", L"Aide")); 
    funcFunc[5]._pFunc = AfficherAide;
    funcFunc[5]._init2Check = false;
    funcFunc[5]._pShKey = NULL;
    
    // Menu 7 : Version
    wcscpy_s(funcFunc[6]._itemName, 64, _T_MSG(L"MSG_VERSION", L"Version"));
    funcFunc[6]._pFunc = AfficherVersion;
    funcFunc[6]._init2Check = false;
    funcFunc[6]._pShKey = NULL;
    
    // Menu 8 : Action IA Contextuelle
    wcscpy_s(funcFunc[7]._itemName, 64, _T_MSG(L"MSG_SOUMETTRE_L_IA_00c9", L"Soumettre à l'IA"));
    funcFunc[7]._pFunc = ActionContextuelIA;
    funcFunc[7]._init2Check = false;
    funcFunc[7]._pShKey = NULL;
    
    // Raccourci Chat
    static ShortcutKey shKeyChat;
    shKeyChat._isCtrl  = false;         
    shKeyChat._isAlt   = true;          
    shKeyChat._isShift = false;         
    shKeyChat._key     = 'I';           
    funcFunc[2]._pShKey = &shKeyChat;   
    
    // Raccourci Action IA (ALT + Q)
    static ShortcutKey shKeyQuestion; 
    shKeyQuestion._isCtrl = false;
    shKeyQuestion._isAlt = true;      
    shKeyQuestion._isShift = false;
    shKeyQuestion._key = 'Q';         
    
    funcFunc[7]._pShKey = &shKeyQuestion; 
    
    return funcFunc;
}

// ============================================================================
// 1. Version ANSI / UTF-8 (Appelée par le moteur Python)
// Sa SEULE mission : Convertir en UTF-16 et passer le bébé à la version Unicode
// ============================================================================
void AppendTextToConsole(const char* text) {
    if (!text) return;
    
    // 1. Calculer la taille nécessaire pour la conversion en UTF-16
    int len = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
    if (len <= 0) return;
    
    // 2. Allocation de la mémoire temporaire pour le texte converti
    wchar_t* wText = new wchar_t[len];
    MultiByteToWideChar(CP_UTF8, 0, text, -1, wText, len);
    
    // 3. ON PASSE LE RELAIS à la fonction Unicode (W) qui gère les couleurs et l'affichage
    AppendTextToConsoleW(wText);
    
    // 4. Nettoyage propre de la mémoire
    delete[] wText;
}

// ============================================================================
// 2. Version Unicode / Wide (Appelée par le C++ et par la fonction ci-dessus)
// Sa mission : Centraliser TOUTE la logique d'affichage (Couleurs, Insertion, Défilement)
// ============================================================================
void AppendTextToConsoleW(const wchar_t* text) {
    if (!g_hConsole || !text) return;
    HWND hEdit = GetDlgItem(g_hConsole, IDC_CONSOLE_EDIT);
    if (!hEdit) return;
    
    // 1. Placer le curseur à la toute fin du texte existant
    int len = GetWindowTextLengthW(hEdit);
    CHARRANGE cr;
    cr.cpMin = len;
    cr.cpMax = len;
    SendMessageW(hEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
    
    // 2. Configuration du format de couleur (Pour fond clair standard)
    CHARFORMAT2W cf;
    ZeroMemory(&cf, sizeof(cf));
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR;
    
    // FILTRE DE COULEUR : Détection des erreurs (Tags C++ et retours d'erreurs Python)
    if (wcsstr(text, L"[ERREUR]") != NULL || wcsstr(text, L"[ERROR]") != NULL || 
        wcsstr(text, L"failed") != NULL  || wcsstr(text, L"Error") != NULL || 
    wcsstr(text, L"Exception") != NULL || wcsstr(text, L"Traceback") != NULL) 
    {
        cf.crTextColor = RGB(200, 0, 0); // Rouge foncé très lisible sur blanc
    } 
    // FILTRE DE COULEUR : Détection des succès
    else if (wcsstr(text, L"[SUCCÈS]") != NULL || wcsstr(text, L"[SUCCESS]") != NULL) 
    {
        cf.crTextColor = RGB(0, 128, 0);  // Vert foncé très lisible sur blanc
    } 
    // Par défaut : Texte normal
    else 
    {
        cf.crTextColor = RGB(0, 0, 0);    // Noir classique
    }
    
    // Appliquer la couleur choisie à l'emplacement du curseur
    SendMessageW(hEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    
    // 3. Insérer le texte et forcer le défilement vers le bas
    SendMessageW(hEdit, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessageW(hEdit, WM_VSCROLL, SB_BOTTOM, 0);
}

void VerifierChangementLangueUI() {
    // 1. CORRECTION ENCODAGE : On utilise un char au lieu d'un wchar_t
    char langFilePath[MAX_PATH] = {0};
    SendMessage(nppData._nppHandle, NPPM_GETNATIVELANGFILENAME, MAX_PATH, (LPARAM)langFilePath);
    
    if (strlen(langFilePath) == 0) return;

    // 2. Extraire juste le nom du fichier ("spanish.xml" -> "spanish")
    std::string path(langFilePath);
    size_t posSlash = path.find_last_of("\\/");
    std::string fileName = (posSlash != std::string::npos) ? path.substr(posSlash + 1) : path;
    
    size_t posDot = fileName.find_last_of(".");
    if (posDot != std::string::npos) {
        fileName = fileName.substr(0, posDot);
    }

    if (fileName == "nativeLang") fileName = "english";

    // 3. LE TEST
    if (!fileName.empty() && fileName != g_ActiveLang) {
        
        g_ActiveLang = fileName;
        
        PyObject* pCM = PyImport_ImportModule("config_manager");
        if (pCM) {
            PyObject* pFunc = PyObject_GetAttrString(pCM, "synchroniser_langue_npp");
            if (pFunc) {
                PyObject* pArgs = PyTuple_Pack(1, PyUnicode_FromString(g_ActiveLang.c_str()));
                PyObject* pResult = PyObject_CallObject(pFunc, pArgs);
                
                if (pResult && PyUnicode_Check(pResult)) {
                    const char* rawMsg = PyUnicode_AsUTF8(pResult);
                    if (rawMsg && strlen(rawMsg) > 0) {
                        int len = MultiByteToWideChar(CP_UTF8, 0, rawMsg, -1, NULL, 0);
                        std::wstring wMsg(len, 0);
                        MultiByteToWideChar(CP_UTF8, 0, rawMsg, -1, &wMsg[0], len);
                        
                        // CORRECTION ÉCRAN : La boîte s'affichera au centre de Notepad++ !!
                        MessageBoxW(nppData._nppHandle, wMsg.c_str(), L"Assistant IA", MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
                    }
                    Py_DECREF(pResult);
                } else if (pResult) {
                    Py_DECREF(pResult);
                }
                Py_DECREF(pArgs);
                Py_DECREF(pFunc);
            }
            Py_DECREF(pCM);
        }
    }
}

extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
    // Bloc 1 : Initialisation au démarrage
    if (notifyCode->nmhdr.code == NPPN_READY) {
        
        // --- 1. Paramètres et Console ---
        LoadSettings();
        CheckFirstRunAndSetup();
        
        g_hConsole = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_CONSOLE_PANEL), nppData._nppHandle, (DLGPROC)DlgProcConsole, 0);
        g_cmdIdPoserQuestion = funcFunc[7]._cmdID; 
        
        // --- 2. Subclassing ---
        SetWindowSubclass(nppData._scintillaMainHandle, ScintillaSubclassProc, 1, 0);
        SetWindowSubclass(nppData._scintillaSecondHandle, ScintillaSubclassProc, 2, 0);
        SetWindowSubclass(nppData._nppHandle, NppSubclassProc, 3, 0);
        
        if (g_hConsole) {
            tTbData tbData = {0}; 
            tbData.hClient = g_hConsole;
            tbData.pszName = L"Console AssistantIA";
            tbData.dlgID = IDD_CONSOLE_PANEL;
            tbData.uMask = DWS_DF_CONT_BOTTOM | DWS_ICONTAB | DWS_ICONBAR;
            tbData.pszModuleName = L"AssistantIA.dll";
            
            ::SendMessage(nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&tbData);
            
            if (g_ConsoleVisible) {
                ::SendMessage(nppData._nppHandle, NPPM_DMMSHOW, 0, (LPARAM)g_hConsole);
                ::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcFunc[3]._cmdID, TRUE);
            } else {
                ::SendMessage(nppData._nppHandle, NPPM_DMMHIDE, 0, (LPARAM)g_hConsole);
                ::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcFunc[3]._cmdID, FALSE);
            }
        }
        
        // --- (LES BLOCS PYTHON ONT ÉTÉ DÉPLACÉS DANS getFuncsArray !) ---

        // --- 3. LOGS DE FIN D'INITIALISATION ---
        if (g_hConsole) {
            LogToConsole(_T_MSG(L"MSG_INFO_PLUGIN_ASS_f6a2", L"[INFO] - Plugin AssistantIA prêt et configuré."));
        }

        wchar_t wLangFinal[16];
        MultiByteToWideChar(CP_UTF8, 0, g_ActiveLang.c_str(), -1, wLangFinal, 16);
        
        std::wstring msgLog = _T_MSG(L"MSG_INFO_LANG_CIBLE", L"[INFO] Lang cible : ");
        msgLog += wLangFinal;
        msgLog += _T_MSG(L"MSG_SYNCHRONISEE_", L" synchronisée.");
        LogToConsole(msgLog.c_str());
    }
    
    // reprise focus après avoir fermé la fenêtre de configuration N++ : test changement de lang
    else if (notifyCode->nmhdr.code == NPPN_BUFFERACTIVATED) {
        VerifierChangementLangueUI();
    }

    // Bloc 3 : Fermeture du plugin
    else if (notifyCode->nmhdr.code == NPPN_SHUTDOWN) {
        
        // --- MISE À JOUR DU CACHE DE LANGUE JUSTE AVANT DE QUITTER ---
        char nppLangPath[MAX_PATH] = {0}; 
        ::SendMessageA(nppData._nppHandle, NPPM_GETNATIVELANGFILENAME, MAX_PATH, (LPARAM)nppLangPath);
        
        std::string langueFermeture = "french"; // Fallback
        if (strlen(nppLangPath) > 0) {
            std::string pathStr(nppLangPath);
            size_t lastSlash = pathStr.find_last_of("\\/");
            if (lastSlash != std::string::npos) pathStr = pathStr.substr(lastSlash + 1);
            size_t lastDot = pathStr.find_last_of(".");
            if (lastDot != std::string::npos) pathStr = pathStr.substr(0, lastDot);
            
            std::transform(pathStr.begin(), pathStr.end(), pathStr.begin(), ::tolower);
            if (!pathStr.empty()) langueFermeture = pathStr;
        }

        // On écrase le fichier cache avec la langue active à la fermeture
        wchar_t szAppData[MAX_PATH];
        GetEnvironmentVariableW(L"APPDATA", szAppData, MAX_PATH);
        std::wstring cachePath = std::wstring(szAppData) + L"\\Notepad++\\plugins\\config\\AssistantIA\\last_lang.txt";
        
        std::ofstream cacheFile(cachePath, std::ios::trunc); // std::ios::trunc force l'écrasement
        if (cacheFile.is_open()) {
            cacheFile << langueFermeture;
            cacheFile.close();
        }
        // -------------------------------------------------------------

        // Arrêt de Python et nettoyage
        if (Py_IsInitialized()) {
            Py_Finalize();
        }
        RemoveWindowSubclass(nppData._scintillaMainHandle, ScintillaSubclassProc, 1);
        RemoveWindowSubclass(nppData._scintillaSecondHandle, ScintillaSubclassProc, 2);
        RemoveWindowSubclass(nppData._nppHandle, NppSubclassProc, 3);
    }
}

void ToggleConsole() {
    if (!g_hConsole) return;
    
    // Inversion de l'état
    g_ConsoleVisible = !g_ConsoleVisible;
    WriteIniBool(L"ConsoleVisible", g_ConsoleVisible);
    
    if (g_ConsoleVisible) {
        // Apparition du panneau
        ::SendMessage(nppData._nppHandle, NPPM_DMMSHOW, 0, (LPARAM)g_hConsole);
        // Ajout de la coche (checkmark) à côté du texte dans le menu Plugins
        ::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcFunc[3]._cmdID, TRUE);
        } else {
        // Disparition du panneau
        ::SendMessage(nppData._nppHandle, NPPM_DMMHIDE, 0, (LPARAM)g_hConsole);
        // Retrait de la coche
        ::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcFunc[3]._cmdID, FALSE);
    }
}

// Fonction pour chercher et remplacer du texte dans un std::wstring
void ReplaceAll(std::wstring& source, const std::wstring& from, const std::wstring& to) {
    if (from.empty()) return;
    size_t start_pos = 0;
    while ((start_pos = source.find(from, start_pos)) != std::wstring::npos) {
        source.replace(start_pos, from.length(), to);
        start_pos += to.length(); // On avance pour éviter les boucles infinies
    }
}

// Fonction utilitaire pour peupler la ListBox
void PopulateScriptList(HWND hList) {
    // 1. Vider la liste actuelle
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    
    // 2. Construire le chemin APPDATA
    wchar_t szAppData[MAX_PATH];
    GetEnvironmentVariableW(L"APPDATA", szAppData, MAX_PATH);
    std::wstring folderPath = std::wstring(szAppData) + L"\\Notepad++\\plugins\\config\\AssistantIA\\providers";
    std::wstring searchPath = folderPath + L"\\*.py";
    
    // 3. Scanner le répertoire
    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &fd);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                // Ajouter le nom du fichier à la ListBox
                SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)fd.cFileName);
            }
        } while (FindNextFileW(hFind, &fd));
        FindClose(hFind);
    }
}

INT_PTR CALLBACK DlgProcNouveauScript(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    
    switch (Message) {
        case WM_INITDIALOG:
        // --- TRADUCTION DYNAMIQUE DES LABELS ET BOUTONS ---
        SetWindowTextW(hWnd, _T_UI(IDD_NEW_SCRIPT_DLG));
        SetDlgItemTextW(hWnd, IDC_STATIC_NAME, _T_UI(IDC_STATIC_NAME));
        SetDlgItemTextW(hWnd, IDC_STATIC_PROVIDER, _T_UI(IDC_STATIC_PROVIDER));
        SetDlgItemTextW(hWnd, IDC_STATIC_API_URL, _T_UI(IDC_STATIC_API_URL));
        SetDlgItemTextW(hWnd, IDC_STATIC_MODEL_URL, _T_UI(IDC_STATIC_MODEL_URL));
        SetDlgItemTextW(hWnd, IDC_STATIC_REG_URL, _T_UI(IDC_STATIC_REG_URL));
        SetDlgItemTextW(hWnd, IDC_STATIC_API_KEY, _T_UI(IDC_STATIC_API_KEY));
        SetDlgItemTextW(hWnd, IDOK_DO_SCRIPT, _T_UI(IDOK_DO_SCRIPT));
        SetDlgItemTextW(hWnd, IDCLOSE, _T_UI(IDCLOSE));
        return TRUE;
        
        case WM_COMMAND:
        if (LOWORD(wParam) == IDOK_DO_SCRIPT) {
            // Étape 0 : Signaler que le clic est bien intercepté
            AppendTextToConsoleW(_T_MSG(L"MSG_NOUVEAUSCRIPT_C_0b1f",L"[NouveauScript] Clic sur 'Créer le script' détecté.\r\n"));
            
            // 1. Récupération des textes saisis
            wchar_t wLocalName[256] = {0}, wProvider[256] = {0};
            wchar_t wApiUrl[512] = {0}, wModelUrl[512] = {0}, regUrlW[512] = {0}, wApiKey[256] = {0};
            
            GetDlgItemTextW(hWnd, IDC_EDIT_LOCAL_NAME, wLocalName, 256);
            GetDlgItemTextW(hWnd, IDC_EDIT_PROVIDER, wProvider, 256);
            GetDlgItemTextW(hWnd, IDC_EDIT_API_URL, wApiUrl, 512);
            GetDlgItemTextW(hWnd, IDC_EDIT_MODEL_URL, wModelUrl, 512);
            GetDlgItemTextW(hWnd, IDC_EDIT_REG_URL, regUrlW, 512);
            GetDlgItemTextW(hWnd, IDC_EDIT_API_KEY, wApiKey, 256);
            
            if (wcslen(wLocalName) == 0) {
                MessageBoxW(hWnd, _T_MSG(L"MSG_LE_CHAMP_NOM_AF_1db9",L"Le champ 'Nom affiché' est obligatoire."), L"Information", MB_OK | MB_ICONWARNING);
                return TRUE;
            }
            
            // 2. Préparation des chemins
            std::wstring templatePath = std::wstring(g_pluginDir) + L"\\providers\\template.py";
            
            wchar_t szAppData[MAX_PATH];
            GetEnvironmentVariableW(L"APPDATA", szAppData, MAX_PATH);
            std::wstring folderPath = std::wstring(szAppData) + L"\\Notepad++\\plugins\\config\\AssistantIA\\providers";
            
            // Sécurité : On s'assure que le dossier utilisateur existe
            _wmkdir(folderPath.c_str());
            
            std::wstring fileName = wLocalName;
            ReplaceAll(fileName, L" ", L"_"); // Remplace les espaces par des underscores
            std::wstring outPath = folderPath + L"\\" + fileName + L".py";
            
            // Logs des chemins dans la console
            AppendTextToConsoleW((L"[NouveauScript] Source : " + templatePath + L"\r\n").c_str());
            AppendTextToConsoleW((L"[NouveauScript] Destination : " + outPath + L"\r\n").c_str());
            
            // 3. Ta stratégie : Copie directe via l'API Windows
            AppendTextToConsoleW(L"[NouveauScript] Tentative de copie du fichier...\r\n");
            if (CopyFileW(templatePath.c_str(), outPath.c_str(), FALSE)) {
                AppendTextToConsoleW(L"[NouveauScript] Copie réussie ! Passage à l'édition du fichier...\r\n");
                
                // 4. Édition de la copie. On utilise std::string (UTF-8) car Python adore l'UTF-8
                // et cela évite les bugs d'encodage de l'implémentation large (wchar_t) de Microsoft.
                std::ifstream inFile(outPath, std::ios::binary);
                if (inFile.is_open()) {
                    std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
                    inFile.close();
                    
                    // Lambda locale pour remplacer dans un std::string
                    auto ReplaceAllA = [](std::string& source, const std::string& from, const std::string& to) {
                        if (from.empty()) return;
                        size_t start_pos = 0;
                        while ((start_pos = source.find(from, start_pos)) != std::string::npos) {
                            source.replace(start_pos, from.length(), to);
                            start_pos += to.length();
                        }
                    };
                    
                    // Lambda pour convertir proprement nos wchar_t de l'interface en UTF-8 pour le script Python
                    auto ToUTF8 = [](const wchar_t* wstr) -> std::string {
                        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
                        std::string strTo(size_needed - 1, 0);
                        WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &strTo[0], size_needed, NULL, NULL);
                        return strTo;
                    };
                    
                    // Remplacement des balises
                    ReplaceAllA(content, "{{API_LOCAL_NAME}}", ToUTF8(wLocalName));
                    ReplaceAllA(content, "{{PROVIDER_NAME}}", ToUTF8(wProvider));
                    ReplaceAllA(content, "{{API_URL}}", ToUTF8(wApiUrl));
                    ReplaceAllA(content, "{{API_URL_MODEL}}", ToUTF8(wModelUrl));
                    ReplaceAllA(content, "{{API_URL_REGISTRATION}}", ToUTF8(regUrlW));
                    ReplaceAllA(content, "{{API_KEY}}", ToUTF8(wApiKey));
                    
                    // Réécriture du fichier personnalisé
                    std::ofstream outFile(outPath, std::ios::binary);
                    if (outFile.is_open()) {
                        outFile << content;
                        outFile.close();
                        
                        
                        AppendTextToConsoleW(L"[NouveauScript] Remplacement des balises effectué avec succès.\r\n");
                        // --- PASSAGE DE RELAIS À PYTHON POUR LE JSON ---
                        char provA[256];
                        char keyA[256];
                        char regUrlA[512];
                        WideCharToMultiByte(CP_UTF8, 0, wLocalName, -1, provA, 256, NULL, NULL);
                        WideCharToMultiByte(CP_UTF8, 0, wApiKey, -1, keyA, 256, NULL, NULL);
                        WideCharToMultiByte(CP_UTF8, 0, regUrlW, -1, regUrlA, 512, NULL, NULL);
                        
                        PyObject* pMod = PyImport_ImportModule("config_manager");
                        if (pMod) {
                            PyObject* pFunc = PyObject_GetAttrString(pMod, "ajouter_nouveau_provider");
                            if (pFunc && PyCallable_Check(pFunc)) {
                                PyObject* pArgs = PyTuple_Pack(3, PyUnicode_FromString(provA), PyUnicode_FromString(keyA), PyUnicode_FromString(regUrlA));
                                PyObject* pRes = PyObject_CallObject(pFunc, pArgs);
                                if (pRes) Py_DECREF(pRes);
                                Py_DECREF(pArgs);
                                Py_DECREF(pFunc);
                            }
                            Py_DECREF(pMod);
                        }
                        MessageBoxW(hWnd, _T_MSG(L"MSG_NOUVEAU_SCRIPT__a3b9",L"Nouveau script d'API créé avec succès !"), L"AssistantIA", MB_OK | MB_ICONINFORMATION);
                        
                        // Ouverture immédiate dans Notepad++
                        ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)outPath.c_str());
                        EndDialog(hWnd, IDOK);
                        } else {
                        AppendTextToConsoleW(_T_MSG(L"MSG_ERREUR_IMPOSSIB_ff66",L"[Erreur] Impossible de réécrire dans le fichier copié.\r\n"));
                    }
                    } else {
                    AppendTextToConsoleW(_T_MSG(L"MSG_ERREUR_IMPOSSIB_ed5c",L"[Erreur] Impossible de lire le fichier après sa copie.\r\n"));
                }
                } else {
                // Si CopyFileW échoue, on récupère le code d'erreur exact de Windows
                DWORD error = GetLastError();
                wchar_t errBuf[256];
                swprintf_s(errBuf, 256, _T_MSG(L"MSG_ERREUR_ECHEC_DE_4b88",L"[Erreur] Échec de CopyFileW. Code erreur Windows : %lu\r\n"), error);
                AppendTextToConsoleW(errBuf);
                MessageBoxW(hWnd, _T_MSG(L"MSG_ECHEC_DE_LA_COP_7d4f",L"Échec de la copie. Vérifie que 'template.py' est bien présent à côté de la DLL du plugin."), L"Erreur", MB_OK | MB_ICONERROR);
            }
            return TRUE;
        }
        else if (LOWORD(wParam) == IDCLOSE) {
            EndDialog(hWnd, IDCLOSE);
            return TRUE;
        }else if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hWnd, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

INT_PTR CALLBACK DlgProcManageScripts(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    switch (Message) {
        case WM_INITDIALOG:
        // Remplir la liste dès l'ouverture
        // --- TRADUCTION DYNAMIQUE ---
        SetWindowTextW(hWnd, _T_UI(IDD_MANAGE_SCRIPTS));
        SetDlgItemTextW(hWnd, IDC_BTN_CREATE_SCRIPT, _T_UI(IDC_BTN_CREATE_SCRIPT));
        SetDlgItemTextW(hWnd, IDC_BTN_EDIT_SCRIPT, _T_UI(IDC_BTN_EDIT_SCRIPT));
        SetDlgItemTextW(hWnd, IDC_BTN_DELETE_SCRIPT, _T_UI(IDC_BTN_DELETE_SCRIPT));
        SetDlgItemTextW(hWnd, IDCLOSE, _T_UI(IDCLOSE));
        PopulateScriptList(GetDlgItem(hWnd, IDC_LIST_SCRIPTS));
        return TRUE;
        
        case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case IDC_BTN_CREATE_SCRIPT:
            // Appel de ta fenêtre de création existante
            DialogBox(g_hInst, MAKEINTRESOURCE(IDD_NEW_SCRIPT_DLG), hWnd, DlgProcNouveauScript);
            // Rafraîchir après retour
            PopulateScriptList(GetDlgItem(hWnd, IDC_LIST_SCRIPTS));
            EndDialog(hWnd, IDOK_DO_SCRIPT);
            break;
            
            case IDC_BTN_EDIT_SCRIPT: {
                HWND hList = GetDlgItem(hWnd, IDC_LIST_SCRIPTS);
                int sel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    wchar_t fileName[256];
                    SendMessage(hList, LB_GETTEXT, sel, (LPARAM)fileName);
                    
                    // Chemin complet
                    wchar_t szAppData[MAX_PATH];
                    GetEnvironmentVariableW(L"APPDATA", szAppData, MAX_PATH);
                    std::wstring path = std::wstring(szAppData) + L"\\Notepad++\\plugins\\config\\AssistantIA\\providers\\" + fileName;
                    
                    // Ouverture dans Npp
                    ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)path.c_str());
                    EndDialog(hWnd, IDOK); // On ferme après l'ouverture
                }
                break;
            }
            
            case IDC_BTN_DELETE_SCRIPT: {
                HWND hList = GetDlgItem(hWnd, IDC_LIST_SCRIPTS);
                int sel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);
                if (sel != LB_ERR) {
                    if (MessageBoxW(hWnd, _T_MSG(L"MSG_SUPPRIMER_CE_SC_2b77",L"Supprimer ce script ?"), L"Confirmation", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                        wchar_t fileName[256];
                        SendMessage(hList, LB_GETTEXT, sel, (LPARAM)fileName);
                        
                        wchar_t szAppData[MAX_PATH];
                        GetEnvironmentVariableW(L"APPDATA", szAppData, MAX_PATH);
                        std::wstring path = std::wstring(szAppData) + L"\\Notepad++\\plugins\\config\\AssistantIA\\providers\\" + fileName;
                        if (g_bForceCloseOnDelete) {
                            // Comportement 1 : Fermeture forcée et silencieuse
                            ::SendMessage(nppData._nppHandle, NPPM_CLOSEFILE, 0, (LPARAM)path.c_str());
                            } else {
                            // Comportement 2 : On laisse Npp gérer la notif de suppression
                            ::SendMessage(nppData._nppHandle, NPPM_RELOADFILE, 0, (LPARAM)path.c_str());
                        }
                        
                        if (DeleteFileW(path.c_str())) {
                            std::wstring msg = std::wstring(_T_MSG(L"MSG_SUCCES_FICHIER__dc7c",L"[SUCCÈS] Fichier supprimé : ")) + path;
                            LogToConsole(msg.c_str());
                            
                            // --- SYNCHRONISATION JSON ---
                            wchar_t providerName[256];
                            wcscpy_s(providerName, fileName);
                            wchar_t* dot = wcsrchr(providerName, L'.');
                            if (dot) *dot = L'\0';
                            
                            char provA[256];
                            WideCharToMultiByte(CP_UTF8, 0, providerName, -1, provA, 256, NULL, NULL);
                            
                            PyObject* pMod = PyImport_ImportModule("config_manager");
                            if (pMod) {
                                PyObject* pFunc = PyObject_GetAttrString(pMod, "supprimer_provider_config");
                                if (pFunc && PyCallable_Check(pFunc)) {
                                    PyObject* pArgs = PyTuple_Pack(1, PyUnicode_FromString(provA));
                                    PyObject* pRes = PyObject_CallObject(pFunc, pArgs);
                                    if (pRes) Py_DECREF(pRes);
                                    Py_DECREF(pArgs);
                                    Py_DECREF(pFunc);
                                }
                                Py_DECREF(pMod);
                            }
                            
                            PopulateScriptList(hList); // Rafraîchir l'UI
                            } else {
                            MessageBoxW(hWnd, _T_MSG(L"MSG_ERREUR_LORS_DE__be23",L"Erreur lors de la suppression."), L"Erreur", MB_OK | MB_ICONERROR);
                        }
                    }
                }
                break;
            }
            
            case IDCLOSE:
            EndDialog(hWnd, IDCLOSE);
            break;
            
            case IDCANCEL:
            EndDialog(hWnd, IDCANCEL);
            break;
        }
        break;
    }
    return FALSE;
}

void CreerNouveauScript() {
    // Affiche la boîte de dialogue de façon "modale" (bloque Notepad++ tant qu'elle est ouverte)
    // g_hInst est la variable globale qui représente ton plugin
    DialogBoxParamW(g_hInst, MAKEINTRESOURCE(IDD_NEW_SCRIPT_DLG), nppData._nppHandle, DlgProcNouveauScript, 0);
}

void GererScriptsUtilisateur() {
    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_MANAGE_SCRIPTS), nppData._nppHandle, DlgProcManageScripts);
}

void LancerAssistant() {
    if (g_hChatDlg != NULL) {
        SetActiveWindow(g_hChatDlg);
        return;
    }
    
    HWND hwndParent = nppData._nppHandle; 
    
    // On appelle directement le template du fichier .rc
    g_hChatDlg = CreateDialogParamW(g_hInst, MAKEINTRESOURCE(IDD_CHAT_DLG), hwndParent, DlgProcChat, 0);
    
    if (g_hChatDlg) {
        ShowWindow(g_hChatDlg, SW_SHOW);
    }
}

void OuvrirConfiguration() {
    DialogBoxParamW(hInst, MAKEINTRESOURCE(IDD_MAIN_CONFIG_DLG), nppData._nppHandle, DlgProcMainConfig, 0);
}

std::string MarkdownToRtfCPlusPlus(std::string text, bool singledoc) {
    if (text.empty()) return "";

    // 1. Échapper les caractères réservés RTF
    text = std::regex_replace(text, std::regex(R"(\\)"), R"(\\)");
    text = std::regex_replace(text, std::regex(R"(\{)"), R"(\{)");
    text = std::regex_replace(text, std::regex(R"(\})"), R"(\})");

    // 2. Analyse ligne par ligne
    std::stringstream in(text);
    std::string line;
    std::string processedText = "";
    bool inTable = false;
    bool inList = false;

    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();

        // --- A. GESTION DES LIGNES DE SÉPARATION ---
        std::regex hr_regex(R"(^[ \t]*(?:___|---|==|\*\*\*)[ \t]*$)");
        if (std::regex_match(line, hr_regex)) {
            if (inTable) { processedText += "\\pard\\fs22\n"; inTable = false; }
            if (inList)  { processedText += "\\pard\\fs22\n"; inList = false; }
            processedText += "\\pard\\cf6\\qc\\fs24 ─── ❖ ───\\par\\pard\\fs22\\cf1\n";
            continue;
        }

        // --- B. GESTION DES TABLEAUX ---
        size_t firstPipe = line.find('|');
        if (firstPipe != std::string::npos) {
            bool isSeparator = true;
            for (char c : line) {
                if (c != '|' && c != '-' && c != ':' && c != ' ' && c != '\t') {
                    isSeparator = false; break;
                }
            }
            if (isSeparator) continue;

            if (inList) { processedText += "\\pard\\fs22\n"; inList = false; }

            std::vector<std::string> cells;
            std::stringstream lineStream(line);
            std::string cell;
            while (std::getline(lineStream, cell, '|')) {
                size_t f = cell.find_first_not_of(" \t");
                if (f == std::string::npos) cells.push_back("");
                else {
                    size_t l = cell.find_last_not_of(" \t");
                    cells.push_back(cell.substr(f, (l - f + 1)));
                }
            }
            if (!cells.empty() && cells.front().empty()) cells.erase(cells.begin());
            if (!cells.empty() && cells.back().empty()) cells.pop_back();

            if (!cells.empty()) {
                std::string rtfRow = "\\trowd\\trgaph70\\trleft108";
                int currentX = 2400;
                for (size_t i = 0; i < cells.size(); ++i) {
                    rtfRow += "\\cellx" + std::to_string(currentX);
                    currentX += 2400;
                }
                rtfRow += " ";
                for (const auto& cText : cells) rtfRow += cText + "\\cell ";
                rtfRow += "\\row\n";
                processedText += rtfRow;
                inTable = true;
                continue;
            }
        }
        if (inTable) { processedText += "\\pard\\fs22\n"; inTable = false; }

        // --- C/D. LISTES ---
        std::smatch listMatch;
        std::regex unordered_list_regex(R"(^[ \t]*[-*+]\s+(.*))");
        std::regex ordered_list_regex(R"(^[ \t]*(\d+)\.\s+(.*))");
        
        if (std::regex_match(line, listMatch, unordered_list_regex)) {
            processedText += "\\pard\\li240\\cf5 \\b \\bullet \\b0\\cf1  " + listMatch[1].str() + "\\par\n";
            inList = true; continue;
        }
        if (std::regex_match(line, listMatch, ordered_list_regex)) {
            processedText += "\\pard\\cf5 \\li240\\b " + listMatch[1].str() + ". \\b0\\cf1  " + listMatch[2].str() + "\\par\n";
            inList = true; continue;
        }
        if (inList) { processedText += "\\pard\\fs22\n"; inList = false; }

        processedText += line + "\\par\n";
    }
    text = processedText;

    // 3/4. Traitement global
    text = std::regex_replace(text, std::regex(R"(### (.*?)\\par)"), "\\cf6 \\b\\fs24 $1\\b0\\fs22\\cf1\\par");
    text = std::regex_replace(text, std::regex(R"(## (.*?)\\par)"), "\\cf4 \\b\\fs28 $1\\b0\\fs22\\cf1\\par");
    text = std::regex_replace(text, std::regex(R"(# (.*?)\\par)"), "\\qc \\cf8 \\b\\fs32 $1\\b0\\fs22\\cf1\\par \\ql");
    text = std::regex_replace(text, std::regex(R"(\*\*(.*?)\*\*)"), "\\cf5\\b $1\\b0 \\cf1 ");
    text = std::regex_replace(text, std::regex(R"(\*(.*?)\*)"), "\\i $1\\i0 ");
    text = std::regex_replace(text, std::regex(R"(`(.*?)`)"), "\\f1\\highlight3 $1\\highlight0\\f0 ");

    // 5. Encapsulation conditionnelle
    if (singledoc) {
        std::string rtfHeader = "{\\rtf1\\ansi\\ansicpg65001\\uc1\\deff0{\\fonttbl{\\f0 Segoe UI;}{\\f1 Consolas;}}{\\colortbl;"
                                "\\red0\\green0\\blue0;\\red40\\green44\\blue52;\\red240\\green240\\blue240;"
                                "\\red0\\green120\\blue215;\\red255\\green150\\blue0;\\red143\\green185\\blue66;"
                                "\\red231\\green76\\blue60;\\red153\\green79\\blue226;}\\fs22\n";
        return rtfHeader + text + "\n}";
    }
    // ENCAPSULATION LIGHT :
    // On renvoie juste le texte dans des accolades. 
    // C'est assez pour que EM_STREAMIN comprenne que c'est du RTF, 
    // et il utilisera la table des couleurs déjà chargée par InitRichEditColors.
    return "{\\rtf1 " + text + "}";
}

// Fonction pour charger le fichier d'aide depuis le dossier 'lang' selon la langue active
std::string ChargerFichierAideMd() {
    wchar_t path[MAX_PATH];
    wchar_t szAppData[MAX_PATH];
    GetEnvironmentVariableW(L"APPDATA", szAppData, MAX_PATH);
    
    // 1. Pointage vers APPDATA
    swprintf_s(path, MAX_PATH, L"%s\\Notepad++\\plugins\\config\\AssistantIA\\lang\\%S.help.md", szAppData, g_ActiveLang.c_str()); 
    
    std::ifstream file(path, std::ios::binary);
    
    // 2. Fallbacks
    if (!file.is_open()) {
        swprintf_s(path, MAX_PATH, L"%s\\Notepad++\\plugins\\config\\AssistantIA\\lang\\french.help.md", szAppData);
        file.open(path, std::ios::binary);
        
        if (!file.is_open()) {
            const wchar_t* wmsg = _T_MSG(L"MSG_ERREUR_LE_FICHI_f5fb", L"# Erreur\nLe fichier d'aide est introuvable.");
            
            // Calcul de la taille requise pour la conversion en UTF-8
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, wmsg, -1, NULL, 0, NULL, NULL);
            std::string utf8_msg(size_needed - 1, 0);
            WideCharToMultiByte(CP_UTF8, 0, wmsg, -1, &utf8_msg[0], size_needed, NULL, NULL);
            
            return utf8_msg;
        }
    }
    
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (content.size() >= 3 && (unsigned char)content[0] == 0xEF && (unsigned char)content[1] == 0xBB && (unsigned char)content[2] == 0xBF) {
        content.erase(0, 3);
    }
    return content;
}

INT_PTR CALLBACK DlgProcAide(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_INITDIALOG: {
            bool isFirstRun = (lParam == 1);
            
            // CORRECTION : Utilisation de l'ID unifié IDC_HELP_BTN_OK
            HWND hBtn = GetDlgItem(hWnd, IDC_HELP_BTN_OK);
            HWND hRichEdit = GetDlgItem(hWnd, IDC_HELP_RICHEDIT);
            
            if (isFirstRun) {
                SetWindowTextW(hBtn, _T_UI(IDC_BTN_MANAGE_API)); 
            } else {
                SetWindowTextW(hBtn, _T_UI(IDCLOSE));           
            }
            
            // Appliquer le padding initial de 16 pixels
            if (hRichEdit) {
                RECT rc;
                GetClientRect(hRichEdit, &rc);
                rc.left   += 16;
                rc.top    += 16;
                rc.right  -= 16;
                rc.bottom -= 16;
                SendMessage(hRichEdit, EM_SETRECT, 0, (LPARAM)&rc);
            }
            
            // Lecture et injection du contenu RTF
            std::string mdContent = ChargerFichierAideMd();
            std::string rtfContent = MarkdownToRtfCPlusPlus(mdContent);
            
            SetWindowTextW(hRichEdit, L"");
            AppendRtfToRichEdit(hRichEdit, rtfContent);
            
            return TRUE;
        }
        
        case WM_SIZE: {
            int width = LOWORD(lParam);   // Nouvelle largeur de la boîte de dialogue
            int height = HIWORD(lParam);  // Nouvelle hauteur de la boîte de dialogue
            
            HWND hRichEdit = GetDlgItem(hWnd, IDC_HELP_RICHEDIT);
            // CORRECTION : Même ID ici pour que hBtn ne soit pas NULL
            HWND hBtn = GetDlgItem(hWnd, IDC_HELP_BTN_OK); 
            
            if (hRichEdit && hBtn) {
                int padding = 20;
                int btnWidth = 120;
                int btnHeight = 30;
                
                // 1. Redimensionner physiquement le RichEdit
                int richWidth = width - (padding * 2);
                int richHeight = height - btnHeight - (padding * 3);
                MoveWindow(hRichEdit, padding, padding, richWidth, richHeight, TRUE);
                
                // 2. Repositionner le bouton "Fermer / Configurer" en bas à droite
                MoveWindow(hBtn, width - btnWidth - padding, height - btnHeight - padding, btnWidth, btnHeight, TRUE);
                
                // 3. CORRECTION CRUCIAL REZIZE : On recalcule et ré-applique le padding interne 
                // basé sur la NOUVELLE taille du RichEdit pour recalculer le Word Wrap
                RECT rc;
                GetClientRect(hRichEdit, &rc);
                rc.left   += 16;
                rc.top    += 16;
                rc.right  -= 16;
                rc.bottom -= 16;
                SendMessage(hRichEdit, EM_SETRECT, 0, (LPARAM)&rc);
            }
            return TRUE;
        }
        
        case WM_COMMAND: {
            if (LOWORD(wParam) == IDC_HELP_BTN_OK) {
                wchar_t btnText[64];
                GetWindowTextW(GetDlgItem(hWnd, IDC_HELP_BTN_OK), btnText, 64);
                
                EndDialog(hWnd, IDC_HELP_BTN_OK);
                
                if (wcscmp(btnText, _T_MSG(L"IDC_BTN_MANAGE_API", L"Configurer")) == 0) {
                    lancerAssistantConfig(); 
                }
                return TRUE;
            }
            if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hWnd, IDCANCEL);
                return TRUE;
            }
            break;
        }
    }
    return FALSE;
}

void CheckFirstRunAndSetup() {
    wchar_t path[MAX_PATH];
    GetIniPath(path);
    
    // Vérifier si le fichier existe
    DWORD dwAttrib = GetFileAttributesW(path);
    
    // Si le fichier n'existe pas, c'est le "First Run"
    if (dwAttrib == INVALID_FILE_ATTRIBUTES) {
        
        // 1. Création forcée du fichier (WritePrivateProfileString crée le fichier)
        WritePrivateProfileStringW(L"Settings", L"FirstRun", L"0", path);
        WritePrivateProfileStringW(L"Settings", L"ConsoleVisible", L"1", path);
        WritePrivateProfileStringW(L"Settings", L"ForceClose", L"0", path);
        WritePrivateProfileStringW(L"Settings", L"SaveSelectionLo", L"1", path);
        
        LogToConsole(_T_MSG(L"MSG_INFO_PREMIER_LA_cd18", L"[INFO] Premier lancement détecté."));
        
        // 2. Lancement de la fenêtre d'Aide en mode "Wizard"
        // Le dernier paramètre '1' est notre signal "First Run"
        DialogBoxParamW(g_hInst, MAKEINTRESOURCE(IDD_HELP_DLG), nppData._nppHandle, DlgProcAide, 1);
    }
}

void AfficherAide() {
    DialogBoxParamW(g_hInst, MAKEINTRESOURCE(IDD_HELP_DLG), nppData._nppHandle, DlgProcAide, 0);
}

INT_PTR CALLBACK VersionDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG:
        SetWindowText(hWnd, _T_MSG(L"MSG_A_PROPOS_DE_ASS_cb57", L"À propos de AssistantIA"));
        g_hBrushFond = CreateSolidBrush(RGB(200, 225, 255)); // fond bleu clair
        g_hBrushControle = CreateSolidBrush(RGB(255, 255, 255));
        
        return TRUE;  // ← doit retourner TRUE pour WM_INITDIALOG

    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        HWND hwndCtrl = (HWND)lParam;

        if (GetDlgCtrlID(hwndCtrl) == IDC_VERSION_TEXT)
        {
            // Couleur du texte
            SetTextColor(hdcStatic, RGB(0, 50, 100));   // bleu foncé
            // Couleur de fond du contrôle
            SetBkColor(hdcStatic, RGB(255, 255, 255));  // blanc
            SetBkMode(hdcStatic, OPAQUE);
            return (INT_PTR)g_hBrushControle;
        }
        break;
    }

    case WM_CTLCOLORDLG:
    {
        // Fond de toute la boîte de dialogue
        HDC hdcDlg = (HDC)wParam;
        SetBkColor(hdcDlg, RGB(255, 255, 255));
        return (INT_PTR)g_hBrushFond;
    }


    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            DestroyWindow(hWnd);  // non-modal → DestroyWindow, pas EndDialog
            return TRUE;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        return TRUE;
    
    case WM_DESTROY:
    // Libérer le brush à la destruction
        if (g_hBrushFond)     { DeleteObject(g_hBrushFond);     g_hBrushFond     = NULL; }
        if (g_hBrushControle) { DeleteObject(g_hBrushControle); g_hBrushControle = NULL; }
        return TRUE;
    }
    return FALSE;  // ← FALSE par défaut (pas TRUE)
}

void AfficherVersion() {
    // Extraction dynamique de plusieurs clés du bloc VERSIONINFO de la DLL
    std::wstring auteur         = ObtenirInfoDLL(L"CompanyName");
    std::wstring assistant      = ObtenirInfoDLL(L"Assistant");
    std::wstring produit        = ObtenirInfoDLL(L"ProductName");
    std::wstring produitversion = ObtenirInfoDLL(L"ProductVersion");
    std::wstring icoversion     = ObtenirInfoDLL(L"icoVersion");
    std::wstring version        = ObtenirInfoDLL(L"FileVersion");
    std::wstring description    = ObtenirInfoDLL(L"FileDescription");
    std::wstring copyright      = ObtenirInfoDLL(L"LegalCopyright");
    std::wstring licence        = ObtenirInfoDLL(L"Licence");
    std::wstring dll            = ObtenirInfoDLL(L"InternalName");

    // Fallbacks au cas où le fichier .rc soit mal configuré pendant le dev
    if (version.empty())     version = L"0.1 (Alpha)";
    if (description.empty()) description = L"Plugin Notepad++ propulsé par un moteur Python natif.";

    // 1. Construction du gros message avec les infos de la DLL + tes traductions _T_MSG
    std::wstring msgCorps = L"\n\n" + produit + L" - ";
    msgCorps += produitversion + L"\n\n";
    msgCorps += description + L"\n\n";
    msgCorps += auteur + L" | Cyril Griboval\n";
    msgCorps += L"Assistant:" +assistant + L"\n\n";
    msgCorps += icoversion+dll + L"\n\n";
    msgCorps += licence + L"\n\n";
    msgCorps += copyright + L"\n\n\n\n";
    
    // Ajout d'une ligne de traduction fixe pour l'infrastructure
    msgCorps += _T_MSG(L"MSG_SYST_ME_DE_CONS_b002", L"Système de console et de configuration opérationnel.");

    // 2. Affichage
    extern HINSTANCE g_hInstance;
    HINSTANCE hInstance = g_hInstance;
    if (hInstance == NULL) {
        DWORD errorCode = GetLastError();
        std::wstring errorText = L"Erreur de récupération du handle de module. Code d'erreur: ";
        errorText += std::to_wstring(errorCode);
        MessageBox(NULL, errorText.c_str(), L"Erreur", MB_ICONERROR);
        return;
    }

    HWND hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_VERSION), nppData._nppHandle, VersionDlgProc);
    if (hWnd == NULL) {
        DWORD errorCode = GetLastError();
        std::wstring errorText = L"Erreur de création de la boîte de dialogue. Code d'erreur: ";
        errorText += std::to_wstring(errorCode);
        MessageBox(NULL, errorText.c_str(), L"Erreur", MB_ICONERROR);
        return;
    }
     else {
        // Afficher le message dans la fenêtre de dialogue
        SetDlgItemText(hWnd, IDC_VERSION_TEXT, msgCorps.c_str());
        ShowWindow(hWnd, SW_SHOW);
    }
}

LRESULT CALLBACK ChatButtonSubclass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {
        case WM_MOUSEMOVE:
        {
            // Si la souris entre sur le bouton pour la première fois
            if (!GetPropW(hWnd, L"MouseHovering"))
            {
                SetPropW(hWnd, L"MouseHovering", (HANDLE)1);
                
                // On demande à être notifié quand la souris quitte CE bouton précis
                TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, hWnd, 0 };
                TrackMouseEvent(&tme);
                
                // On force le bouton à se repeindre immédiatement avec la couleur Hover
                InvalidateRect(hWnd, NULL, FALSE);
            }
            break;
        }
        
        case WM_MOUSELEAVE:
        {
            // La souris a quitté le bouton
            RemovePropW(hWnd, L"MouseHovering");
            // On force le bouton à se repeindre avec sa couleur normale
            InvalidateRect(hWnd, NULL, FALSE);
            break;
        }
        
        case WM_SETCURSOR:
        {
            // On force le curseur MAIN (Main de survol)
            SetCursor(LoadCursor(NULL, IDC_HAND));
            return TRUE; // On indique à Windows qu'on a géré le curseur
        }
        
        case WM_NCDESTROY:
        {
            // Nettoyage de sécurité en cas de fermeture
            RemovePropW(hWnd, L"MouseHovering");
            RemoveWindowSubclass(hWnd, ChatButtonSubclass, uIdSubclass);
            break;
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK DlgProcChat(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static wchar_t s_providerName[64] = L"IA"; 
    
    switch (message) {
        case WM_INITDIALOG: {
            HWND hwndHistory = GetDlgItem(hWnd, IDC_CHAT_HISTORY);
            HWND hwndInput = GetDlgItem(hWnd, IDC_CHAT_INPUT);
            HWND hwndCombo = GetDlgItem(hWnd, IDC_CHAT_MODEL_COMBO);
            
            AppendRtfToRichEdit(hwndHistory, MarkdownToRtfCPlusPlus(" ", true));

            SendMessageW(hwndInput, EM_SETLIMITTEXT, 20000, 0); 
            CheckDlgButton(hWnd, IDC_CHK_SAVE_CHAT, BST_CHECKED);
            
            SetWindowTextW(hWnd, _T_UI(IDD_CHAT_DLG));
            
            // Les textes des composants (La macro stringifie l'ID toute seule !)
            SetDlgItemTextW(hWnd, IDC_CHAT_STATIC_MODEL, _T_UI(IDC_CHAT_STATIC_MODEL));
            SetDlgItemTextW(hWnd, IDC_CHK_SAVE_CHAT,     _T_UI(IDC_CHK_SAVE_CHAT));
            SetDlgItemTextW(hWnd, IDC_BTN_CLEAR,         _T_UI(IDC_BTN_CLEAR));
            SetDlgItemTextW(hWnd, IDC_BTN_SEND,          _T_UI(IDC_BTN_SEND));            
            
            
            
            
            // --- NOUVEAU : On applique proprement le sous-classement aux deux boutons ---
            SetWindowSubclass(GetDlgItem(hWnd, IDC_BTN_SEND), ChatButtonSubclass, IDC_BTN_SEND, 0);
            SetWindowSubclass(GetDlgItem(hWnd, IDC_BTN_CLEAR), ChatButtonSubclass, IDC_BTN_CLEAR, 0);
            
            wchar_t currentModelW[128] = L"";
            std::vector<std::wstring> listeModeles;

            PyObject* pCM = PyImport_ImportModule("config_manager");
            if (!pCM) {
                AppendTextToCustomEdit(hwndHistory, _T_MSG(L"MSG_SYST_ME_ECHEC_I_25ec",L"[Système] Échec : Impossible de charger 'config_manager.py'.\r\n")); 
                if (PyErr_Occurred()) PyErr_Print();
                } else {
                PyObject* pProv = PyObject_GetAttrString(pCM, "obtenir_provider_actif");
                if (pProv && PyCallable_Check(pProv)) {
                    PyObject* pRes = PyObject_CallObject(pProv, NULL);
                    if (pRes) {
                        const char* provA = PyUnicode_AsUTF8(pRes);
                        wchar_t provTmp[64];
                        MultiByteToWideChar(CP_UTF8, 0, provA, -1, provTmp, 64);
                        if (wcslen(provTmp) > 0) {
                            provTmp[0] = towupper(provTmp[0]);
                            wcscpy_s(s_providerName, provTmp);
                        }
                        Py_DECREF(pRes);
                    }
                    Py_DECREF(pProv);
                }
                
                PyObject* pMod = PyObject_GetAttrString(pCM, "obtenir_modele_selectionne");
                if (pMod && PyCallable_Check(pMod)) {
                    PyObject* pRes = PyObject_CallObject(pMod, NULL);
                    if (pRes) {
                        const char* modA = PyUnicode_AsUTF8(pRes);
                        MultiByteToWideChar(CP_UTF8, 0, modA, -1, currentModelW, 128);
                        Py_DECREF(pRes);
                    }
                    Py_DECREF(pMod);
                }
                
                PyObject* pList = PyObject_GetAttrString(pCM, "obtenir_liste_modeles_str");
                if (pList && PyCallable_Check(pList)) {
                    PyObject* pRes = PyObject_CallObject(pList, NULL);
                    if (pRes) {
                        const char* listA = PyUnicode_AsUTF8(pRes);
                        wchar_t listW[2048];
                        MultiByteToWideChar(CP_UTF8, 0, listA, -1, listW, 2048);
                        
                        wchar_t* context = NULL;
                        wchar_t* token = wcstok_s(listW, L",", &context);
                        while (token != NULL) {
                            listeModeles.push_back(token);
                            token = wcstok_s(NULL, L",", &context);
                        }
                        Py_DECREF(pRes);
                    }
                    Py_DECREF(pList);
                }
                Py_DECREF(pCM);
            }
            
            SetWindowTextW(hWnd, _T_UI(IDD_CHAT_DLG));
            
            for (const auto& model : listeModeles) {
                SendMessageW(hwndCombo, CB_ADDSTRING, 0, (LPARAM)model.c_str());
            }
            int idxSel = SendMessageW(hwndCombo, CB_FINDSTRINGEXACT, -1, (LPARAM)currentModelW);
            SendMessageW(hwndCombo, CB_SETCURSEL, (idxSel != CB_ERR) ? idxSel : 0, 0);
            
            HFONT hFont = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            EnumChildWindows(hWnd, [](HWND hChild, LPARAM lp) -> BOOL { SendMessageW(hChild, WM_SETFONT, (WPARAM)lp, TRUE); return TRUE; }, (LPARAM)hFont);
            
            AppendRtfToRichEdit(hwndHistory, WcharToRtf( _T_MSG(L"MSG_ASSISTANT_S_BON_c9ca", L"Assistant IA : Bonjour ! Comment puis-je vous aider ?\r\n\r\n")));
            
            return TRUE;
        }
        
        case WM_SIZE: {
            int cx = LOWORD(lParam);
            int cy = HIWORD(lParam);
            
            HWND hwndStatic = GetDlgItem(hWnd, IDC_CHAT_STATIC_MODEL);
            HWND hwndCombo = GetDlgItem(hWnd, IDC_CHAT_MODEL_COMBO);
            HWND hwndHistory = GetDlgItem(hWnd, IDC_CHAT_HISTORY);
            HWND hwndInput = GetDlgItem(hWnd, IDC_CHAT_INPUT);
            HWND hwndBtnSend = GetDlgItem(hWnd, IDC_BTN_SEND);
            HWND hwndBtnClear = GetDlgItem(hWnd, IDC_BTN_CLEAR);
            HWND hwndCheck = GetDlgItem(hWnd, IDC_CHK_SAVE_CHAT);
            
            if (!hwndHistory) return FALSE;
            
            int spacing = 15;
            int btnWidth = 80;
            int checkHeight = 20;
            
            MoveWindow(hwndStatic, spacing, 18, 90, 20, TRUE);
            MoveWindow(hwndCombo, 110, 15, cx - 110 - spacing, 200, TRUE);
            
            int checkTop = cy - checkHeight - spacing;
            MoveWindow(hwndCheck, spacing, checkTop, cx - (spacing * 2), checkHeight, TRUE);
            
            int inputHeight = (int)(cy * 0.20);
            if (inputHeight < 60)  inputHeight = 20;
            if (inputHeight > 160) inputHeight = 160;
            
            int inputTop = checkTop - inputHeight - 10;
            int inputWidth = cx - btnWidth - (spacing * 2) - 10; 
            int singleBtnHeight = inputHeight / 2;
            
            MoveWindow(hwndInput, spacing, inputTop, inputWidth, inputHeight, TRUE);
            MoveWindow(hwndBtnClear, spacing + inputWidth + 10, inputTop, btnWidth, singleBtnHeight, TRUE);
            MoveWindow(hwndBtnSend, spacing + inputWidth + 10, inputTop + singleBtnHeight, btnWidth, inputHeight - singleBtnHeight, TRUE);
            
            int historyTop = 50;
            int historyHeight = inputTop - historyTop - 10;
            if (historyHeight < 50) historyHeight = 50;
            
            MoveWindow(hwndHistory, spacing, historyTop, cx - (spacing * 2), historyHeight, TRUE);
            
            return TRUE;
        }
        
        case WM_COMMAND: {
            WORD controlID = LOWORD(wParam);
            
            if (controlID == IDC_BTN_CLEAR) {
                HWND hwndHistory = GetDlgItem(hWnd, IDC_CHAT_HISTORY);
                SetWindowTextW(hwndHistory, L"");
                
                PyObject* pModule = PyImport_ImportModule("providers.groq");
                if (pModule) {
                    PyObject* pFunc = PyObject_GetAttrString(pModule, "effacer_historique_chat");
                    if (pFunc && PyCallable_Check(pFunc)) {
                        Py_XDECREF(PyObject_CallObject(pFunc, NULL));
                    }
                    Py_DECREF(pModule);
                }
                
                wchar_t msgBienvenue[128];
                swprintf_s(msgBienvenue, _T_MSG(L"MSG_ASSISTANT_S_HIS_6739",L"Assistant %s : Historique effacé. Nouvelle session.\n\r\n\r"), s_providerName);
                AppendTextToCustomEdit(hwndHistory, msgBienvenue);
                return TRUE;
            }
            
            if (controlID == ID_CONSOLE_COPY || controlID == ID_CONSOLE_SELECTALL || controlID == ID_CONSOLE_CLEAR) {
                HWND targetHwnd = (HWND)GetWindowLongPtrW(hWnd, DWLP_USER);
                if (targetHwnd) {
                    if (controlID == ID_CONSOLE_COPY) {
                        SendMessageW(targetHwnd, WM_COPY, 0, 0);
                    } 
                    else if (controlID == ID_CONSOLE_SELECTALL) {
                        CHARRANGE cr = { 0, -1 };
                        SendMessageW(targetHwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
                    } 
                    else if (controlID == ID_CONSOLE_CLEAR && targetHwnd == GetDlgItem(hWnd, IDC_CHAT_HISTORY)) {
                        SendMessageW(hWnd, WM_COMMAND, MAKEWPARAM(IDC_BTN_CLEAR, BN_CLICKED), 0);
                    }
                }
                return TRUE;
            }
            
            if (controlID == IDC_BTN_SEND) {
                HWND hwndHistory = GetDlgItem(hWnd, IDC_CHAT_HISTORY);
                HWND hwndInput = GetDlgItem(hWnd, IDC_CHAT_INPUT);
                HWND hwndCombo = GetDlgItem(hWnd, IDC_CHAT_MODEL_COMBO);
                
                wchar_t userText[20001] = {0};
                GetWindowTextW(hwndInput, userText, 20001);
                
                if (wcslen(userText) <= 0) {
                    LogToConsole(L"[ERREUR] Il n'y a pas de question à soumettre !!!"); // MSG_ERREUR_IL_N_Y_A_21c1 = "[ERREUR] Il n'y a pas de question à soumettre !!!\n"
                    MessageBoxW(nppData._nppHandle, 
                        _T_MSG(L"MSG_VEUILLEZ_POSER__2b0e",L"Veuillez poser votre question avant d'interroger l'Assistant IA."), 
                    _T_MSG(L"MSG_QUESTION_REQUIS_6a87",L"Question requise!"), MB_OK | MB_ICONWARNING);
                }
                
                else  {
                    wchar_t selectedModelW[128] = {0};
                    int selIdx = SendMessageW(hwndCombo, CB_GETCURSEL, 0, 0);
                    if (selIdx != CB_ERR) {
                        SendMessageW(hwndCombo, CB_GETLBTEXT, selIdx, (LPARAM)selectedModelW);
                    }
                    
                    AppendTextToCustomEdit(hwndHistory, L"\r\n");
                    AppendTextToCustomEdit(hwndHistory, _T_MSG(L"MSG_VOUS_9b9f",L"Vous : "));
                    AppendTextToCustomEdit(hwndHistory, L"\r\n");
                    AppendTextToCustomEdit(hwndHistory, userText);
                    AppendTextToCustomEdit(hwndHistory, L"\r\n\r\n");
                    
                    bool stockerChat = (IsDlgButtonChecked(hWnd, IDC_CHK_SAVE_CHAT) == BST_CHECKED);
                    
                    int size_needed = WideCharToMultiByte(CP_UTF8, 0, userText, -1, NULL, 0, NULL, NULL);
                    std::string userTextUtf8(size_needed, 0);
                    WideCharToMultiByte(CP_UTF8, 0, userText, -1, &userTextUtf8[0], size_needed, NULL, NULL);
                    
                    char selectedModelA[128];
                    WideCharToMultiByte(CP_UTF8, 0, selectedModelW, -1, selectedModelA, 128, NULL, NULL);
                    
                    SetWindowTextW(hwndInput, L"");
                    
                    // Récupère la valeur actuelle depuis Python (toujours à jour)
                    std::string activeProv = GetActiveProvider(); 
                    
                    // Utilise cette valeur pour charger le module
                    std::string moduleName = "providers." + activeProv;
                    PyObject* pModule = PyImport_ImportModule(moduleName.c_str());
                    
                    if (!pModule) {
                        AppendTextToCustomEdit(hwndHistory, _T_MSG(L"MSG_SYST_ME_ERREUR__33b7",L"[Système] Erreur : Impossible de charger le module .\r\n"));
                            if (PyErr_Occurred()) {
                                PyErr_Print(); // Cela va maintenant s'afficher dans votre console grâce à la redirection
                            }
                        } else {
                        PyObject* pFunc = PyObject_GetAttrString(pModule, "envoyer_chat");
                        if (!pFunc || !PyCallable_Check(pFunc)) {
                            AppendTextToCustomEdit(hwndHistory, _T_MSG(L"MSG_SYST_ME_ERREUR__d397",L"[Système] Erreur : La fonction 'envoyer_chat' est introuvable.\r\n\r\n"));
                            if (PyErr_Occurred()) PyErr_Print();
                            } else {
                            PyObject* pArgs = PyTuple_Pack(3, 
                                PyUnicode_FromString(userTextUtf8.c_str()), 
                                PyBool_FromLong(stockerChat),
                                PyUnicode_FromString(selectedModelA)
                            );
                            
                            PyObject* pRes = PyObject_CallObject(pFunc, pArgs);
                            
                            if (pRes) {
                                const char* responseA = PyUnicode_AsUTF8(pRes);
                                
                                PyObject* pFuncRtf = PyObject_GetAttrString(pModule, "markdown_to_rtf");
                                if (pFuncRtf && PyCallable_Check(pFuncRtf)) {
                                    PyObject* pArgsRtf = PyTuple_Pack(1, PyUnicode_FromString(responseA));
                                    PyObject* pResRtf = PyObject_CallObject(pFuncRtf, pArgsRtf);
                                    
                                    if (pResRtf) {
                                        const char* rtfContentA = PyUnicode_AsUTF8(pResRtf);
                                        
                                        wchar_t prefixAssistant[128];
                                        swprintf_s(prefixAssistant, 128,
                                                   _T_MSG(L"MSG_ASSISTANT_S_b8ed", L"Assistant %s : "),
                                                   s_providerName);
                                                   LogToConsole(prefixAssistant);

                                        AppendRtfToRichEdit(hwndHistory, WcharToRtf(prefixAssistant));
                                        AppendTextToCustomEdit(hwndHistory, L"\r\n");
                                        AppendRtfToRichEdit(hwndHistory, rtfContentA);
                                        AppendTextToCustomEdit(hwndHistory, L"\r\n\r\n");
                                        
                                        Py_DECREF(pResRtf);
                                    }
                                    Py_DECREF(pArgsRtf);
                                    Py_DECREF(pFuncRtf);
                                } 
                                else {
                                    wchar_t responseW[4096];
                                    MultiByteToWideChar(CP_UTF8, 0, responseA, -1, responseW, 4096);
                                    
                                    wchar_t prefixAssistant[128];
                                    swprintf_s(prefixAssistant,_T_MSG(L"MSG_ASSISTANT_S_f273", L"Assistant %s : "), s_providerName);
                                    
                                    AppendTextToCustomEdit(hwndHistory, prefixAssistant);
                                    AppendTextToCustomEdit(hwndHistory, responseW);
                                    AppendTextToCustomEdit(hwndHistory, L"\r\n\r\n");
                                }
                                Py_DECREF(pRes);
                                } else {
                                AppendTextToCustomEdit(hwndHistory, _T_MSG(L"MSG_SYST_ME_ERREUR__87b7",L"[Système] Erreur lors de l'exécution de la requête.\r\n"));
                                if (PyErr_Occurred()) PyErr_Print();
                            }
                            Py_DECREF(pArgs);
                            Py_DECREF(pFunc);
                        }
                        Py_DECREF(pModule);
                    }
                    
                    int len = GetWindowTextLengthW(hwndHistory);
                    SendMessageW(hwndHistory, EM_SETSEL, len, len);
                    SendMessageW(hwndHistory, EM_SCROLLCARET, 0, 0);
                }
                return TRUE;
            }
            if (controlID == IDCANCEL) {
                DestroyWindow(hWnd);
                g_hChatDlg = NULL;
                return TRUE;
            }
            break;
        }
        
        case WM_CONTEXTMENU: {
            HWND hwndHistory = GetDlgItem(hWnd, IDC_CHAT_HISTORY);
            HWND hwndInput = GetDlgItem(hWnd, IDC_CHAT_INPUT);
            
            if ((HWND)wParam == hwndHistory || (HWND)wParam == hwndInput) {
                HMENU hMenu = CreatePopupMenu();
                AppendMenuW(hMenu, MF_STRING, ID_CONSOLE_COPY, _T_MSG(L"MSG_COPIER",L"Copier"));
                AppendMenuW(hMenu, MF_STRING, ID_CONSOLE_SELECTALL, _T_MSG(L"MSG_TOUT_S_LECTIONN_e5c2",L"Tout sélectionner"));
                
                if ((HWND)wParam == hwndHistory) {
                    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
                    AppendMenuW(hMenu, MF_STRING, ID_CONSOLE_CLEAR, _T_MSG(L"MSG_NOUVELLE_DISCUS_136b",L"Nouvelle discussion (RAZ)"));
                }
                
                int xPos = GET_X_LPARAM(lParam);
                int yPos = GET_Y_LPARAM(lParam);
                
                if (xPos == -1 && yPos == -1) {
                    RECT rc; 
                    GetWindowRect((HWND)wParam, &rc);
                    xPos = rc.left + (rc.right - rc.left) / 2;
                    yPos = rc.top + (rc.bottom - rc.top) / 2;
                }
                
                TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, xPos, yPos, 0, hWnd, NULL);
                DestroyMenu(hMenu);
                
                SetWindowLongPtrW(hWnd, DWLP_USER, (LONG_PTR)wParam);
                return TRUE;
            }
            break;
        }
        
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam;
            
            if (lpDrawItem->CtlID == IDC_BTN_SEND || lpDrawItem->CtlID == IDC_BTN_CLEAR)
            {
                HDC hdc = lpDrawItem->hDC;
                RECT rect = lpDrawItem->rcItem;
                
                rect.top += 1;
                rect.bottom -= 1;
                
                // Détection propre des états du bouton
                bool isSelected = (lpDrawItem->itemState & ODS_SELECTED);
                bool isDisabled = (lpDrawItem->itemState & ODS_DISABLED);
                
                // --- ISSU DU SOUS-CLASSEMENT : On lit directement la propriété de la fenêtre ---
                bool isHovered = (GetPropW(lpDrawItem->hwndItem, L"MouseHovering") != NULL);
                
                COLORREF colorBg;
                COLORREF colorBorder;
                COLORREF colorText = RGB(255, 255, 255);
                
                if (lpDrawItem->CtlID == IDC_BTN_SEND) // Bouton Envoyer (Bleu)
                {
                    if (isDisabled) {
                        colorBg = RGB(60, 60, 60); colorBorder = RGB(80, 80, 80); colorText = RGB(130, 130, 130);
                        } else if (isSelected) {
                        colorBg = RGB(0, 90, 150);     colorBorder = RGB(0, 70, 120);
                        } else if (isHovered) {
                        colorBg = RGB(20, 142, 224);   colorBorder = RGB(0, 122, 204); // Éclairci au survol
                        } else {
                        colorBg = RGB(0, 122, 204);    colorBorder = RGB(0, 100, 180);
                    }
                }
                else // Bouton Effacer (Rouge)
                {
                    if (isDisabled) {
                        colorBg = RGB(60, 60, 60); colorBorder = RGB(80, 80, 80); colorText = RGB(130, 130, 130);
                        } else if (isSelected) {
                        colorBg = RGB(140, 30, 30);    colorBorder = RGB(110, 20, 20);
                        } else if (isHovered) {
                        colorBg = RGB(210, 70, 70);    colorBorder = RGB(180, 50, 50); // Éclairci au survol
                        } else {
                        colorBg = RGB(180, 50, 50);    colorBorder = RGB(150, 40, 40);
                    }
                }
                
                HPEN hPen = CreatePen(PS_SOLID, 1, colorBorder);
                HBRUSH hBrush = CreateSolidBrush(colorBg);
                
                HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
                
                RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 8, 8);
                
                SelectObject(hdc, hOldPen);
                SelectObject(hdc, hOldBrush);
                DeleteObject(hPen);
                DeleteObject(hBrush);
                
                wchar_t buttonText[64];
                GetWindowText(lpDrawItem->hwndItem, buttonText, 64);
                
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, colorText);
                DrawText(hdc, buttonText, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                
                if (lpDrawItem->itemState & ODS_FOCUS)
                {
                    InflateRect(&rect, -3, -3);
                    DrawFocusRect(hdc, &rect);
                }
                
                return TRUE;
            }
            break;
        }
        
        case WM_DESTROY:
        g_hChatDlg = NULL;
        return TRUE;
    }
    return FALSE;
}

// Fonction utilitaire locale pour ajouter proprement du texte dans notre historique
void AppendTextToCustomEdit(HWND hEdit, const wchar_t* wText) {
    if (!wText) return;
    
    // Normalisation des sauts de ligne (\n -> \r\n) pour rendre le Markdown lisible
    std::wstring normalized;
    for (size_t i = 0; wText[i] != L'\0'; ++i) {
        if (wText[i] == L'\n') {
            // Évite de doubler si c'est déjà précédé d'un \r
            if (i == 0 || wText[i - 1] != L'\r') {
                normalized += L"\r\n";
                } else {
                normalized += L'\n';
            }
            } else {
            normalized += wText[i];
        }
    }
    
    // Insertion à la fin du contrôle
    int index = GetWindowTextLengthW(hEdit);
    SendMessageW(hEdit, EM_SETSEL, index, index);
    SendMessageW(hEdit, EM_REPLACESEL, FALSE, (LPARAM)normalized.c_str());
    SendMessageW(hEdit, EM_SCROLLCARET, 0, 0);
}

// Callback nécessaire pour injecter du flux RTF dans le RichEdit
DWORD CALLBACK EditStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb) {
    std::string *pstr = (std::string *)dwCookie;
    if (pstr->empty()) {
        *pcb = 0;
        return 0;
    }
    int numToCopy = (pstr->length() < (size_t)cb) ? (int)pstr->length() : cb;
    memcpy(pbBuff, pstr->c_str(), numToCopy);
    *pcb = numToCopy;
    pstr->erase(0, numToCopy);
    return 0;
}

// Fonction pour ajouter du contenu RTF à la fin du RichEdit
void AppendRtfToRichEdit(HWND hRichEdit, const std::string& rtfContent) {
    std::string rtf = rtfContent;
    // Sélectionne la fin du document
    int index = GetWindowTextLengthW(hRichEdit);
    SendMessageW(hRichEdit, EM_SETSEL, index, index);
    
    EDITSTREAM es = { 0 };
    es.dwCookie = (DWORD_PTR)&rtf;
    es.pfnCallback = EditStreamCallback;
    
    // Injecte le RTF en l'insérant à la sélection actuelle sans écraser le reste
    SendMessageW(hRichEdit, EM_STREAMIN, SF_RTF | SFF_SELECTION, (LPARAM)&es);
    SendMessageW(hRichEdit, EM_SCROLLCARET, 0, 0);
}

