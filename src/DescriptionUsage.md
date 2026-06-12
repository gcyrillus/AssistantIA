Tableau des fonctionnalités du plugin AssistantIA pour Notepad++
### Tableau des fonctionnalités
Groupe | Fonctionnalité | Description | Couche | Accès
-----|-----|-----|-----|-----
**Chat IA** |  |  |  | 
Chat flottant | Ouvrir un chat IA | Fenêtre flottante redimensionnable avec historique multi-tours, sélection de modèle à la volée, bouton RESET conversation | C++ / Py | Menu + Ctrl
 | Sélection du modèle | Combobox peuplée dynamiquement par l'API du fournisseur actif ; modèle overridable par message | Python | Chat
 | Sauvegarde conversation | Export automatique en .md horodaté dans Chats\ (opt-in par case à cocher) | Python | Chat
 | Rendu RTF des réponses | Convertisseur Markdown→RTF natif (titres, listes, gras, italique, blocs de code Consolas, encodage Unicode \uXXXX?) | Python | Auto
**Action sur sélection** |  |  |  | 
IA contextuelle | Soumettre sélection à l'IA | Saisit la sélection Scintilla active, demande une consigne via dialog, appelle l'API et insère la réponse dans un fichier temporaire en vue scindée | C++ / Py | Menu + Clic droit
 | Vue scindée automatique | Résultat ouvert dans la vue opposée de Notepad++ (NPPM_DMMSHOW) sans perturber l'éditeur source | C++ | Auto
 | Backup sélection | Log quotidien question + code original + réponse IA en .md dans bckSelect\ | Python | Auto (opt-in)
**Gestion des fournisseurs API** |  |  |  | 
Providers | Ajouter un fournisseur | Formulaire (nom, provider, URL API, URL modèles, clé API) → génération du script Python depuis template.py | C++ / Py | Menu
 | Gérer les scripts | Liste + actions Créer / Éditer / Supprimer sur les modules providers | C++ | Menu
 | Changer de provider actif | Combobox dans Config générale ; bascule immédiate et persistance dans config.json | C++ / Py | Config
 | Mise à jour des modèles | Interroge l'endpoint /models du provider, peuple la liste et sauvegarde dans config.json | Python | Auto / Manuel
 | Vérification de la clé API | Validation indirecte via la récupération des modèles au premier démarrage | Python | Config
**Console de débogage** |  |  |  | 
Console | Panneau console ancrable | RichEdit50W ancré dans Notepad++ (DockingDlg), log horodaté de toutes les opérations C++ et Python | C++ | Menu / Toggle
 | Entrée Python interactive | Champ de saisie >>> permettant d'exécuter du code Python directement (PyRun_SimpleString) | C++ | Console
 | Menu contextuel console | Copier / Tout sélectionner / Effacer sur clic droit dans le panneau | C++ | Clic droit
 | Redirection stdout Python | sys.stdout / sys.stderr redirigés vers la console via callback C (logger.py + ctypes) | Python | Auto
**Internationalisation** |  |  |  | 
I18n | Détection langue Notepad++ | Langue lue au démarrage, synchronisée avec Python ; défaut : français | C++ / Py | Auto
 | Traduction UI dynamique | Toutes les fenêtres de dialogue traduites à l'initialisation via le moteur Traducteur Python | C++ / Py | Auto
 | Auto-génération des fichiers langue | Si clé API disponible, traduit les .ini de référence via l'API IA au premier démarrage dans la langue cible | Python | 1er démarrage
 | Écran de chargement | Fenêtre Win32 native affichée pendant la génération des traductions (thread séparé) | Python | 1er démarrage
**Configuration générale** |  |  |  | 
Paramètres | Configuration générale | Afficher console au démarrage, fermeture forcée, sélection provider actif + accès direct à la gestion des providers | C++ | Menu
 | Configuration provider | Saisie/masquage clé API (password), liste des modèles avec rafraîchissement, sauvegarde immédiate | C++ | Menu
 | Persistance paramètres | Booléens (console, log sélection) persistés dans AssistantIA.ini via API Windows WritePrivateProfileString | C++ | Auto
**Informations plugin** |  |  |  | 
Meta | Aide intégrée | Fenêtre RichEdit50W redimensionnable affichant le guide d'utilisation en RTF | C++ | Menu
 | À propos / Version | Informations produit (auteur, licence CC BY-NC-SA 4.0, version, nom DLL) lues depuis VERSIONINFO | C++ | Menu

C++ = AssistantIA.cpp/rc  
Python = config_manager.py / template.py / logger.py  
C++ / Py = orchestration inter-couches
