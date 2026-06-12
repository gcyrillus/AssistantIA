Tableaux des requis et fonctionnalités du plugin AssistantIA pour Notepad++
 
Tableau des requis
Catégorie | Requis | Détail / Valeur | Statut
---------|--------|---------------|--------
Plateforme | Windows uniquement | API Win32, RichEdit, HMODULE, HWND | Critique
Hôte | Notepad++ (x64) | Interface PluginInterface.h + Scintilla.h | Requis
Compilateur | C++ avec support DLL export | __declspec(dllexport), MSVC recommandé | Requis
Runtime Python | Python 3.14.5 embarqué | Dossier python3.14.5\ + python314.zip dans le plugin | Critique
Bibliothèques C++ | comctl32, version, kernel32, riched20 | Liées via #pragma comment(lib, ...) | Requis
Scripts Python | config_manager.py, logger.py, template.py | Dans python3.14.5\ du plugin | Requis
Dossier providers | Modules Python par fournisseur API | Sous plugins\AssistantIA\providers\ (usine) et AppData (user) | Requis
Stockage config | Fichier config.json | %APPDATA%\Notepad++\plugins\config\AssistantIA\ | Requis
Fichier INI | AssistantIA.ini (paramètres persistants) | Même dossier AppData — booléens (console, log…) | Requis
Fichiers langue | Fichiers .ini de traduction | Générés au premier démarrage via l'API IA si clé présente | Auto-généré
Connexion réseau | Accès HTTPS aux API IA distantes | urllib / ssl (contexte non-vérifié) — port 443 | Requis
Clé API | Clé valide du fournisseur sélectionné | Ex. Groq, OpenAI, Gemini… — stockée en clair dans config.json | Optionnel*
Encodage | UTF-8 bout en bout | ansi_cpg65001 (RTF), CP_UTF8 (Win32), encode('utf-8') Python | Critique
Droits écriture | Accès en écriture sur AppData | Création de dossiers Chats/, bckSelect/, fichiers .md | Environnement
Détection langue | Langue de Notepad++ détectée au démarrage | Synchronisée avec Python via synchroniser_langue_npp() | Système
 
* La clé API est optionnelle au sens technique (le plugin démarre sans), mais indispensable pour utiliser les fonctions IA.