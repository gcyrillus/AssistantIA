# Manuel d'Utilisation : Assistant IA
## Version Alpha 0.0.0.1

Le plugin **Assistant IA** transforme Notepad++ en un environnement de développement assisté par l'intelligence artificielle. Il vous permet d'interagir avec divers modèles de langage directement depuis votre éditeur pour analyser, corriger ou générer du code.
Le plugin est proposé avec, à minima, 4 assistants(API) : groq, openrouter, anthropic et gemini.

___

## Installation

### Prérequis
Version (c)Windows 64 bits & Notepad++ x64 : **au minimum Windows 10** (version **1809** ou supérieure) ou Windows 11**(*)**.
**(*)** Python: les versions **3.13** et **3.14**, le support de **Windows 7, 8** et **8.1** a été définitivement abandonné.
Ce plugin est compilé avec l'option `/MD` et est exclusivement conçu pour les architectures **x64**. 
- **Vérifier la compatibilité** : Dans Notepad++, allez dans le menu `?` (Aide) > `Informations de débogage...`. Vérifiez que la ligne d'architecture mentionne bien `64-bit x64`.
- **Dépendance système** : Étant compilé en `/MD`, le package ***Microsoft Visual C++ Redistributable*** doit être installé sur votre machine pour que Windows puisse charger le plugin.

### Téléchargement et installation
1. Téléchargez la dernière version du plugin depuis GitHub. https://github.com/gcyrillus/AssistantIA
2. Dézippez l'archive téléchargée dans un répertoire provisoire de votre choix.
3. Copiez le dossier complet `AssistantIA` (qui contient la DLL et ses dépendances).
4. Collez ce dossier dans le répertoire des plugins de Notepad++ (généralement situé dans `C:\Program Files\Notepad++\plugins`).
5. Redémarrez Notepad++.

___

## Utilisation

### Premier lancement
Lors du tout premier lancement de Notepad++ après l'installation, le plugin détectera qu'il n'a pas encore de paramètres. Une boîte de dialogue s'ouvrira automatiquement pour vous guider dans la configuration initiale de vos clés API.
Par défaut, le plugin est en Français, si il detecte une autre langue de configuration de Notepad++ il traduira les fichiers de langue et d'aide dans la langue correspondante à celle de Notepad++. 

### Configuration
Accessible à tout moment via la page de configuration, vous pouvez y gérer vos paramètres :
- **Assistant par défaut** : Le plugin utilise **Groq** par défaut pour des réponses quasi instantanées.
- **Clés API** : Vous devez renseigner votre clé pour communiquer avec l'IA. Pour Groq, vous pouvez obtenir une clé gratuitement sur la console développeur (*console.groq.com*).
- **Modèles** : L'interface vous permet d'ajouter, modifier ou mettre à jour les modèles de langage disponibles pour chaque fournisseur.
- **Enregistrement des logs** : Une case à cocher vous permet d'activer la sauvegarde de l'historique de vos interactions avec l'IA directement dans les fichiers de configuration.

___

## Utilisation au quotidien

### Le Panneau de Chat
C'est votre centre de commande principal pour discuter avec l'IA :
- **raccourci d'ouverture** : `ALT + I`.
- **Modèle actif** : Utilisez la liste déroulante en haut du panneau pour basculer rapidement entre les différents modèles proposés par votre fournisseur.
- **Input** : Tapez vos instructions dans la zone de texte située en bas.
- **Envoi** : Appuyez sur le bouton *Envoyer* ou utilisez votre raccourci clavier habituel pour soumettre la requête.
- **Historique** : La fenêtre centrale affiche vos échanges en temps réel. Si l'option est cochée dans la configuration, ces logs seront conservés.

### Menu Contextuel
Le menu contextuel (clic droit) vous permet d'interagir rapidement avec votre **code sélectionné** :
- **Code selectionné** : Il est limité au 20 000 premiers caractéres. Les fournisseurs(API) inclus sont des versions gratuites. Notepad++ vous indique dans sa barre de statut la longueur de la chaine selectionnée et le nombre de lignes: ex: *Sel : 787 : 7* .
- **raccourci d'ouverture** : `ALT + Q`.
- **Poser une question à l'IA** : Ouvre le panneau pour poser votre question(demande).
- **Valider** : Envoi votre demande à l'IA en injectant directement le contexte de votre sélection actuelle en arriere plan.
- **Réponse de l'IA** : La reponse de l'IA s'affiche en splitview dans un document provisoire *AssistantIA.txt* face au document de votre selection.

___


## La console :

Le plugin embarque une console. celle-ci est affichable ou non et peut-être detachée en fenêtre  flottante. Elle affiche des informations concernant les actions en cours et les evntuelles erreurs. Elle dispose aussi d'un prompt python.
Elle est utiles à des fins de débogages principalement.
___

## Fonctionnalités Avancées : Scripts et Templates

Le plugin est conçu pour être extensible par les utilisateurs avancés souhaitant personnaliser leur expérience et ajouter de nouveaux fournisseurs(APIs).

- **Création de nouveaux fournisseurs** : Via le menu du plugin, sélectionnez `Nouveau script` pour intégrer une nouvelle API IA.
- **Structure d'un script** : Chaque nouveau script est construit sur une base commune générée automatiquement. Il ne vous reste plus qu'à finaliser la logique de requête propre au fournisseur.
- **Modification des prompts** : Vous pouvez modifier le *System Prompt* directement dans les fichiers Python de chaque fournisseur pour adapter le comportement de l'IA à vos besoins en modifiant la valeur traduite  de : `system_prompt = _T("MESSAGES", "MSG_PROV_SYS_PROMPT_CODE")`. ex: *`system_prompt ="Tu es un expert en PHP"*, *"Réponds uniquement en français, ..."`*, etc.

___

## Dépannage (Troubleshooting)

Si le plugin ne se charge pas ou affiche une erreur, vérifiez les points suivants :

- **Absence de réponse** : Vérifiez d'abord que vous êtes bien connecté au réseau Internet.
- **Liste des modèles vide** : Vérifiez votre connexion internet, assurez-vous que votre clé API est valide, et vérifiez que le nom exact du modèle est bien reconnu par le fournisseur.
- **Crash au démarrage ou Plugin invisible** : Assurez-vous que le *Visual C++ Redistributable (x64)* est installé sur votre machine. Cette installation est strictement requise pour le fonctionnement de la DLL et le chargement des librairies Python associées (`.pyd`).

___

*Note : Ce plugin est un outil d'assistance au codage. Rester critique. Veillez à toujours relire et tester le code généré avant de l'intégrer dans vos projets de production.*
