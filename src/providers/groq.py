# -*- coding: utf-8 -*-
import urllib.request
import json
import ssl
import os
from datetime import datetime
import config_manager
from config_manager import _T
import re

HISTORIQUE_CHAT = []

def mettre_a_jour_modeles(api_key):
    """Va chercher la liste des modèles chez Groq en ignorant les blocages de certificats SSL locaux"""
    url = "https://api.groq.com/openai/v1/models"
    req = urllib.request.Request(url)
    req.add_header("Authorization", f"Bearer {api_key}")
    req.add_header("Content-Type", "application/json")
    req.add_header("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) NotepadPlusPlus/AssistantIA")
    
    ctx = ssl._create_unverified_context()
    try:
        with urllib.request.urlopen(req, timeout=7, context=ctx) as response:
            donnees = json.loads(response.read().decode("utf-8"))
            dictionnaire_modeles = {}
            for m in donnees.get("data", []):
                m_id = m.get("id")
                if "llama" in m_id or "mixtral" in m_id or "gemma" in m_id:
                    dictionnaire_modeles[m_id] = m_id
            return dictionnaire_modeles
    except Exception as e:
        return {"error": str(e)}

def verifier_cle(api_key):
    res = mettre_a_jour_modeles(api_key)
    if "error" in res:
        return False
    return True

def enregistrer_historique_dans_fichier(provider, modele):
    """Sauvegarde la discussion complète au format .md avec le nommage dynamique demandé"""
    global HISTORIQUE_CHAT
    try:
        # Organisation propre dans le sous-dossier Chats de AppData
        log_dir = os.path.join(config_manager.APPDATA_DIR, "Chats")
        if not os.path.exists(log_dir):
            os.makedirs(log_dir)
            
        # Format de la date : ANNEE_MOIS_JOUR-HEURE_MIN
        date_str = datetime.now().strftime("%Y%m%d-%H%M")
        
        # Nettoyage du nom du modèle pour éviter les caractères interdits sous Windows
        modele_clean = re.sub(r'[^a-zA-Z0-9_-]', '_', modele)
        
        # Format exact exigé : assistant-[provider]-[modele]-[date].md
        nom_fichier = f"assistant-{provider.lower()}-{modele_clean}-{date_str}.md"
        chemin_complet = os.path.join(log_dir, nom_fichier)
        
        with open(chemin_complet, "w", encoding="utf-8") as f:
            f.write(f"# Session de Chat Assistant IA - {datetime.now().strftime('%d/%m/%Y %H:%M')}\n\n")
            f.write(f"**Fournisseur :** `{provider}`  \n")
            f.write(f"**Modèle exploité :** `{modele}`  \n\n")
            f.write("---\n\n")
            
            for msg in HISTORIQUE_CHAT:
                if msg["role"] == "system":
                    continue
                auteur = "👤 **VOUS**" if msg["role"] == "user" else "🤖 **ASSISTANT IA**"
                f.write(f"### {auteur} :\n")
                f.write(f"{msg['content']}\n\n")
                f.write("---\n\n")
                
        return chemin_complet
    except Exception as e:
        print(f"[AssistantIA][" + provider.capitalize() + "] Erreur sauvegarde log .md : {e}")
        return None

# ============================================================================
# LOGIQUE DE CHAT ENRICHIE AVEC PARAMÈTRE À LA VOLÉE ET LOGS DE CONSOLE
# ============================================================================

def envoyer_chat(prompt, stocker_log, modele_a_la_volee=None):
    """Envoie la conversation à Groq en incluant l'historique et affiche le statut dans la console"""
    global HISTORIQUE_CHAT
    
    api_key = config_manager.obtenir_api_key()
    provider = config_manager.obtenir_provider_actif()
    
    # Choix prioritaire du modèle sélectionné à la volée dans la ComboBox C++
    if modele_a_la_volee and modele_a_la_volee.strip():
        modele = modele_a_la_volee
    else:
        modele = config_manager.obtenir_modele_selectionne()
        
    if not api_key:
        print("[AssistantIA][" + provider.capitalize() + "] ❌ ERREUR : Aucune clé API trouvée dans config.json.")
        return "[Système] Erreur : Aucune clé API configurée. Allez dans les options du plugin."
    if not modele:
        modele = "llama-3.3-70b-versatile"

    # --- LOG CONSOLE : ENVOI ---
    print(f"\n[AssistantIA][" + provider.capitalize() + "] 🚀 ---> ENVOI REQUÊTE API (Fournisseur: {provider})\n")
    print(f"\t\t : Modèle exploité : {modele}\n")
    print(f"\t\t : Taille du prompt : {len(prompt)} caractères\n")

    if not HISTORIQUE_CHAT:
        HISTORIQUE_CHAT.append({
            "role": "system", 
            "content": "Tu es un assistant IA expert en développement, intégré dans l'éditeur de texte Notepad++. Sois précis, concis et privilégie un code propre."
        })
        
    HISTORIQUE_CHAT.append({"role": "user", "content": prompt})
    
    url = "https://api.groq.com/openai/v1/chat/completions"
    payload = {
        "model": modele,
        "messages": HISTORIQUE_CHAT,
        "temperature": 0.3
    }
    
    req = urllib.request.Request(url)
    req.add_header("Authorization", f"Bearer {api_key}")
    req.add_header("Content-Type", "application/json")
    req.add_header("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) NotepadPlusPlus/AssistantIA")
    
    ctx = ssl._create_unverified_context()
    
    try:
        body = json.dumps(payload).encode("utf-8")
        print("[AssistantIA] 📡 Connexion aux serveurs de " + provider.capitalize() + "...")
        
        with urllib.request.urlopen(req, data=body, timeout=20, context=ctx) as response:
            status_code = response.getcode()
            
            # --- LOG CONSOLE : STATUT REÇU ---
            print(f"\t\t >>> 🟩 <--- RÉPONSE REÇUE: (Code HTTP : {status_code} OK)\n")
            
            donnees = json.loads(response.read().decode("utf-8"))
            reponse_ia = donnees["choices"][0]["message"]["content"]
            
            HISTORIQUE_CHAT.append({"role": "assistant", "content": reponse_ia})
            
            if stocker_log:
                chemin = enregistrer_historique_dans_fichier(provider, modele)
                if chemin:
                    print(f"[SUCCESS] 💾 Session de chat archivée au format .md : {chemin}\r\n")
                
            return reponse_ia
            
    except Exception as e:
        if HISTORIQUE_CHAT and HISTORIQUE_CHAT[-1]["role"] == "user":
            HISTORIQUE_CHAT.pop()
            
        # --- LOG CONSOLE : EN CAS D'ERREUR ---
        print(f"\nfailed ❌ !!! ÉCHEC REQUÊTE API !!! Details : {str(e)}")
        return f"[Erreur API Groq] Impossible de récupérer la réponse : {str(e)}"

def effacer_historique_chat():
    provider = config_manager.obtenir_provider_actif()
    global HISTORIQUE_CHAT
    HISTORIQUE_CHAT = []
    print("[AssistantIA][" + provider.capitalize() + "][SUCCESS] 🗑️ Mémoire de session réinitialisée.\r\n")
    return True

def markdown_to_rtf(text_md):
    """Convertisseur complet Markdown vers RTF natif pour Win32 RichEdit"""
    if not text_md:
        return ""
        
    # Table des polices (\f0=Segoe, \f1=Consolas) et Couleurs (1=Noir, 2=Gris foncé, 3=Gris clair)
    # AJOUT MAJEUR : \ansicpg65001 force le RichEdit à interpréter le flux en UTF-8 !
    en_tete = r"{\rtf1\ansi\ansicpg65001\uc1\deff0{\fonttbl{\f0\fnil\fcharset0 Segoe UI;}{\f1\fmodern\fcharset0 Consolas;}}{\colortbl;\red0\green0\blue0;\red40\green44\blue52;\red240\green240\blue240;}"
    
    # Nettoyage strict des retours chariots Windows pour éviter les sauts de ligne fantômes
    text_md = text_md.replace('\r\n', '\n').replace('\r', '\n')
    
    lines = text_md.split('\n')
    rtf_lines = []
    dans_code = False
    
    for line in lines:
        # 1. BASCULE DES BLOCS DE CODE (```)
        if line.strip().startswith("```"):
            dans_code = not dans_code
            if dans_code:
                # Ouvre le bloc : \pard (reset), fond gris (\highlight3), texte gris foncé (\cf2), Consolas (\f1), décalage (\li200)
                rtf_lines.append(r"\pard\highlight3\cf2\f1\fs19\li200 ")
            else:
                # Ferme le bloc : retour à la normale
                rtf_lines.append(r"\pard\highlight0\cf1\f0\fs20\li0 ")
            continue
        
        # 2. PROTECTION DES BALISES RTF INTERNES
        escaped_line = line.replace('\\', '\\\\').replace('{', '\\{').replace('}', '\\}')
        
        # 3. CONVERSION UNICODE (Sauvegarde les accents é, è, à pour le C++)
        escaped_line = "".join([f"\\u{ord(c)}?" if ord(c) > 127 else c for c in escaped_line])
        
        if dans_code:
            # Si on est dans un bloc de code, on n'interprète plus rien, on saute juste la ligne
            rtf_lines.append(f"{escaped_line}\\par")
        else:
            # --- 4. INTERPRÉTATION DU MARKDOWN ---
            
            # Titres (Gras + Taille de police augmentée)
            if escaped_line.startswith("### "):
                escaped_line = re.sub(r'^### (.*)', r'\\pard\\b\\fs24 \1\\b0\\fs20\\par', escaped_line)
            elif escaped_line.startswith("## "):
                escaped_line = re.sub(r'^## (.*)', r'\\pard\\b\\fs28 \1\\b0\\fs20\\par', escaped_line)
            elif escaped_line.startswith("# "):
                escaped_line = re.sub(r'^# (.*)', r'\\pard\\b\\fs32 \1\\b0\\fs20\\par', escaped_line)
            
            # Listes à puces (* ou -)
            elif re.match(r'^[\*\-] ', escaped_line):
                escaped_line = re.sub(r'^[\*\-] (.*)', r'\\pard\\li300\\bullet  \1\\par', escaped_line)
            
            # Listes numérotées (1. )
            elif re.match(r'^\d+\. ', escaped_line):
                escaped_line = re.sub(r'^(\d+\.) (.*)', r'\\pard\\li300 \1 \2\\par', escaped_line)
            
            # Ligne normale
            else:
                escaped_line = r"\pard\li0 " + escaped_line + r"\par"
            
            # Gras, Italique, et Code en ligne (inline)
            escaped_line = re.sub(r'\*\*(.*?)\*\*', r'\\b \1\\b0 ', escaped_line)
            escaped_line = re.sub(r'\*(.*?)\*', r'\\i \1\\i0 ', escaped_line)
            escaped_line = re.sub(r'`(.*?)`', r'\\highlight3\\f1 \1\\highlight0\\f0 ', escaped_line)
            
            rtf_lines.append(escaped_line)
            
    # On assemble le tout et on ferme le document RTF
    corps = "\n".join(rtf_lines)
    return en_tete + corps + "}"

def sauvegarder_log_selection_md(provider, modele, consigne, code_original, code_reponse):
    """Sauvegarde les actions contextuelles dans un fichier .md par jour et par modele."""
    provider = config_manager.obtenir_provider_actif()
    try:
        # Création du dossier bckSelect s'il n'existe pas
        log_dir = os.path.join(config_manager.APPDATA_DIR, "bckSelect")
        if not os.path.exists(log_dir):
            os.makedirs(log_dir)

        # Formatage du nom de fichier : AnnéeMoisJour-Provider-Modele.md
        date_file_str = datetime.now().strftime("%Y%m%d")
        modele_clean = re.sub(r'[^a-zA-Z0-9_-]', '_', modele)
        nom_fichier = f"{date_file_str}-{provider.lower()}-{modele_clean}.md"
        chemin_complet = os.path.join(log_dir, nom_fichier)

        # Formatage de l'heure pour l'en-tête interne
        date_entry_str = datetime.now().strftime("%Y/%m/%d %H:%M:%S")

        # Mode "a" (append) : Ajoute à la fin du fichier sans écraser les anciennes questions de la journée
        with open(chemin_complet, "a", encoding="utf-8") as f:
            f.write(f"## {date_entry_str}\n\n")
            f.write(f"### Question:\n{consigne}\n\n")
            f.write(f"### Code envoyé:\n```\n{code_original}\n```\n\n")
            f.write(f"### Code reçu:\n```\n{code_reponse}\n```\n\n")
            f.write("---\n\n")
            
    except Exception as e:
        print(f"[AssistantIA][" + provider.capitalize() + "] Erreur sauvegarde log bckSelect : {e}")

def demander_code_sur_selection(consigne, code_selectionne, sauvegarder_log=True):
    """Envoie la sélection à Groq de manière autonome et retourne du code brut (anti-markdown)."""
    # 1. Récupération automatique de la configuration
    api_key = config_manager.obtenir_api_key()
    modele = config_manager.obtenir_modele_selectionne()
    provider = config_manager.obtenir_provider_actif()
    
    if not api_key: return _T("MESSAGES", "MSG_PROV_ERR_NO_KEY_CODE")
    if not modele:  modele = "llama-3.3-70b-versatile" # Sécurité
        
    url = "https://api.groq.com/openai/v1/chat/completions"
    
    # 2. Système ultra-directif pour éviter le Markdown
    system_prompt = _T("MESSAGES", "MSG_PROV_SYS_PROMPT_CODE")
    
    print(system_prompt+"\n")
    payload = {
        "model": modele,
        "messages": [
            {"role": "system", "content": system_prompt},
            {"role": "user", "content": f"Consigne: {consigne}\n\nCode cible:\n{code_selectionne}"}
        ],
        "temperature": 0.2
    }
    
    req = urllib.request.Request(url)
    req.add_header("Authorization", f"Bearer {api_key}")
    req.add_header("Content-Type", "application/json")
    req.add_header("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) NotepadPlusPlus/AssistantIA")
    
    ctx = ssl._create_unverified_context()
    
    try:
        with urllib.request.urlopen(req, data=json.dumps(payload).encode("utf-8"), timeout=20, context=ctx) as response:
            res = json.loads(response.read().decode("utf-8"))
            code = res["choices"][0]["message"]["content"].strip()
                       
            # Nettoyage additionnel contre les blocs de code markdown
            code = re.sub(r'^```[a-zA-Z]*\n|```$', '', code, flags=re.MULTILINE)
            
            if sauvegarder_log:
                # Appel propre à la nouvelle fonction
                provider = config_manager.obtenir_provider_actif()
                sauvegarder_log_selection_md(provider, modele, consigne, code_selectionne, code)
                
            return code
    except Exception as e:
        return _T("MESSAGES", "MSG_PROV_ERR_REQ_CODE").format(provider="{provider.capitalize()}", e=str(e))